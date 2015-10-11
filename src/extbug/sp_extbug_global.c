#include "spinc.h"
#include "spdb.h"
#include "spextb.h"
#include "speglobal.h"


/* global data */
MGCH	*exbGarCol;
BUFF	*extSaveBuf;

void	*extbugMsgSet;			/* MSGSET */
void	*threadMemPool, *procMemPool;	/* DMPH */
void	*ebSemControl;			/* MSEM */

EXBSET	exbRunSet;
MYSQL	dbNewsHandler, dbDicHandler, dbKeysHandler;
CLISTS	charTermList, charHeadSave;

MGCO	extResCol;

char	sqlSeleCom[SQL_LICOM_LEN];

char	tblNewsName[SQL_TABNAME_LEN], dbNewsName[SQL_DBNAME_LEN];
char	tblWordName[SQL_TABNAME_LEN], dbDicName[SQL_DBNAME_LEN];
char	tblKeysName[SQL_TABNAME_LEN], dbKeysName[SQL_DBNAME_LEN];

MATOS	pthreadCtlLock, freeCtlLock, nPaperLock, dicDbLock;

uLong	tPaperNum;
int	exbugIpcFd, nExbugPthead, upMaxTerms, nKlistSize;

