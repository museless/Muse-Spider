#ifndef	_SPMSG_H
#define	_SPMSG_H


/* define */
/* part id */
#define	PART_UNINIT	0x0
#define	PART_INITED	0x1
#define	PART_CTIME	0x2
#define	PART_IC		(PART_INITED | PART_CTIME)

#define	PART_MAX	0x20

#define	PART_URLBUG	0x0
#define	PART_TEXTBUG	0x1
#define	PART_EXTBUG	0x2
#define	PART_OUTBUG	0x3

/* part comm */
#define	NO_MSG		0x0
#define	KEEP_WORKING	0x1
#define	CHANGE_TIME	0x2

/* typedef */
typedef	void	(*msctim)(void);
typedef	void	(*mskpwk)(void *);
typedef	int	(*mssend)(int);

typedef	void	*(*msginit)(int);

typedef	struct	pmsg	PMSG;
typedef	struct	msgset	MSGSET;

/* struct */
struct	pmsg {
	short	pm_recver;
	int	pm_comm;
};

struct	msgset {
	char	*ms_name;
	int	ms_num;
	int	ms_sptime;
	int	ms_fd;

	msctim	ms_ctime;
	mskpwk	ms_kpwork;
	mssend	ms_send;
};

#define	MSG_LEN	sizeof(PMSG)

/* global function */

/* flib/sp_message.c */
int	sp_msg_read(int msgFd, PMSG *msgStru);
int	sp_msg_select_read(int nSock, PMSG *mStru, int nSec);
int	sp_msg_write(int msgFd, PMSG *pMessage);
int	sp_msg_exam_recver(PMSG *msgStru, int nPart, char *ownName);
int	sp_msg_exam_command(PMSG *mStru, int nComm);
int	sp_msg_recver_exist(PMSG *msgRec);
int	sp_msg_recver(PMSG *pMsg);
int	sp_msg_command(PMSG *pMsg);

void	sp_msg_take_comm(PMSG *pGet, PMSG *pTake);
void	sp_msg_fill_stru(PMSG *pMsg, int nRecver, int nComm);
void	sp_msg_unfill_comm(PMSG *pMsg);

MSGSET	*sp_msg_frame_init(char *mName, int mNum, int mFd, int mSleep, msctim fCtime, mskpwk fKwork, mssend fSend);
int	sp_msg_frame_run(MSGSET *msgSet, void *kpPara);
void	sp_msg_frame_destroy(MSGSET *msgSet);
int	sp_msg_frame_fd(MSGSET *msgSet);

#endif
