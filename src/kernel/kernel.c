/*------------------------------------------
	Source file content Eleven part

	Part Zero:	Include
	Part One:	Local data
	Part Two:	Local function
	Part Three:	Define

	Part Four:	Kernel main
	Part Five:	Kernel Initialization
	Part Six:	Kernel signal handle
	Part Seven:	Exec part
	Part Eight:	PLC control
	Part Nine:	Time change part
	Part Ten:	Help

--------------------------------------------*/


/*------------------------------------------
	Part Zero: Include
--------------------------------------------*/

#include "spinc.h"
#include "spmsg.h"
#include "mgc.h"
#include <strings.h>
#include <sys/select.h>
#include <sys/select.h>
#include <sys/socket.h>


/*------------------------------------------
	Part One: Local data
--------------------------------------------*/

/* typedef */
typedef	void	(*kn_exec)(char *fdStr);

typedef	struct	partlist	PARTL;

/* struct */
struct	partlist {
	kn_exec	p_exec;

	int	p_fd;
	int	p_pid;
	PMSG	p_mstru;

	int	p_flags;
};

/* data */
static	PARTL	mPartList[PART_MAX];
static	MGCH	*kernGarCol;
static	char	dateSaveBuf[SMALL_BUF];


/*------------------------------------------
	Part Two: Local function
--------------------------------------------*/

/* Part Four */
static	void	kern_main(void);

/* Part Five */
static	void	kern_data_init(void);
static	void	kern_init(int nPara, char **agBuf);
static	void	kern_base_init(void);
static	void	kern_signal_init(void);
static	void	kern_child_init(int iPart, int nFlags, kn_exec pExec);

/* Part Six */
static	void	kern_sigdeal_entrance(int nSignal);

/* Part Seven */
static	void	kern_exec_urlbug(char *fdStr);
static	void	kern_exec_textbug(char *fdStr);
static	void	kern_exec_extbug(char *fdStr);
static	void	kern_exec_outbug(char *fdStr);

/* Part Eight */
static	void	kern_part_add(int nId, int nPid, int nFlags, int nPipe);
static	void	kern_part_cut(PARTL *cutPart);

/* Part Nine */
static	void	kern_ctime_init(void);
static	void	kern_ctime_part(void);

/* Part Ten */
static	void	kern_print_help(void);


/*------------------------------------------
	Part Three: Define
--------------------------------------------*/

#define	FD_MAX		PART_MAX

#define	SELECT_WTIME	0x8


/*------------------------------------------
	Part Four: Kernel main

	1. main
	2. kern_main

--------------------------------------------*/

/*-----main-----*/
int main(int argc, char *argv[])
{
	kern_data_init();
	kern_base_init();
	kern_signal_init();

	kern_init(argc, argv);

	kern_main();

	return	FUN_RUN_END;
}


/*-----kern_main-----*/
static void kern_main(void)
{
	struct	timeval	seleTime;
	PARTL		*pTable;
	PMSG		readMsg;
	fd_set		kFds;

	FD_ZERO(&kFds);
	kern_ctime_init();

	while(FUN_RUN_OK) {
		kern_ctime_part();

		for(pTable = mPartList; pTable < mPartList + FD_MAX; pTable++) {
			if(pTable->p_flags & PART_INITED)
				FD_SET(pTable->p_fd, &kFds);
		}

		seleTime.tv_sec = SELECT_WTIME;
		seleTime.tv_usec = 0;

		if(select(FD_MAX, &kFds, NULL, NULL, &seleTime) < FUN_RUN_OK) {
			sleep(TAKE_A_REST);
			continue;
		}

		for(pTable = mPartList; pTable < mPartList + FD_MAX; pTable++) {
			if((pTable->p_flags & PART_INITED) && FD_ISSET(pTable->p_fd, &kFds)) {
				if(sp_msg_read(pTable->p_fd, &readMsg) == FUN_RUN_FAIL) {
					elog_write("kern_main - sp_msg_read", FUNCTION_STR, ERROR_STR);
					if(errno == ECONNRESET)
						kern_part_cut(pTable);

					continue;
				}

				if(sp_msg_recver_exist(&readMsg) && 
				   (mPartList[sp_msg_recver(&readMsg)].p_flags & PART_INITED))
					sp_msg_take_comm(&mPartList[sp_msg_recver(&readMsg)].p_mstru, &readMsg);
			}
		}

		for(pTable = mPartList; pTable < mPartList + PART_MAX; pTable++) {
			if((pTable->p_flags & PART_INITED) && sp_msg_command(&pTable->p_mstru)) {
				if(writen(pTable->p_fd, &pTable->p_mstru, sizeof(PMSG)) == FUN_RUN_FAIL) {
					if(errno == EPIPE)
						pTable->p_flags ^= PART_INITED;

					elog_write("kern_main - writen", FUNCTION_STR, ERROR_STR);
					continue;
				}

				sp_msg_unfill_comm(&pTable->p_mstru);
			}
		}
	}
}


/*------------------------------------------
	Part Five: Kernel Initialization

	1. kern_data_init
	1. kern_init
	2. kern_base_init
	3. kern_signal_init
	4. kern_child_init

--------------------------------------------*/

/*-----kern_data_init-----*/
static void kern_data_init(void)
{
	int	nCir;

	for(nCir = 0; nCir < PART_MAX; nCir++)
		mPartList[nCir].p_flags = PART_UNINIT;
}


/*-----kern_init-----*/
static void kern_init(int nPara, char **agBuf)
{
	int	nCir;

	bzero(mPartList, PART_MAX * sizeof(PARTL));

	for(nCir = 1; nCir < nPara; nCir++) {
		if(!strcmp(agBuf[nCir], "-u") || !strcmp(agBuf[nCir], "--urlbug")) {
			mPartList[PART_URLBUG].p_flags = PART_INITED;
			mPartList[PART_URLBUG].p_exec = kern_exec_urlbug;

		} else if(!strcmp(agBuf[nCir], "-t") || !strcmp(agBuf[nCir], "--textbug")) {
			mPartList[PART_TEXTBUG].p_flags = PART_INITED;
			mPartList[PART_TEXTBUG].p_exec = kern_exec_textbug;
			
		} else if(!strcmp(agBuf[nCir], "-e") || !strcmp(agBuf[nCir], "--extbug")) {
			mPartList[PART_EXTBUG].p_flags = PART_INITED;
			mPartList[PART_EXTBUG].p_exec = kern_exec_extbug;

		} else if(!strcmp(agBuf[nCir], "-o") || !strcmp(agBuf[nCir], "--outbug")) {
			mPartList[PART_OUTBUG].p_flags = PART_INITED;
			mPartList[PART_OUTBUG].p_exec = kern_exec_outbug;

		} else {
			kern_print_help();
			exit(FUN_RUN_FAIL);
		}
	}

	if(nCir == 1) {
		printf("---> kernel dos not have the other part - exit\n");
		kern_print_help();
		exit(FUN_RUN_FAIL);
	}

	for(nCir = 0; nCir < PART_MAX; nCir++) {
		if(mPartList[nCir].p_flags == PART_INITED)
			kern_child_init(nCir, PART_IC, mPartList[nCir].p_exec);
	}

	sleep(TAKE_A_EYECLOSE);
}


/*-----kern_base_init-----*/
static void kern_base_init(void)
{
	if((kernGarCol = mgc_init()) == NULL) {
		printf("Kernel---> kern_base_init - mgc_init - no memery to alloc kernGarCol\n");
		exit(FUN_RUN_FAIL);
	}

	mc_conf_load("Kernel", "/MuseSp/conf/kernel.cnf");

	if(mgc_add(kernGarCol, NULL_POINT, (gcfun)mc_conf_unload) == MGC_FAILED)
		perror("Kernel---> kern_base_init - mgc_add - kernGarCol");

	elog_init("kernel_err_log");

	if(mgc_add(kernGarCol, NULL_POINT, (gcfun)elog_destroy) == MGC_FAILED)
		elog_write("kern_base_init - mgc_add", "kern_error_log", ERROR_STR);
}


/*-----kern_signal_init-----*/
static void kern_signal_init(void)
{
	struct	sigaction	sigStru;
	sigset_t		sigSet;

	/* SIGINT */
	sigfillset(&sigSet);

	sigStru.sa_handler = kern_sigdeal_entrance;
	sigStru.sa_flags = 0;
	sigStru.sa_mask = sigSet;

	if(sigaction(SIGINT, &sigStru, NULL) == FUN_RUN_FAIL) {
		perror("Kernel---> kern_signal_init - sigaction - SIGINT");
		exit(FUN_RUN_FAIL);
	}

	/* SIGPIPE */
	sigfillset(&sigSet);

	sigStru.sa_handler = kern_sigdeal_entrance;
	sigStru.sa_flags = 0;
	sigStru.sa_mask = sigSet;

	if(sigaction(SIGPIPE, &sigStru, NULL) == FUN_RUN_FAIL) {
		perror("Kernel---> kern_signal_init - sigaction - SIGPIPE");
		exit(FUN_RUN_FAIL);
	}
}


/*-----kern_child_init-----*/
static void kern_child_init(int iPart, int nFlags, kn_exec pExec)
{
	int	chPid, sockBuf[2];
	char	fdBuf[8];

	if(socketpair(AF_LOCAL, SOCK_STREAM, 0, sockBuf) == FUN_RUN_FAIL) {
		perror("Kernel---> kernel - kern_child_init - socketpair");
		exit(FUN_RUN_FAIL);
	}

	if((chPid = fork()) == FUN_RUN_FAIL) {
		perror("Kernel---> kernel - kern_child_init - fork");
		exit(FUN_RUN_FAIL);
	
	} else if(chPid == 0) {
		close(sockBuf[0]);
		sprintf(fdBuf, "%d", sockBuf[1]);

		pExec(fdBuf);
	}

	kern_part_add(iPart, chPid, nFlags, sockBuf[0]);
	close(sockBuf[1]);
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
		sleep(TAKE_A_SHNAP);

		mgc_all_clean(kernGarCol);
		exit(FUN_RUN_FAIL);
	}

	if(nSignal == SIGPIPE) {

	}
}


/*------------------------------------------
	Part Seven: Exec part

	1. kern_exec_urlbug
	2. kern_exec_textbug
	3. kern_exec_extbug

--------------------------------------------*/

/*-----kern_exec_urlbug-----*/
static void kern_exec_urlbug(char *fdStr)
{
	char	ubugPath[PATH_LEN];

	if(mc_conf_read("urlbug_exec_locate", CONF_STR, ubugPath, PATH_LEN) == FUN_RUN_FAIL) {
		mc_conf_print_err("urlbug_exec_locate");
		exit(FUN_RUN_FAIL);
	}

	if(execl(ubugPath, ubugPath, "--write_db", "-f", fdStr, NULL) == FUN_RUN_FAIL) {
		perror("Kernel---> kern_exec_urlbug - execl - urlbug");
		exit(FUN_RUN_FAIL);
	}
}


/*-----kern_exec_textbug-----*/
static void kern_exec_textbug(char *fdStr)
{
	char	txbugPath[PATH_LEN];

	if(mc_conf_read("textbug_exec_locate", CONF_STR, txbugPath, PATH_LEN) == FUN_RUN_FAIL) {
		mc_conf_print_err("textbug_exec_locate");
		exit(FUN_RUN_FAIL);
	}

	if(execl(txbugPath, txbugPath, "-d", "-f", fdStr, NULL) == FUN_RUN_FAIL) {
		perror("Kernel---> kern_exec_textbug - execl - textbug");
		exit(FUN_RUN_FAIL);
	}
}


/*-----kern_exec_extbug-----*/
static void kern_exec_extbug(char *fdStr)
{
	char	exbugPath[PATH_LEN];

	if(mc_conf_read("exbug_exec_locate", CONF_STR, exbugPath, PATH_LEN) == FUN_RUN_FAIL) {
		mc_conf_print_err("exbug_exec_locate");
		exit(FUN_RUN_FAIL);
	}

	if(execl(exbugPath, exbugPath, "-d", "-f", fdStr, NULL) == FUN_RUN_FAIL) {
		perror("Kernel---> kern_exec_extbug - execl - extbug");
		exit(FUN_RUN_FAIL);
	}
}


/*-----kern_exec_outbug-----*/
static void kern_exec_outbug(char *fdStr)
{
	char	oubugPath[PATH_LEN];

	if(mc_conf_read("outbug_exec_locate", CONF_STR, oubugPath, PATH_LEN) == FUN_RUN_FAIL) {
		mc_conf_print_err("outbug_exec_locate");
		exit(FUN_RUN_FAIL);
	}

	if(execl(oubugPath, oubugPath, "-f", fdStr, NULL) == FUN_RUN_FAIL) {
		perror("Kernel---> kern_exec_outbug - execl - outbug");
		exit(FUN_RUN_FAIL);
	}
}


/*------------------------------------------
	Part Eight: PLC control

	1. kern_part_add
	2. kern_part_cut

--------------------------------------------*/

/*-----kern_part_add-----*/
static void kern_part_add(int nId, int nPid, int nFlags, int nPipe)
{
	if(nPipe < 0 || nId < 0) {
		printf("kern_part_add - wrong nId or nPipe - please check your code\n\n");
		exit(FUN_RUN_FAIL);
	}

	mPartList[nId].p_pid = nPid;
	mPartList[nId].p_fd = nPipe;
	mPartList[nId].p_flags |= nFlags;
	sp_msg_fill_stru(&mPartList[nId].p_mstru, nId, NO_MSG);
}


/*-----kern_part_cut-----*/
static void kern_part_cut(PARTL *cutPart)
{
	cutPart->p_flags = PART_UNINIT;
	close(cutPart->p_fd);
}


/*------------------------------------------
	Part Nine: Time change part

	1. kern_ctime_init
	1. kern_ctime_part

--------------------------------------------*/

/*-----kern_ctime_init-----*/
static void kern_ctime_init(void)
{
	struct	tm	*tStru;
	time_t		timSec;

	timSec = time(NULL);
	tStru = localtime(&timSec);

	sprintf(dateSaveBuf, "%04d%02d%02d%02d", tStru->tm_year + 1900, tStru->tm_mon, tStru->tm_mday, tStru->tm_min);
}


/*-----kern_ctime_part-----*/
static void kern_ctime_part(void)
{
	struct	tm	*tStru;
	PMSG		writeMsg;
	time_t		timSec;
	char		timSave[SMALL_BUF];
	int		nCir;

	timSec = time(NULL);
	tStru = localtime(&timSec);

	sprintf(timSave, "%04d%02d%02d%02d", tStru->tm_year + 1900, tStru->tm_mon, tStru->tm_mday, tStru->tm_min);

	/* use for changing everyone's time */
	if(strcmp(timSave, dateSaveBuf)) {
		strcpy(dateSaveBuf, timSave);

		for(nCir = 0; nCir < PART_MAX; nCir++) {
			if((mPartList[nCir].p_flags & PART_IC) == PART_IC) {
				sp_msg_fill_stru(&writeMsg, CHANGE_TIME, nCir);

				if(sp_msg_write(mPartList[nCir].p_fd, &writeMsg) == FUN_RUN_FAIL)
					perror("Kernel---> ipc_main - sp_msg_write");
			}
		}
	}
}


/*------------------------------------------
	Part Ten: Help

	1. kern_print_help

--------------------------------------------*/

/*-----kern_print_help-----*/
static void kern_print_help(void)
{
	printf("usage: \r\t[-u or --urlbug] urlbug\n \
		\r\t[-t or --textbug] textbug\n \
		\r\t[-e or --extbug] extbug\n \
		\r\t[-o or --outbug] outbug\n \
		\r\t[-h or --help]\n\n");
}

