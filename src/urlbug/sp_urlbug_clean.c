#include "spinc.h"
#include "spmpool.h"
#include "spnet.h"
#include "spsock.h"
#include "spdb.h"
#include "spmsg.h"
#include "spuglobal.h"


/* global data */
BUFF	*urlBufStu;
WEBIN	*urlSaveList;
WPOOL	*contStorePool, *urlStorePool;
MGCH	*urlGarCol;
MSGSET	*urlMsgSet;

void	*ubugThreadPool;/* PTHPOOL */

UBSET	urlRunSet;
MYSQL	urlDataBase;

char	confNameBuf[PATH_LEN];
char	urlTabName[SMALL_BUF];

char	perWebPatt[][PATTERN_LEN] = {
		"mofcom.gov.cn", "p5w.net", "ccstock.cn", "eeo.com.cn", "nbd.com.cn", "time-weekly.com", "guancha.cn",
		"cneo.com.cn", "caixin.com", "cankaoxiaoxi.com", "youth.cn", "stcn.com", "cnr.cn", "people.com.cn",
		"cnfol.com", "qq.com", "xinhuanet.com", "sina.com.cn", "ifeng.com", "china.com.cn", "cntv.cn", 
		"163.com", "hexun.com", "eastmoney.com", "chinanews.com", "yangtse.com", "bjnews.com.cn", 
		"ynet.com", "chinadaily.com.cn", "cfi.cn", "gmw.cn", "southcn.com"};

FBSTR	forbitStrList[] = {{"ad", 2}, {"blog", 4}, {"video", 5}, {"bbs", 3}, {"fashion", 7}, {"sports", 6},
			   {"travel", 6}, {"bschool", 7}, {"shoucang", 8}, {"pxpt", 4}, {"auto", 4}, {"tv", 2},
			   {"vhouse", 6}, {"book", 4}, {"bj", 2}, {"photo", 5}, {"lady", 4}, {"kb", 2}, {"pic", 3}};

int	procCommuFd, nRunPthread, urlMaxLen;
MATOS	writeStoreLock;

int	nFbWord = sizeof(forbitStrList) / sizeof(forbitStrList[0]);
int	webPattNum = sizeof(perWebPatt) / sizeof(perWebPatt[0]);


/*------------------------------------------
	Part Zero: Cleaning function

	1. ubug_free_weblist
	2. ubug_db_clean

--------------------------------------------*/

/*-----ubug_free_weblist-----*/
void ubug_free_weblist(void *pNull)
{
	WEBIN	*pList = urlSaveList;

	while(urlSaveList != NULL) {
		pList = urlSaveList->w_next;
		free(urlSaveList);
		urlSaveList = pList;
	}
}


/*-----ubug_db_clean-----*/
void ubug_db_clean(void *pNULL)
{
	mysql_close(&urlDataBase);
	mysql_library_end();
}
