#ifndef	_SPOGLOBAL_H
#define	_SPOGLOBAL_H

/* global data */
extern	MGCH	*obGarCol;
extern	MGCO	otResCol;

extern	void	*obDynaMp, *obThreadMp;	/* DMPH  (mmdpool.h) */
extern	void	*obMsgSet;		/* MSGSET (spmsg.h) */
extern	void	*obShmContainer;	/* MSHM  (mshm.h) */
extern	void	*obHashBucket;		/* HBET  (malgo.h) */
extern	void	*obShmCtler, *obTimLock;/* TLCTL (spoutb.h) */
extern	void	*obThreadPool;		/* PTHPOOL (mpctl.h) */

extern	MYSQL	keysDataBase;

extern	int	nOtbugPthread;
extern	int	obProcFd;
extern	usec_t	obSyncSec, obSyncMicrosec;
extern	pth_t	fcThreadTid, tmpFileTid;

extern	char	keysTblName[], keysDbName[];

#endif
