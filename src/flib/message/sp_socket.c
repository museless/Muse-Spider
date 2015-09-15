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
	Source file content Eight part

	Part Zero:	Include
	Part One:	Define
	Part Two:	Local data
	Part Three:	Local function

	Part Four:	Msg socket serv
	Part Five:	Msg socket client
	Part Six:	Msg control
	Part Seven:	Msg socket frame

--------------------------------------------*/

/*------------------------------------------
	Part Zero: Include
--------------------------------------------*/

#include "spinc.h"
#include "spmsg.h"


/*------------------------------------------
	Part One: Define
--------------------------------------------*/


/*------------------------------------------
	Part Four: Msg socket serv

	1. sp_msgs_serv_init
	2. sp_msgs_accept_and_test
	3. sp_msgs_set_fdset
	4. sp_msgs_sendsig_all

--------------------------------------------*/

/*-----sp_msgs_serv_init-----*/
SOCKSV *sp_msgs_serv_init(char *remoteIp, int remotePort, int backLog, mafun memFun, void *memHand, mafree freeFun)
{
	SOCKSV	*soServ;
	int	nReuse = 1;

	if(!memFun) {
		errno = EINVAL;
		return	NULL;
	}

	if(!(soServ = memFun(memHand, sizeof(SOCKSV))))
		return	NULL;

	/* assignment */
	soServ->sv_plist = NULL;

	soServ->sv_mhand = memHand;
	soServ->sv_malloc = memFun;
	soServ->sv_free = freeFun;

	soServ->sv_npeer = 0;
	soServ->sv_magic = SV_MAGIC;

	/* start to get socket */
	if((soServ->sv_sock = socket(AF_INET, SOCK_STREAM, 0)) == FUN_RET_NEG)
		return	NULL;

	if(setsockopt(soServ->sv_sock, SOL_SOCKET, SO_REUSEADDR, &nReuse, sizeof(int)) == FUN_RET_NEG)
		return	NULL;

	soServ->sv_sockin.sin_family = AF_INET;
	soServ->sv_sockin.sin_port = htons(remotePort);
	soServ->sv_sockin.sin_addr.s_addr = inet_addr(remoteIp);

	if(bind(soServ->sv_sock, (struct sockaddr *)(&soServ->sv_sockin), sizeof(SOCKIN)) == FUN_RET_NEG)
		return	NULL;

	if(listen(soServ->sv_sock, backLog) == FUN_RET_NEG)
		return	NULL;

	return	soServ;
}


/*-----sp_msgs_accept_and_test-----*/
int sp_msgs_accept_and_test(SOCKSV *pServ)
{
	SOCKPC	*peerCtl;
	SOCKP	soPeer;
	SOCKMG	soMsg;
	int	newSock, nSave;

	if((newSock = accept(pServ->sv_sock, NULL, NULL)) == FUN_RET_NEG)
		return	FUN_RUN_END;

	if(!sp_msgs_recv(newSock, &soPeer, sizeof(SOCKP))) {
		close(newSock);
		return	FUN_RUN_END;
	}

	nSave = (sp_msgs_peer_existed(pServ, soPeer.s_own) ? CLIENT_EXISTED : CLIENT_SIGNOK);
	sp_msgs_fill(soMsg, PART_KERNEL, soPeer.s_own, nSave);

	if(!sp_msgs_send(newSock, &soMsg, sizeof(SOCKMG)) || nSave == CLIENT_EXISTED) {
		close(newSock);
		return	FUN_RUN_OK;
	}

	/* when client isn't existed */
	if(!(peerCtl = pServ->sv_malloc(pServ->sv_mhand, sizeof(SOCKPC)))) {
		close(newSock);
		return	FUN_RUN_END;
	}

	peerCtl->sp_next = NULL;
	peerCtl->sp_info = soPeer;
	peerCtl->sp_sock = newSock;

	peerCtl->sp_savmsg = NO_MSG;

	sp_msgs_peer_insert(pServ, peerCtl);

	return	FUN_RUN_OK;
}


/*-----sp_msgs_set_fdset-----*/
int sp_msgs_set_fdset(SOCKSV *pServ, fd_set *pSet)
{
	SOCKPC	*sockCtl;
	int	maxSock;

	for(maxSock = 0, sockCtl = pServ->sv_plist; sockCtl; sockCtl = sockCtl->sp_next) {
		FD_SET(sockCtl->sp_sock, pSet);

		if(maxSock < sockCtl->sp_sock)
			maxSock = sockCtl->sp_sock;
	}

	return	maxSock;
}


/*-----sp_msgs_sendsig_all-----*/
void sp_msgs_sendsig_all(SOCKSV *pServ, int nSign)
{
	SOCKPC	*pPeer;

	for(pPeer = pServ->sv_plist; pPeer; pPeer = pPeer->sp_next)
		kill(pPeer->sp_info.s_pid, nSign);
}


/*------------------------------------------
	Part Five: Msg socket client

	1. sp_msgs_link
	2. sp_msgs_sign_and_test

--------------------------------------------*/

/*-----sp_msgs_link-----*/
int sp_msgs_link(char *remoteIp, int remotePort)
{
	SOCKIN	sockInfo;
	int	sockFd;

	if((sockFd = socket(AF_INET, SOCK_STREAM, 0)) == FUN_RET_NEG)
		return	FUN_RUN_FAIL;

	sockInfo.sin_family = AF_INET;
	sockInfo.sin_port = htons(remotePort);
	sockInfo.sin_addr.s_addr = inet_addr(remoteIp);

	if(connect(sockFd, (struct sockaddr *)&sockInfo, sizeof(SOCKIN)) == FUN_RET_NEG)
		return	FUN_RUN_FAIL;

	return	sockFd;
}


/*-----sp_msgs_sign_and_test-----*/
int sp_msgs_sign_and_test(int nSocket, int nOwn)
{
	SOCKP	sendInfo;
	SOCKMG	recvMsg;

	sendInfo.s_pid = getpid();
	sendInfo.s_own = nOwn;

	if(!sp_msgs_send(nSocket, &sendInfo, sizeof(SOCKP)))
		return	FUN_RUN_END;

	if(!sp_msgs_recv(nSocket, &recvMsg, sizeof(SOCKMG)))
		return	FUN_RUN_END;

	if(recvMsg.sm_send != PART_KERNEL) {
		errno = EBADMSG;
		return	FUN_RUN_END;
	}

	if(recvMsg.sm_comm == CLIENT_EXISTED) {
		errno = EUSERS;
		return	FUN_RUN_END;
	}

	return	FUN_RUN_OK;
}


/*------------------------------------------
	Part Six: Msg control

	1. sp_msgs_send
	2. sp_msgs_detect_send
	3. sp_msgs_msend
	3. sp_msgs_recv
	4. sp_msgs_select_recv
	5. sp_msgs_peer_existed
	6. sp_msgs_peer_insert
	7. sp_msgs_take_comm
	8. sp_msgs_make_empty
	9. sp_msgs_msg_empty
	10. sp_msgs_exam

--------------------------------------------*/

/*-----sp_msgs_send-----*/
int sp_msgs_send(int nSock, void *sockMsg, int nSize)
{
	return	((write(nSock, sockMsg, nSize) == nSize) ? FUN_RUN_OK : FUN_RUN_END);
}


/*-----sp_msgs_detect_send-----*/
int sp_msgs_detect_send(int nSock, void *sockMsg, int nSize)
{
	return	(!nSize ? FUN_RUN_OK : ((!write(nSock, sockMsg, nSize) || errno) ? FUN_RUN_END: FUN_RUN_OK));
}


/*-----sp_msgs_msend-----*/
int sp_msgs_msend(SOCKSV *pServ, SOCKPC *pControl, int nSend, int nCommand)
{
	SOCKMG	sockMsg;

	sp_msgs_fill(sockMsg, nSend, (pControl->sp_info.s_own), nCommand);

	return	sp_msgs_detect_send(pControl->sp_sock, &sockMsg, sizeof(SOCKMG));
}


/*-----sp_msgs_recv-----*/
int sp_msgs_recv(int nSock, void *sockMsg, int nSize)
{
	return	((read(nSock, sockMsg, nSize) > FUN_RET_ZERO) ? FUN_RUN_OK : FUN_RUN_END);
}


/*-----sp_msgs_select_recv-----*/
int sp_msgs_select_recv(int nSock, void *sockMsg, int nSize)
{
	TMVAL	timeSet;
	fd_set	fdBuf;

	timeSet.tv_sec = TAKE_A_SEC;
	timeSet.tv_usec = 0;

	FD_ZERO(&fdBuf);
	FD_SET(nSock, &fdBuf);

	if(select(nSock + 1, &fdBuf, NULL, NULL, &timeSet) > FUN_RUN_END) {
		if(FD_ISSET(nSock, &fdBuf))
			return	(read(nSock, sockMsg, nSize));
	}

	return	FUN_RUN_END;
}


/*-----sp_msgs_peer_existed-----*/
SOCKPC *sp_msgs_peer_existed(SOCKSV *pServ, int nCheck)
{
	SOCKPC	*lPeer;
	int	nCir;

	for(nCir = 0, lPeer = pServ->sv_plist; lPeer && nCir < pServ->sv_npeer; nCir++, lPeer = lPeer->sp_next) {
		if(lPeer->sp_info.s_own == nCheck)
			return	lPeer;
	}

	return	NULL;
}


/*-----sp_msgs_peer_insert-----*/
void sp_msgs_peer_insert(SOCKSV *pServ, SOCKPC *pList)
{
	pServ->sv_npeer++;
	pList->sp_next = pServ->sv_plist;
	pServ->sv_plist = pList;
}


/*-----sp_msgs_take_message-----*/
void sp_msgs_take_message(SOCKMG *curMsg, SOCKMG *comeMsg)
{
	curMsg->sm_comm |= comeMsg->sm_comm;
}


/*-----sp_msgs_make_empty-----*/
void sp_msgs_make_empty(SOCKMG *sockMsg)
{
	sockMsg->sm_comm = NO_MSG;
}


/*-----sp_msgs_msg_empty-----*/
int sp_msgs_msg_empty(SOCKMG *pMsg)
{
	return	(pMsg->sm_comm == NO_MSG) ? FUN_RUN_OK : FUN_RUN_END;
}


/*-----sp_msgs_exam-----*/
int sp_msgs_exam(SOCKMG *pMsg, int nCommand)
{
	return	(pMsg->sm_comm & nCommand) ? FUN_RUN_OK : FUN_RUN_END;
}


/*-----sp_msgs_sender-----*/
int sp_msgs_sender(SOCKMG *pMsg, int nSender)
{
	return	(pMsg->sm_send == nSender) ? FUN_RUN_OK : FUN_RUN_END;
}


/*------------------------------------------
	Part Seven: Msg socket frame

	1. sp_msgs_frame_init
	2. sp_msgs_frame_run

--------------------------------------------*/

/*-----sp_msgs_frame_init-----*/
SOSET *sp_msgs_frame_init(int nIndex, msctim fCtime, mskpwk fKeep, mssend fSend)
{
	SOSET	*soSet;

	if((soSet = malloc(sizeof(SOSET))) == NULL)
		return	NULL;

	if((soSet->sc_sock = sp_msgs_link(SOCK_IP, SOCK_PORT)) == FUN_RUN_FAIL)
		return	NULL;

	if(!sp_msgs_sign_and_test(soSet->sc_sock, nIndex))
		return	NULL;

	soSet->sc_ctime = fCtime;
	soSet->sc_kpwork = fKeep;
	soSet->sc_send = fSend;

	return	soSet;
}


/*-----sp_msgs_frame_run-----*/
int sp_msgs_frame_run(SOSET *msgSet, void *kpPara)
{
	SOCKMG	readMsg, curMsg;
	int	fRet = FUN_RUN_OK;

	sp_msgs_make_empty(&curMsg);

	while(sp_msgs_select_recv(msgSet->sc_sock, &readMsg, sizeof(SOCKMG)) > FUN_RUN_END)
		sp_msgs_take_message(&curMsg, &readMsg);

	if(!sp_msgs_msg_empty(&curMsg)) {
		if(sp_msgs_exam(&curMsg, CALL_PROCESS)) {
			printf("Message: at call process\n");
		}

		if(sp_msgs_exam(&curMsg, CHANGE_TIME)) {
			if(msgSet->sc_ctime)
				msgSet->sc_ctime();
		}

		if(sp_msgs_exam(&curMsg, KEEP_WORKING)) {
			if(msgSet->sc_kpwork)
				msgSet->sc_kpwork(kpPara);
		}
	}

	if(msgSet->sc_send)
		fRet = msgSet->sc_send(msgSet->sc_sock);

	return	fRet;
}


/*-----sp_msgs_frame_destroy-----*/
void sp_msgs_frame_destroy(SOSET *msgSet)
{
	if(msgSet)
		free(msgSet);
}
