/*------------------------------------------
	Otbug keyword relate

	Source file content Five part

	Part Zero:	Include
	Part One:	Define
	Part Two:	Local data
	Part Three:	Local function

	Part Four:	Init part

--------------------------------------------*/

/*------------------------------------------
	Part Zero: Include
--------------------------------------------*/

#include "spotool.h"


/*------------------------------------------
	Part One: Define
--------------------------------------------*/

/* Define */

/* typedef */

/* struct */


/*------------------------------------------
	Part Two: Local data
--------------------------------------------*/


/*------------------------------------------
	Part Three: Local function
--------------------------------------------*/


/*------------------------------------------
	Part Four: Init part

	1. otbug_tool_frame_init
	2. otbug_tool_signup
	3. otbug_tool_signal_init

--------------------------------------------*/

/*-----otbug_tool_frame_init-----*/
int otbug_tool_frame_init(char *frameCaller, MGCH **garCollect, MSHM **shmReader, otpefun errFun)
{
	char	semFile[PATH_LEN];

	/* garbage collector init */
	if(!((*garCollect) = mgc_init())) {
		errFun("otbug_tool_frame_init - mgc_init", errno);
		return	FUN_RUN_END;
	}

	/* conf reader init */
	if(mc_conf_load(frameCaller, "/MuseSp/conf/outbug.cnf") == FUN_RUN_FAIL) {
		errFun("otbug_tool_frame_init - mc_conf_load", errno);
		return	FUN_RUN_END;
	}

	if(mgc_add((*garCollect), NULL_POINT, (gcfun)mc_conf_unload) == MGC_FAILED)
		errFun("otbug_tool_frame_init - mgc_add - mc_conf", errno);

	/* share memory link init */
	if(mc_conf_read("outbug_semkey_file", CONF_STR, semFile, PATH_LEN) == FUN_RUN_FAIL) {
		mc_conf_print_err("outbug_semkey_file");
		return	FUN_RUN_END;
	}

	if(!((*shmReader) = mshm_link(semFile, PROJ_SHM_CTL))) {
		errFun("otbug_tool_frame_init - mshm_link", errno);
		return	FUN_RUN_END;
	}

	return	FUN_RUN_OK;
}


/*-----otbug_tool_signup-----*/
int otbug_tool_signup(char *pathBuf, TLCTL *pCtl, otpefun errFun)
{
	int	nLen, signFd;
	STAT	stBuf;
	pid_t	nPid;

	if(mc_conf_read("outbug_datsave_locate", CONF_STR, pathBuf, PATH_LEN) == FUN_RUN_FAIL) {
		mc_conf_print_err("outbug_datsave_locate");
		return	FUN_RUN_END;
	}

	nLen = strlen(pathBuf);
	nLen += sprintf(pathBuf + nLen, "%s/", otbug_tcler_date(pCtl));

	sprintf(pathBuf + nLen, "tmpFile");

	if((signFd = open(pathBuf, O_RDWR)) == FUN_RET_NEG) {
		errFun("otbug_tool_signup - open", errno);
		return	FUN_RUN_END;
	}

	otbug_tmpfile_lock(pCtl);

	if(fstat(signFd, &stBuf) == FUN_RET_NEG) {
		errFun("otbug_tool_signup - fstat", errno);
		otbug_tmpfile_unlock(pCtl);
		return	FUN_RUN_END;
	}

	nPid = getpid();

	if(pwrite(signFd, &nPid, sizeof(pid_t), stBuf.st_size) == FUN_RET_NEG) {
		errFun("otbug_tool_signup - pwrite", errno);
		otbug_tmpfile_unlock(pCtl);
		return	FUN_RUN_END;
	}

	pathBuf[nLen] = 0;

	otbug_tmpfile_unlock(pCtl);
	close(signFd);

	return	FUN_RUN_OK;	
}


/*-----otbug_tool_signal_init-----*/
int otbug_tool_signal_init(sigfun fSignal, otpefun errFun)
{
	struct	sigaction	sigStru;
	sigset_t		sigMask;

	/* for signal int */
	sigemptyset(&sigMask);
	sigaddset(&sigMask, SIGINT);
	sigaddset(&sigMask, SIGSEGV);

	sigStru.sa_mask = sigMask;
	sigStru.sa_handler = fSignal;
	sigStru.sa_flags = 0;

	if(sigaction(SIGINT, &sigStru, NULL) == FUN_RUN_FAIL) {
		errFun("otbug_tool_signal_init - sigaction - SIGINT", errno);
		return	FUN_RUN_END;
	}

	/* for signal segv */
	sigStru.sa_mask = sigMask;

	if(sigaction(SIGSEGV, &sigStru, NULL) == FUN_RUN_FAIL) {
		errFun("otbug_tool_signal_init - sigaction - SIGSEGV", errno);
		return	FUN_RUN_END;
	}

	/* for signal usr1 */
	sigemptyset(&sigMask);
	sigaddset(&sigMask, SIGINT);
	sigaddset(&sigMask, SIGUSR1);

	if(sigaction(SIGUSR1, &sigStru, NULL) == FUN_RUN_FAIL) {
		errFun("otbug_tool_signal_init - sigaction - SIGUSR1", errno);
		return	FUN_RUN_END;
	}

	/* for signal usr2 */
	sigaddset(&sigMask, SIGINT);
	sigaddset(&sigMask, SIGUSR1);
	sigaddset(&sigMask, SIGSEGV);

	if(sigaction(SIGUSR2, &sigStru, NULL) == FUN_RUN_FAIL) {
		errFun("otbug_tool_signal_init - sigaction - SIGUSR2", errno);
		return	FUN_RUN_END;
	}

	return	FUN_RUN_OK;
}
