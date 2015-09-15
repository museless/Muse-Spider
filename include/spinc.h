/*Copyright (c) 2015, William Muse
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


#ifndef	_SPINC_H
#define	_SPINC_H

/*---------------------------
	include
-----------------------------*/

#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define __USE_GNU
#include <pthread.h>

#include <ctype.h>
#include <string.h>
#include <strings.h>
#include <signal.h>
#include <time.h>
#include <fcntl.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/types.h>

#include <iconv.h>
#include <errno.h>

#include "sperr.h"

#include "mato.h"
#include "mgc.h"


/*---------------------------
	define
-----------------------------*/

#define	FUN_RUN_OK	1
#define	FUN_RUN_END	0
#define	FUN_RUN_FAIL	-1

#define	FUN_RET_POS	1
#define	FUN_RET_ZERO	0
#define	FUN_RET_NEG	-1

#define	OWN_NAME_LEN	0x9

#define	CHARSET_LEN	0x10

#define	HEAD_LEN	0x30
#define	WEBFILE_LEN	0x50

#define	LATEST_LEN	0x40
#define	PATH_LEN	0x80

#define	HOST_LEN	0x100
#define	FILE_LEN	0x100
#define	URL_LEN		(HOST_LEN + FILE_LEN)

#define	RECE_DATA	0x1000

#define	DATE_CMODE	0x7

#define	SMALL_BUF	0x10
#define	MIDDLE_BUF	0x30
#define	BIG_BUF		0x100
#define	HUGE_BUF	0x1000

#define	NAMBUF_LEN	0x100
#define	FNAME_LEN	0x40
#define	PATTERN_LEN	0x20
#define	LOGBUF_LEN	0x20000
#define	LOGBASE_LEN	0x20
#define	DATEBUF_LEN	0x8

#define	NPAPER_LEN	0x40

#define	HTTP_RECALL	RECE_DATA

#define	DB_UNINITED	0x0
#define	DB_INITED	0x1

#define	FILE_AUTHOR	(S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define	SCRIPT_AUTHOR	(S_IXUSR | S_IRUSR | S_IWUSR | S_IRGRP)

/* sem project id */
#define	PROJ_PTH_CTL	0x0
#define	PROJ_SHM_CTL	0x1

/* elog */
#define	EMPTY_OBJ_STR	"Empty"
#define	FUNCTION_STR	"Function"
#define	THREAD_STR	"Thread"
#define	ERROR_STR	strerror(errno)
#define	HERROR_STR	((char *)hstrerror(h_errno))
#define	MYERR_STR(pSql)	((char *)mysql_error(pSql))

/* configure read setting */
#define	CONF_STR	0x1
#define	CONF_NUM	0x2

/* sleep time: sec */
#define	TAKE_A_NO	0x0
#define	TAKE_A_SEC	0x1
#define	TAKE_A_NOTHING	0x2
#define	TAKE_A_EYECLOSE	0x4
#define	TAKE_A_SHNAP	0x8
#define	TAKE_A_NAP	0x10
#define	TAKE_A_REST	0x20
#define	TAKE_A_SLEEP	0x40
#define	TAKE_A_LONGSLP	0x80
#define	TAKE_A_LLSLP	0x12C

#define	TINY_TIME	0x2
#define	TINY_USEC	0x80000

/* sleep time: microsec */
#define	MICSEC_PER_SEC	1000000
#define	MILLISEC_500	500000

/* socket linking */
#define	SOCK_IP		"127.0.0.1"
#define	SOCK_PORT	12345
#define	SOCK_BACKLOG	5


/*---------------------------
	typedef
-----------------------------*/

typedef	pthread_t		pth_t;
typedef	pthread_attr_t		pattr_t;

typedef	unsigned char		uChar;
typedef	unsigned int		uInt;
typedef	unsigned long		uLong;
typedef	useconds_t		usec_t;

typedef	struct	stat		STAT;
typedef	struct	tm		TMS;
typedef	struct	timeval		TMVAL;

typedef	struct	buff		BUFF;

typedef	struct sigaction	SIGAC;
typedef	sigset_t		SIGSET;

typedef	void	*(*mafun)(void *, int);
typedef	void	(*mafree)(void *);
typedef	void	(*sigfun)(int);


/*---------------------------
	struct
-----------------------------*/

struct	buff {
	void	*b_start;
	
	int	b_cap;
	int	b_size;
};


/*---------------------------
	extern data
-----------------------------*/

extern	char	kerNameBuf[], ubNameBuf[];
extern	char	tbNameBuf[], ebNameBuf[];


/*---------------------------
	global function
-----------------------------*/

/* kern_conf.c */
int	mc_conf_load(char *pUser, char *confPath);
void	mc_conf_unload(void);
int	mc_conf_read(char *findStr, int dType, void *dBuf, int dLen);
void	mc_conf_print_err(char *pFind);

/* sp_misc.c */
void	sp_stop(void);
void	sp_stop_str(void);

/* sp_pfun.c */
int	readn(int rFd, void *rBuf, size_t bCount);
int	writen(int wFd, void *wBuf, size_t bCount);

char	*strnstr(char *findBuf, char *needStr, int nLimit);
char	*strnchr(char *findBuf, char needCh, int nLimit);
int	select_read(int nSock, char *readBuf, int nRead, int nSec, int uSec);
char	*strchr_backward(char *pEnd, int nLimit, char fCh);
char	*strchr_forward(char *pEnd, int nLen, char findCh);
int	atoin(char *datBuf, int nLen);
int	read_all_file(char **pStore, char *ofName, int readOff);
int	examine_empty_string(char *exaStr);
int	socket_set_timer(int nSocket, int nSec, int uSec, int nFlags);

TMS	*time_str_extract(char *timStr);

/* fun libary */

/* dboper/dboper.c */
int	mysql_string_exist_check(void *chkSql, char *chkCom);

/* sp_bufoper.c */

/* 1 means empty
   0 means no empty
  -1 means failed */
int	buff_stru_empty(BUFF *pBuff);
/* 1 means enough 
   0 means no enough
  -1 means failed */
int	buff_size_enough(BUFF *pBuff, int nCheck);
int	buff_check_exist(BUFF *cBuff);
void	buff_stru_make_empty(BUFF *pBuff);
int	buff_now_size(BUFF *pBuff);
char	*buff_place_locate(BUFF *pBuff, int nSize);
char	*buff_place_end(BUFF *pBuff);
char	*buff_place_start(BUFF *pBuff);
void	buff_size_add(BUFF *pBuff, int addSize);

BUFF	*buff_stru_init(int nMalloc);
void	buff_stru_free_all(void *bufStru);

char	*buff_stru_strstr(BUFF *strBuf, char *needStr);

/* sp_elog.c */
int	elog_init(char *confStr);
int	elog_write(char *errStr, char *objStr1, char *objStr2);
void	elog_write_force(void);
void	elog_destroy(void);

#endif
