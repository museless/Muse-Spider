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


#ifndef	_SPURLB_H
#define	_SPURLB_H


/*---------------------------
	define
-----------------------------*/

#define	RUN_ONCE		0x0
#define	RUN_PERN		0x1

#define	UBUG_NREAD		0x80

#define	UBUG_PTHREAD_MIN	0x1
#define	UBUG_PTHREAD_MAX	0x4


/*---------------------------
	global function
-----------------------------*/

/* sp_urlbug.c */
void	ubug_time_change(void);
int	ubug_send_message(int nSock);

/* sp_urlbug_db.c */
void	ubug_init_database(void);
void	ubug_create_dbtable(void);
int	ubug_url_review(char *pUrl, int uLen);

void	ubug_tran_db(char *pUrl, int uLen, int nPattern);
void	ubug_tran_db_force(void);
int	ubug_tran_db_real(void);

int	ubug_dberr_dispose(MYSQL *sHandler, char *withStr);

/* sp_urlbug_disc.c */
void	ubug_print_help(void);

void	ubug_disc_init(void);

void	ubug_tran_disc(char *pUrl, int uLen, int pattWay);
void	ubug_tran_disc_force(void);

int	ubug_tran_disc_real(void);

/* sp_urlbug_signal.c */
void	ubug_init_signal(void);
void	ubug_signal_handler(int nSign);

/* sp_urlbug_clean.c */
void	ubug_free_weblist(void *pNull);
void	ubug_db_clean(void *pNULL);

/* sp_urlbug_msg.c */
void	*ubug_msg_init(void);

/* sp_urlbug.err.c */
void	ubug_sig_error(void);
void	ubug_perror(char *errStr, int nErr);

#endif
