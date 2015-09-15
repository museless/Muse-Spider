/* Copyright (c) 2015, William Muse
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */


/*------------------------------------------
	Source file content Ten part

	Part Zero:	Include
	Part One:	Local data
	Part Two:	Local function
	Part Three:	Define

	Part Four:	Kernel main
	Part Five:	Kernel Initialization
	Part Six:	Kernel signal handle
	Part Seven:	Time change part
	Part Eight:	Process control
	Part Ten:	Error handler

--------------------------------------------*/


/*------------------------------------------
	Part Zero: Include
--------------------------------------------*/

#include "spinc.h"
#include "spmsg.h"

#include "mmdpool.h"
#include "mgc.h"

#include <sys/select.h>
#include <sys/socket.h>


/*------------------------------------------
	Part One: Local data
--------------------------------------------*/

/* typedef */
typedef	struct	partlist	PARTL;

typedef	int	(*ktcf)(void);

/* struct */
struct	partlist {
	int	p_fd;
	int	p_pid;
	PMSG	p_mstru;

	int	p_flags;
	int	p_cmode;
};

/* local data */
static	MGCH	*kernGarCol;
static	DMPH	*kernMemPool;
static	SOCKSV	*kernSockServ;
static	char	dateSaveBuf[SMALL_BUF];
static	char	minSaveBuf[SMALL_BUF];


/*------------------------------------------
	Part Two: Local function
--------------------------------------------*/

/* Part Four */
static	void	kern_main(void);

/* Part Five */
static	int	kern_data_init(void);
static	int	kern_base_init(void);
static	int	kern_signal_init(void);

/* Part Six */
static	void	kern_sigdeal_entrance(int nSignal);

/* Part Seven */
static	void	kern_time_check_init(void);
static	void	kern_time_checking(ktcf ctFun);
static	int	kern_time_switch_check(void);
static	int	kern_time_call_check(void);

/* Part Eight */
static	void	kern_remove_socket(SOCKSV *pServ, SOCKPC *pForward, SOCKPC *pRemove);

/* Part Nine */
static	void	kern_perror(char *errStr, int nError);


/*------------------------------------------
	Part Three: Define
--------------------------------------------*/

#define	SELECT_TIME	0x8


/*------------------------------------------
	Part Four: Kernel main

	1. main
	2. kern_main

--------------------------------------------*/

/*-----main-----*/
int main(int argc, char *argv[])
{
	if(kern_base_init() && kern_data_init()) {
		if(kern_signal_init())
			kern_main();
	}

	printf("Kernel---> unexpected exit\n");

	mgc_all_clean(kernGarCol);

	return	FUN_RUN_END;
}


/*-----kern_main-----*/
static void kern_main(void)
{
	SOCKPC	*peerCtl, *recvPeer;
	TMVAL	seleTime;
	SOCKMG	soMsg;
	fd_set	readSet;
	int	maxSock, nSend;

	FD_ZERO(&readSet);
	kern_time_check_init();

	while(FUN_RUN_OK) {
		kern_time_checking(kern_time_call_check);
		kern_time_checking(kern_time_switch_check);

		/* setting maxsock and fdset */
		FD_SET(kernSockServ->sv_sock, &readSet);
		maxSock = sp_msgs_set_fdset(kernSockServ, &readSet);
		maxSock = ((!maxSock) ? kernSockServ->sv_sock : maxSock) + 1;

		seleTime.tv_sec = SELECT_TIME;
		seleTime.tv_usec = 0;

		if(select(maxSock, &readSet, NULL, NULL, &seleTime) < FUN_RUN_OK)
			continue;

		for(nSend = 0, peerCtl = kernSockServ->sv_plist; peerCtl; peerCtl = peerCtl->sp_next) {
			if(FD_ISSET(peerCtl->sp_sock, &readSet)) {
				if(!sp_msgs_recv(peerCtl->sp_sock, &soMsg, sizeof(SOCKMG)))
					continue;

				if(peerCtl->sp_info.s_own != soMsg.sm_send) {
					printf("Kernel---> own: %d - msgown: %d\n", peerCtl->sp_info.s_own, soMsg.sm_send);
					continue;
				}

				if(!(recvPeer = sp_msgs_peer_existed(kernSockServ, soMsg.sm_recv)))
					continue;

				recvPeer->sp_savmsg |= soMsg.sm_comm;
				nSend++;
			}
		}

		if(FD_ISSET(kernSockServ->sv_sock, &readSet)) {
			if(!sp_msgs_accept_and_test(kernSockServ)) {
				if(errno == ENOMEM)
					printf("Kernel---> kernel source out!!!\n");
			}
		}

		if(nSend) {
			for(peerCtl = kernSockServ->sv_plist; peerCtl; peerCtl = peerCtl->sp_next) {
				if(peerCtl->sp_savmsg != NO_MSG) {
					sp_msgs_fill(soMsg, PART_KERNEL, peerCtl->sp_info.s_own, peerCtl->sp_savmsg);

					sp_msgs_send(peerCtl->sp_sock, &soMsg, sizeof(SOCKMG));
					peerCtl->sp_savmsg = NO_MSG;
				}
			}
		}
	}
}


/*------------------------------------------
	Part Five: Kernel Initialization

	1. kern_base_init
	2. kern_data_init
	3. kern_signal_init

--------------------------------------------*/

/*-----kern_base_init-----*/
static int kern_base_init(void)
{
	if(!(kernGarCol = mgc_init())) {
		kern_perror("kern_base_init - mgc_init", errno);
		return	FUN_RUN_END;
	}

	mc_conf_load("Kernel", "/MuseSp/conf/kernel.cnf");

	if(mgc_add(kernGarCol, NULL_POINT, (gcfun)mc_conf_unload) == MGC_FAILED)
		kern_perror("kern_base_init - mgc_add - garcol", errno);

	elog_init("kernel_err_log");

	if(mgc_add(kernGarCol, NULL_POINT, (gcfun)elog_destroy) == MGC_FAILED)
		kern_perror("kern_base_init - mgc_add - elog", errno);

	return	FUN_RUN_OK;
}


/*-----kern_data_init-----*/
static int kern_data_init(void)
{
	int	mpSize;

	if(mc_conf_read("kernel_mpool_size", CONF_NUM, &mpSize, sizeof(int)) == FUN_RUN_FAIL) {
		mc_conf_print_err("kernel_mpool_size");
		return	FUN_RUN_END;
	}

	if(!(kernMemPool = mmdp_create(mpSize))) {
		elog_write("kern_data_init - sp_msg_write", FUNCTION_STR, ERROR_STR);
		return	FUN_RUN_END;
	}

	if(mgc_add(kernGarCol, kernMemPool, (gcfun)mmdp_free_all) == MGC_FAILED)
		kern_perror("kern_data_init - mgc_add - mempool", errno);

	if(!(kernSockServ = sp_msgs_serv_init(SOCK_IP, SOCK_PORT, SOCK_BACKLOG, (mafun)mmdp_malloc, kernMemPool, NULL))) {
		elog_write("kern_data_init - sp_msgs_serv_init", FUNCTION_STR, ERROR_STR);
		return	FUN_RUN_END; 
	}

	return	FUN_RUN_OK;
}


/*-----kern_signal_init-----*/
static int kern_signal_init(void)
{
	SIGAC	sigStru;
	SIGSET	sigSet;

	/* SIGINT */
	sigfillset(&sigSet);

	sigStru.sa_handler = kern_sigdeal_entrance;
	sigStru.sa_flags = 0;
	sigStru.sa_mask = sigSet;

	if(sigaction(SIGINT, &sigStru, NULL) == FUN_RUN_FAIL) {
		kern_perror("kern_signal_init - sigaction - SIGINT", errno);
		return	FUN_RUN_END;
	}

	/* SIGPIPE */
	sigfillset(&sigSet);

	sigStru.sa_handler = kern_sigdeal_entrance;
	sigStru.sa_flags = 0;
	sigStru.sa_mask = sigSet;

	if(sigaction(SIGPIPE, &sigStru, NULL) == FUN_RUN_FAIL) {
		kern_perror("kern_signal_init - sigaction - SIGPIPE", errno);
		return	FUN_RUN_END;
	}

	return	FUN_RUN_OK;
}


/*------------------------------------------
	Part Six: Kernel signal handle

	1. kern_sigdeal_entrance

--------------------------------------------*/

/*-----kern_sigdeal_entrance-----*/
static void kern_sigdeal_entrance(int nSignal)
{
	if(nSignal == SIGINT) {
		printf("Kernel---> quitting...\n");

		sp_msgs_sendsig_all(kernSockServ, SIGINT);

		mgc_all_clean(kernGarCol);
		exit(FUN_RUN_FAIL);
	}

	if(nSignal == SIGPIPE) {

	}
}


/*------------------------------------------
	Part Seven: Time change part

	1. kern_time_check_init
	2. kern_time_checking
	3. kern_time_switch_check
	4. kern_time_call_check

--------------------------------------------*/

/*-----kern_time_check_init-----*/
static void kern_time_check_init(void)
{
	TMS	*tStru;
	time_t	timSec;

	timSec = time(NULL);
	tStru = localtime(&timSec);

	sprintf(dateSaveBuf, "%04d%02d%02d", tStru->tm_year + 1900, tStru->tm_mon, tStru->tm_mday);
	sprintf(minSaveBuf, "%02d", tStru->tm_min);
}


/*-----kern_ctime_part-----*/
static void kern_time_checking(ktcf ctFun)
{
	SOCKPC	*soPeer, *foPeer;

	/* use for changing everyone's time */
	if(ctFun()) {
		for(foPeer = soPeer = kernSockServ->sv_plist; soPeer; foPeer = soPeer, soPeer = soPeer->sp_next) {
			if(!sp_msgs_msend(kernSockServ, soPeer, PART_KERNEL, CALL_PROCESS))
				kern_remove_socket(kernSockServ, foPeer, soPeer);
		}
	}
}


/*-----kern_time_switch_check-----*/
static int kern_time_switch_check(void)
{
	TMS	*tStru;
	time_t	timSec;
	char	timSave[SMALL_BUF];

	timSec = time(NULL);
	tStru = localtime(&timSec);

	sprintf(timSave, "%04d%02d%02d", tStru->tm_year + 1900, tStru->tm_mon, tStru->tm_mday);

	if(strcmp(timSave, dateSaveBuf)) {
		strcpy(dateSaveBuf, timSave);
		return	FUN_RUN_OK;
	}

	return	 FUN_RUN_END;
}


/*----kern_time_call_check-----*/
static int kern_time_call_check(void)
{
	TMS	*tStru;
	time_t	timSec;
	char	minSave[SMALL_BUF];

	timSec = time(NULL);
	tStru = localtime(&timSec);

	sprintf(minSave, "%02d", tStru->tm_min);

	if(strcmp(minSave, minSaveBuf)) {
		strcpy(minSaveBuf, minSave);
		return	FUN_RUN_OK;
	}

	return	 FUN_RUN_END;
}


/*------------------------------------------
	Part Eight: Process control

	1. kern_remove_socket

--------------------------------------------*/

/*-----kern_remove_socket-----*/
static void kern_remove_socket(SOCKSV *pServ, SOCKPC *pForward, SOCKPC *pRemove)
{
	printf("Kernel---> Process: %d - Own: %d down\n", pRemove->sp_info.s_pid, pRemove->sp_info.s_own);

	if(pRemove == pServ->sv_plist) {
		pServ->sv_plist = pRemove->sp_next;

	} else {
		pForward->sp_next = pRemove->sp_next;
	}

	mmdp_free(kernMemPool, (mpt_t *)pRemove);

	close(pRemove->sp_sock);
	pServ->sv_npeer--;
}


/*------------------------------------------
	Part Ten: Error handler

	1. kern_perror

--------------------------------------------*/

/*-----kern_perror-----*/
static void kern_perror(char *errStr, int nError)
{
	printf("Kernel---> %s: %s\n", errStr, strerror(nError));
}
