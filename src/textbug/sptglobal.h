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


#ifndef	_SPTGLOBAL_H
#define	_SPTGLOBAL_H

/* typedef */
typedef	struct	txbset	TSET;
typedef	struct	alword	ALWORD;
typedef	union	txhand	TXHAND;

typedef	void		(*finit)(void);
typedef	void		(*fwt)(void *);
typedef	void		(*fwtf)(void *, void *, void *);
typedef	int		(*fwtr)(void *, void *);
typedef	void		(*fctime)(void);

/* struct */
struct	txbset {
	finit	ts_init;
	fwt	ts_wt;
	fwtf	ts_wtf;
	fwtr	ts_wtr;
	fctime	ts_fctim;

	void	*ts_hand;
};

struct	alword {
	char	*al_str;
	int	al_len;
};

union	txhand {
	MYSQL	tx_sql;
	int	tx_fd;
};


/* global data */
extern	MGCH	*textGarCol;
extern	SCONF	*urlConfigSave;
extern	WPOOL	*contMemPool, *tsMemPool;
extern	BUFF	*contStoreBuf, *urlSaveBuf;
extern	TXHAND	txTranHand;

extern	void	*txbugRunMp;	/* DMPH */
extern	void	*txbugMsgSet;	/* SOSET */
extern	void	*tbThreadPool;	/* PTHPOOL */

extern	MYSQL	urlDataBase;
extern	int	nTxbugPthread, nLoadLimit;
extern	int	initMsgFlags;
extern	short	nToleError, cSyncTime;
extern	char	urlsTblName[], urlsDbName[];
extern	char	newsTblName[], newsDbName[];
extern	char	charSetBuf[][CHARSET_LEN];

extern	MATOS	writeDataLock;
extern	TSET	textbugRunSet;
extern	MGCO	txbugResCol;

#endif
