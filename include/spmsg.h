/*Copyright (c) 2015, William Muse
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


#ifndef	_SPMSG_H
#define	_SPMSG_H


/*---------------------------
	include
-----------------------------*/

#include "spnet.h"


/*---------------------------
	define
-----------------------------*/

/* part id */
#define	PART_UNINIT	0x0
#define	PART_INITED	0x1
#define	PART_CTIME	0x2
#define	PART_IC		(PART_INITED | PART_CTIME)

#define	PART_MAX	0x20

#define	PART_KERNEL	0xFF
#define	PART_URLBUG	0x0
#define	PART_TEXTBUG	0x1
#define	PART_EXTBUG	0x2
#define	PART_OUTBUG	0x3

/* part comm */
#define	NO_MSG		0x0
#define	KEEP_WORKING	0x1
#define	CHANGE_TIME	0x2
#define	CALL_PROCESS	0x3

/* change time mode */
#define	CMODE_SIGNAL	0x1
#define	CMODE_SOCKET	0x2

/*---------------------------
	typedef
-----------------------------*/

typedef	void	(*msctim)(void);
typedef	void	(*mskpwk)(void *);
typedef	int	(*mssend)(int);

typedef	void	*(*msginit)(void);


/*-----------------------------
-------------------------------

	Socketpair

-------------------------------
-------------------------------*/

/*---------------------------
	typedef
-----------------------------*/

typedef	struct	pmsg	PMSG;
typedef	struct	msgset	MSGSET;


/*---------------------------
	struct
-----------------------------*/

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


/*---------------------------
	global function
-----------------------------*/

/* flib/message/sp_socketpair.c */

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


/*-----------------------------
-------------------------------

	For Socket

-------------------------------
-------------------------------*/

/*---------------------------
	define
-----------------------------*/

#define	CLIENT_EXISTED	0x4
#define	CLIENT_SIGNOK	0x5

#define	SV_MAGIC	0x4D757365


/*---------------------------
	typedef
-----------------------------*/

typedef	struct sockaddr_in	SOCKIN;

typedef	struct sockserv		SOCKSV;	/* service's control */
typedef	struct sockpeer		SOCKP;	/* when linking, client send it to service */
typedef	struct sockpeerctl	SOCKPC;	/* service use it to control the peer */

typedef	struct sockmsg		SOCKMG;	/* service and client use it to trans message */

typedef	struct sockset		SOSET;

/*---------------------------
	struct
-----------------------------*/

/* SOCKSV */
struct	sockserv {
	SOCKPC	*sv_plist;

	void	*sv_mhand;
	mafun	sv_malloc;
	mafree	sv_free;

	int	sv_magic;

	SOCKIN	sv_sockin;

	int	sv_sock;
	int	sv_npeer;
};

/* SOCKP */
struct	sockpeer {
	pid_t	s_pid;
	int	s_own;
};

/* SOCKPC */
struct	sockpeerctl {
	SOCKPC	*sp_next;

	SOCKP	sp_info;
	int	sp_sock;

	int	sp_savmsg;
};

/* msg part */
struct	sockmsg {
	int	sm_recv;
	int	sm_send;
	int	sm_comm;
};

#define	MSG_SIZE	(sizeof(struct sockmsg))

/* SOSET */
struct	sockset {
	int	sc_sock;

	msctim	sc_ctime;
	mskpwk	sc_kpwork;
	mssend	sc_send;
};

/*---------------------------
	global function
-----------------------------*/

/* flib/message/sp_socket.c */
SOCKSV	*sp_msgs_serv_init(char *remoteIp, int remotePort, int backLog, mafun memFun, void *memHand, mafree freeFun);
int	sp_msgs_accept_and_test(SOCKSV *pServ);
int	sp_msgs_set_fdset(SOCKSV *pServ, fd_set *pSet);
void	sp_msgs_sendsig_all(SOCKSV *pServ, int nSign);

int	sp_msgs_link(char *remoteIp, int remotePort);
int	sp_msgs_sign_and_test(int nSocket, int nOwn);

int	sp_msgs_send(int nSock, void *sockMsg, int nSize);
int	sp_msgs_detect_send(int nSock, void *sockMsg, int nSize);
int	sp_msgs_msend(SOCKSV *pServ, SOCKPC *pControl, int nSend, int nCommand);
int	sp_msgs_recv(int nSock, void *sockMsg, int nSize);
int	sp_msgs_select_recv(int nSock, void *sockMsg, int nSize);
SOCKPC	*sp_msgs_peer_existed(SOCKSV *pServ, int nCheck);
void	sp_msgs_peer_insert(SOCKSV *pServ, SOCKPC *pList);
void	sp_msgs_take_message(SOCKMG *curMsg, SOCKMG *comeMsg);
void	sp_msgs_make_empty(SOCKMG *sockMsg);
int	sp_msgs_msg_empty(SOCKMG *pMsg);
int	sp_msgs_exam(SOCKMG *pMsg, int nCommand);
int	sp_msgs_sender(SOCKMG *pMsg, int nSender);

SOSET	*sp_msgs_frame_init(int nIndex, msctim fCtime, mskpwk fKeep, mssend fSend);
int	sp_msgs_frame_run(SOSET *msgSet, void *kpPara);
void	sp_msgs_frame_destroy(SOSET *msgSet);

#define	sp_msgs_fill(msgStru, nSend, nRecv, nComm) { \
	msgStru.sm_send = (nSend); \
	msgStru.sm_recv = (nRecv); \
	msgStru.sm_comm = (nComm); \
}

#endif
