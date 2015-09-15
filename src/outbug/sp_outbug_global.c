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
#include "spoglobal.h"
#include "spoutb.h"


/* global data */
MGCH	*obGarCol;
MGCO	otResCol;

void	*obDynaMp, *obThreadMp;	/* DMPH: mmdpool.h */
void	*obMsgSet;		/* SOSET: spmsg.h */
void	*obShmContainer;	/* MSHM: mshm.h */
void	*obHashBucket;		/* HBET: malgo.h */
void	*obShmCtler, *obTimLock;/* TLCTL: spoutb.h */
void	*obThreadPool;		/* PTHPOOL: mpctl.h */

MYSQL	keysDataBase;
int	nOtbugPthread;
int	initMsgFlags;
usec_t	obSyncSec, obSyncMicrosec;
pth_t	fcThreadTid, tmpFileTid;

char	keysTblName[SQL_TABNAME_LEN], keysDbName[SQL_DBNAME_LEN];

