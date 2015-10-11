#ifndef	_SPURLB_H
#define	_SPURLB_H


/*---------------------------
	define
-----------------------------*/

#define RUN_ONCE                0x0
#define RUN_PERN                0x1

#define UBUG_NREAD              0x80

#define UBUG_PTHREAD_MIN        0x1
#define UBUG_PTHREAD_MAX        0x4


/*---------------------------
	global function
-----------------------------*/

/* sp_urlbug.c */
void    ubug_time_change(void);
int     ubug_send_message(int msgFd);

/* sp_urlbug_db.c */
void    ubug_init_database(void);
void    ubug_create_dbtable(void);

void    ubug_tran_db(void *pInfo, char *pUrl, int uLen);        /* pInfo (WEBIN) */
void    ubug_tran_db_whole(void);
void    ubug_tran_db_force(BUFF *pBuff);
int     ubug_tran_db_real(BUFF *pBuff);

int     ubug_dberr(MYSQL *sHandler, char *withStr);

/* sp_urlbug_disc.c */
void    ubug_print_help(void);

void    ubug_disc_init(void);

void    ubug_tran_disc(void *pInfo, char *pUrl, int uLen);
void    ubug_tran_disc_whole(void);
void    ubug_tran_disc_force(BUFF *pBuff);

int     ubug_tran_disc_real(BUFF *pBuff);

/* sp_urlbug_signal.c */
void    ubug_init_signal(void);
void    ubug_signal_handler(int nSign);

/* sp_urlbug_clean.c */
void    ubug_free_weblist(void *pNull);
void    ubug_db_clean(void *pNULL);

/* sp_urlbug_msg.c */
void    *ubug_msg_init(int msgFd);

/* sp_urlbug.err.c */
void    ubug_sig_error(void);
void    ubug_perror(char *errStr, int nErr);

#endif
