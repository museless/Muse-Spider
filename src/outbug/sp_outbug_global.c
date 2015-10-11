#include "spinc.h"
#include "spmpool.h"
#include "mmdpool.h"
#include "spnet.h"
#include "spdb.h"
#include "spoglobal.h"
#include "spoutb.h"


/* global data */
MGCH	*obGarCol;
MGCO	otResCol;

void	*obDynaMp, *obThreadMp;	/* DMPH: mmdpool.h */
void	*obMsgSet;		/* MSGSET: spmsg.h */
void	*obShmContainer;	/* MSHM: mshm.h */
void	*obHashBucket;		/* HBET: malgo.h */
void	*obShmCtler, *obTimLock;/* TLCTL: spoutb.h */
void	*obThreadPool;		/* PTHPOOL: mpctl.h */

MYSQL	keysDataBase;
int	nOtbugPthread;
int	obProcFd;
usec_t	obSyncSec, obSyncMicrosec;
pth_t	fcThreadTid, tmpFileTid;

char	keysTblName[SQL_TABNAME_LEN], keysDbName[SQL_DBNAME_LEN];

