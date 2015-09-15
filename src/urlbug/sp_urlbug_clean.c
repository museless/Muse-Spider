/* Copyright (c) 2015, William Muse
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


/*------------------------------------------
	Source file content Five part

	Part Zero:	Include
	Part One:	Define
	Part Two:	Global data
	Part Three:	Local function

	Part Four:	Cleaning function

--------------------------------------------*/

/*------------------------------------------
	Part Zero: Include
--------------------------------------------*/

#include "spinc.h"
#include "spmpool.h"
#include "spnet.h"
#include "spdb.h"
#include "spmsg.h"
#include "spuglobal.h"


/*------------------------------------------
	Part One: Global data
--------------------------------------------*/

BUFF	*urlBufStu;
WEBIN	*urlSaveList;
WPOOL	*contStorePool, *urlStorePool;
MGCH	*urlGarCol;
SOSET	*urlMsgSet;

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

int	initMsgFlags, nRunPthread;
MATOS	writeStoreLock;

int	nFbWord = sizeof(forbitStrList) / sizeof(forbitStrList[0]);
int	webPattNum = sizeof(perWebPatt) / sizeof(perWebPatt[0]);


/*------------------------------------------
	Part Four: Cleaning function

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
