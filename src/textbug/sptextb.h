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


#ifndef	_SPTEXTB_H
#define	_SPTEXTB_H

/*---------------------
	include
-----------------------*/

#include "sppatt.h"

/*---------------------
	define
-----------------------*/

#define	NEWS_BUFF_SIZE	0x12000
#define	CONT_BUFF_SIZE	0x20000

#define	TEXT_NREAD	0x80

/* mempool argument */
#define	PROC_MP_MAX	0x1000

/* for texting */
#define	UTF8_WHEAD	0xE0
#define	UTF8_WORD_LEN	0x3

#define	GB2312_BEG	0xA1
#define	GB2312_END	0xF7
#define	GB2312_WORD_LEN	0x2

#define	GBK_BEG		0x81
#define	GBK_END		0xFE
#define	GBK_WORD_LEN	0x2

#define	RET_VALUE_MASK	0x3
#define	RET_WORD_NO_ACC	0x0

/* define function */
#define	txbug_ending(pSrc)	\
	(*pSrc == '/' || *pSrc == ' ' || *pSrc == '_' || *pSrc == '|' || \
	*pSrc == '-' || *pSrc == '\n' || *pSrc == '\r')


/*---------------------
	global fun
-----------------------*/

/* sp_textbug.c */
void	txbug_keep_working(void *pResult);
void	txbug_time_change(void);
int	txbug_send_message(int nSock);

/* sp_textbug_signal.c */
void	txbug_signal_init(void);
void	txbug_signal_handler(int nSignal);

/* sp_textbug_clean.c */
void	txbug_pool_free(TEXT *textPoint);

void	txbug_db_unlink_url(void);
void	txbug_db_unlink_cont(void);
void	txbug_db_close(void);

/* sp_textbug_disc.c */
void	module_disc_init(void);

void	txbug_tran_disc(void *pText);
void	txbug_tran_disc_force(void *writeFd, MATOS *atoLock, BUFF *pBuff);
int	txbug_tran_disc_real(void *writeFd, BUFF *pBuff);

void	txbug_print_help(void);

/* sp_textbug_db.c */
void	txbug_database_init(void);
void	txbug_db_rewind_state(char *wStr, char *pID);
int	txbug_dberr_dispose(void *sqlHand, char *dbName, char *pHead);

void	module_database_init(void);
void	txbug_tran_db(void *txtStru);
void	txbug_tran_db_force(void *sqlHandler, MATOS *atoLock, BUFF *bufStru);
int	txbug_tran_db_news(void *dbHandler, BUFF *pBuff);

void	txbug_create_news_table(void);
void	txbug_create_view(void);
void	txbug_create_nv(void);

/* sp_textbug_message.c */
void	*txbug_msg_init(void);
void	txbug_wait_arouse(void *msgSet);

/* sp_textbug_err.c */
void	txbug_sig_error(void);
void	txbug_perror(char *errStr, int nErr);

/* sp_textbug_httpstate.c */
int	txbug_dispose_http301(char *retBuf, TEXT *pText);

#endif
