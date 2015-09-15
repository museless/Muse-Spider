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


#ifndef	_SPUBDATA_H
#define	_SPUBDATA_H


/*---------------------------
	typedef
-----------------------------*/

/* typedef struct */
typedef	struct	fbword	FBSTR;
typedef	struct	ubset	UBSET;

/* typedef function */
typedef	void	(*fn_ent)(WEBIN *);
typedef	int	(*fn_dway)(WEBIN *);
typedef	void	(*fn_init)(void);		/* module init function */
typedef	void	(*fn_st)(char *, int, int);	/* str = string tran */
typedef	void	(*fn_stf)(void);		/* str = string tran force */
typedef	int	(*fn_str)(void);		/* str = string tran real */
typedef	int	(*fn_chk)(char *, int);

/*---------------------------
	struct
-----------------------------*/

struct fbword {
	char	*fb_str;
	int	fb_len;
};

/* urlbug setting */
struct ubset {
	fn_ent	ubs_fent;
	fn_dway	ubs_dway;

	fn_init	ubs_init;
	fn_st	ubs_fst;
	fn_stf	ubs_fstf;
	fn_str	ubs_fstr;
	fn_chk	ubs_chk;

	int	ubs_rtime;
};

/*---------------------------
	global data
-----------------------------*/

extern	WEBIN	*urlSaveList;
extern	WPOOL	*contStorePool, *urlStorePool;
extern	BUFF	*urlBufStu;
extern	MGCH	*urlGarCol;
extern	UBSET	urlRunSet;
extern	MYSQL	urlDataBase;
extern	SOSET	*urlMsgSet;

extern	void	*ubugThreadPool;	/* PTHPOOL (mpctl.h) */

extern	int	nFbWord, webPattNum;
extern	int	initMsgFlags, nRunPthread;

extern	char	confNameBuf[], urlTabName[];

extern	MATOS	writeStoreLock;

extern	FBSTR	forbitStrList[];
extern	char	perWebPatt[][PATTERN_LEN];

#endif
