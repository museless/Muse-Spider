/*------------------------------------------
	Source file content Six part

	Part Zero:	Include
	Part One:	Define
	Part Two:	Local data
	Part Three:	Local function

	Part Four:	Message operator
	Part Five:	Message system framework

--------------------------------------------*/

/*------------------------------------------
	Part Zero: Include
--------------------------------------------*/

#include "spinc.h"
#include "spmsg.h"


/*------------------------------------------
	Part One: Define
--------------------------------------------*/

#define	sp_msg_is_part_exist(nPart) \
	((nPart < PART_MAX && (nPart == PART_URLBUG || nPart == PART_TEXTBUG || nPart == PART_EXTBUG)) ? \
	FUN_RUN_OK : FUN_RUN_END)

#define	sp_msg_is_require_com(chkComm, nComm) \
	((nComm & chkComm) == nComm) ? FUN_RUN_OK : FUN_RUN_END


/*------------------------------------------
	Part Four: Message operator

	1. sp_msg_read
	2. sp_msg_select_read
	3. sp_msg_write
	4. sp_msg_exam_recver
	5. sp_msg_exam_command
	6. sp_msg_recver_exist
	7. sp_msg_recver
	8. sp_msg_command
	9. sp_msg_take_comm
	10. sp_msg_fill_stru
	11. sp_msg_unfill_comm

--------------------------------------------*/

/*-----sp_msg_read-----*/
int sp_msg_read(int msgFd, PMSG *msgStru)
{
	int	nRet;

	if((nRet = readn(msgFd, (char *)msgStru, MSG_LEN)) == FUN_RUN_FAIL) {
		elog_write("sp_msg_read - select_read", FUNCTION_STR, ERROR_STR);
		return	FUN_RUN_FAIL;
	}

	return	nRet;
}


/*-----sp_msg_select_read-----*/
int sp_msg_select_read(int nSock, PMSG *mStru, int nSec)
{
	struct	timeval	timStru;
	fd_set		fdBuf;

	timStru.tv_sec = nSec;
	timStru.tv_usec = 0;

	FD_ZERO(&fdBuf);
	FD_SET(nSock, &fdBuf);

	if(select(nSock + 1, &fdBuf, NULL, NULL, &timStru) > FUN_RUN_END) {
		if(FD_ISSET(nSock, &fdBuf))
			return	(read(nSock, mStru, MSG_LEN));
	}

	return	FUN_RUN_END;
}


/*-----sp_msg_write-----*/
int sp_msg_write(int msgFd, PMSG *pMessage)
{
	if(write(msgFd, pMessage, MSG_LEN) == FUN_RUN_FAIL) {
		perror("sp_msg_write - write");
		return	FUN_RUN_FAIL;
	}

	return	FUN_RUN_OK;
}


/*-----sp_msg_exam_recver-----*/
int sp_msg_exam_recver(PMSG *msgStru, int nPart, char *ownName)
{
	if(sp_msg_is_part_exist(nPart))
		return	(msgStru->pm_recver == nPart) ? FUN_RUN_OK : FUN_RUN_END;

	char	strBuf[MIDDLE_BUF];

	sprintf(strBuf, "%s - %d", ownName, nPart);
	elog_write("sp_msg_exam_recver - part_exist", "unexisted part with", strBuf);

	return	FUN_RUN_FAIL;
}


/*-----sp_msg_exam_command-----*/
int sp_msg_exam_command(PMSG *mStru, int nComm)
{
	return	(sp_msg_is_require_com(mStru->pm_comm, nComm));
}


/*-----sp_msg_recver_exist-----*/
int sp_msg_recver_exist(PMSG *msgRec)
{
	return	sp_msg_is_part_exist(msgRec->pm_recver);
}


/*-----sp_msg_recver-----*/
inline int sp_msg_recver(PMSG *pMsg)
{
	return	pMsg->pm_recver;
}


/*-----sp_msg_command-----*/
inline int sp_msg_command(PMSG *pMsg)
{
	return	pMsg->pm_comm;
}


/*-----sp_msg_take_comm-----*/
void sp_msg_take_comm(PMSG *pGet, PMSG *pTake)
{
	pGet->pm_comm |= pTake->pm_comm;
	return;
}


/*-----sp_msg_fill_stru-----*/
void sp_msg_fill_stru(PMSG *pMsg, int nRecver, int nComm)
{
	pMsg->pm_recver = nRecver;
	pMsg->pm_comm |= nComm;

	return;
}


/*-----sp_msg_unfill_comm-----*/
void sp_msg_unfill_comm(PMSG *pMsg)
{
	pMsg->pm_comm = NO_MSG;
	return;
}


/*------------------------------------------
	Part Five: Message system framework

	1. sp_msg_frame_init
	2. sp_msg_frame_run
	3. sp_msg_frame_destroy
	4. sp_msg_frame_fd

--------------------------------------------*/

/*-----sp_msg_frame_init-----*/
MSGSET *sp_msg_frame_init(char *mName, int mNum, int mFd, int mSleep, msctim fCtime, mskpwk fKwork, mssend fSend)
{
	MSGSET	*msgSet;

	if((msgSet = malloc(sizeof(MSGSET))) == NULL)
		return	NULL;

	msgSet->ms_name = mName;
	msgSet->ms_num = mNum;
	msgSet->ms_fd = mFd;
	msgSet->ms_sptime = mSleep;

	msgSet->ms_ctime = fCtime;
	msgSet->ms_kpwork = fKwork;
	msgSet->ms_send = fSend;

	return	msgSet;
}


/*-----sp_msg_frame_run-----*/
int sp_msg_frame_run(MSGSET *msgSet, void *kpPara)
{
	PMSG	readMsg, cntMsg;
	int	msRet = FUN_RUN_OK;

	sp_msg_unfill_comm(&cntMsg);

	while(sp_msg_select_read(msgSet->ms_fd, &readMsg, msgSet->ms_sptime) > FUN_RUN_END) {
		if(sp_msg_exam_recver(&readMsg, msgSet->ms_num, msgSet->ms_name) == FUN_RUN_OK)
			sp_msg_take_comm(&cntMsg, &readMsg);
	}

	if(sp_msg_command(&cntMsg)) {
		if(sp_msg_exam_command(&cntMsg, CHANGE_TIME)) {
			if(msgSet->ms_ctime)
				msgSet->ms_ctime();
		}

		if(sp_msg_exam_command(&cntMsg, KEEP_WORKING)) {
			if(msgSet->ms_kpwork)
				msgSet->ms_kpwork(kpPara);
		}
	}

	if(msgSet->ms_send)
		msRet = msgSet->ms_send(msgSet->ms_fd);

	return	msRet;
}


/*-----sp_msg_frame_destroy-----*/
void sp_msg_frame_destroy(MSGSET *msgSet)
{
	if(msgSet)
		free(msgSet);
}


inline int sp_msg_frame_fd(MSGSET *msgSet)
{
	return	(msgSet) ? msgSet->ms_fd : FUN_RUN_FAIL;
}


