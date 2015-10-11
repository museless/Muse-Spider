#include "spinc.h"
#include "spmpool.h"
#include "mmdpool.h"
#include "spnet.h"
#include "spdb.h"
#include "sptglobal.h"


/* global data */
MGCH	*textGarCol;
SCONF	*urlConfigSave;
WPOOL	*contMemPool, *tsMemPool;
MYSQL	urlDataBase;
TXHAND	txTranHand;

void	*txbugRunMp;	/* DMPH */
void	*txbugMsgSet;	/* MSGSET */
void	*tbThreadPool;	/* PTHPOOL */

int	nTxbugPthread, nLoadLimit;
int	textProcFd;
short	nToleError, cSyncTime;
char	urlsTblName[SQL_TABNAME_LEN], urlsDbName[SQL_DBNAME_LEN];
char	newsTblName[SQL_TABNAME_LEN], newsDbName[SQL_DBNAME_LEN];

char	charSetBuf[][CHARSET_LEN] = {{"utf-8"}, {"gb2312"}};

MATOS	writeDataLock;
BUFF	*contStoreBuf, *urlSaveBuf;
TSET	textbugRunSet;

MGCO	txbugResCol;
