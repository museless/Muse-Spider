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
#include "spdb.h"
#include "spextb.h"
#include "speglobal.h"


/* global data */
MGCH	*exbGarCol;
BUFF	*extSaveBuf;

void	*extbugMsgSet;			/* SOSET */
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
int	initMsgFlags, nExbugPthead, upMaxTerms, nKlistSize;
