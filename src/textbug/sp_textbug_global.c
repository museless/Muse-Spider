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


#include "spinc.h"
#include "spmpool.h"
#include "mmdpool.h"
#include "spnet.h"
#include "spdb.h"
#include "sptglobal.h"


/* global data */
MGCH	*textGarCol;
MGCO	txbugResCol;
SCONF	*urlConfigSave;
WPOOL	*contMemPool, *tsMemPool;
MYSQL	urlDataBase;
TXHAND	txTranHand;

void	*txbugRunMp;	/* DMPH */
void	*txbugMsgSet;	/* SOSET */
void	*tbThreadPool;	/* PTHPOOL */

int	nTxbugPthread, nLoadLimit;
int	initMsgFlags;
short	nToleError, cSyncTime;
char	urlsTblName[SQL_TABNAME_LEN], urlsDbName[SQL_DBNAME_LEN];
char	newsTblName[SQL_TABNAME_LEN], newsDbName[SQL_DBNAME_LEN];

char	charSetBuf[][CHARSET_LEN] = {{"utf-8"}, {"gb2312"}};

MATOS	writeDataLock;
BUFF	*contStoreBuf, *urlSaveBuf;
TSET	textbugRunSet;

