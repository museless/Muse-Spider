#ifndef	_SPOUTB_H
#define	_SPOUTB_H


/*---------------------
	include
-----------------------*/

#include "mpctl.h"


/*---------------------
	define
-----------------------*/

#define	OTBUG_MP_NORMAL	0x2000
#define	OTBUG_MP_HASH	0x4000
#define	OTBUG_MP_THREAD	0x80000

#define	TC_LENGTH_LEN	(sizeof(int))
#define	TC_INDEX_LEN	(sizeof(int))
#define	TC_CNT_LEN	(sizeof(int))
#define	TC_NULL_LEN	(sizeof(char))
#define	TC_ENTER_LEN	TC_NULL_LEN
#define	TC_MAX_TMS_LEN	0x30

#define	TC_CNT_STRLEN	0xA

#define	TC_PERWR_LEN	(TC_CNT_STRLEN + TC_MAX_TMS_LEN + TC_ENTER_LEN)

#define	LOG_CHECK_DATA	0xFFFFFFFF


/*---------------------
	typedef
-----------------------*/

typedef	void			shmd;

typedef	struct	termlistctl	TLCTL;
typedef	struct	termcontain	TMCON;
typedef	struct	smdata		SDT, *PSDT, **PPSDT;

typedef	struct	logdata		LGDATA;
typedef	struct	tfctl		TFCTL;

typedef	struct	pidsave		PIDSAV;


/*---------------------
	struct
-----------------------*/

struct	termlistctl {
	MATOS	tcl_lock;
	MATOS	tcl_flck;
	int	tcl_cnter;	/* cnt for element */

	char	tcl_date[DATEBUF_LEN + 1];
};

struct	termcontain {
	int	tc_length;
	int	tc_index;
	int	tc_count;
};

#define	TC_FIG_LEN	(sizeof(TMCON))

struct	smdata {
	TMCON	sd_tmcon;
	char	sd_data;	/* the first byte of data, use & to get it */
};

/* for logfile */
struct	logdata {
	time_t	ld_tstart;
	time_t	ld_tend;

	int	ld_keyoff;
	int	ld_cntoff;
	int	ld_datoff;

	int	ld_check;
};

#define	LOG_HEAD_SEC	sizeof(usec_t)
#define	LOG_DATA_SIZE	sizeof(LGDATA)

/* for tmpfile */
struct	tfctl {
	pmut_t	tf_mutex;
	pcd_t	tf_cond;

	int	tf_flags;
};

struct pidsave {
	PIDSAV	*ps_next;
	pid_t	ps_pid;
};


/*---------------------
    global function
-----------------------*/

/* sp_outbug_error.c */
void	otbug_sig_error(void);
void	otbug_perror(char *errStr, int nErr);

/* sp_outbug_signal.c */
int	otbug_init_signal(void);

/* sp_outbug_db.c */
void	otbug_database_init(void);
int	otbug_dberr_dispose(void *sqlHand, char *dbName, char *pHead);
void	otbug_rewind_state(char *wStr, char *pID);

void	otbug_unlink_database(void *sqlHandler);
void	otbug_unlink_database_serv(void);

/* sp_outbug_message.c */
void	*otbug_msg_init(int messageFd);

/* sp_outbug_shm.c */
void	*otbug_shm_alloc(void *pTerms, int datLen, int *numSave);
int	otbug_shm_cmp(void *beCmped, void *cmpEr, int cmpLen);
uLong	otbug_shm_hash(void *pData, int datLen);
TMCON	*otbug_shm_mov_next(TMCON *movTm);

TLCTL	*otbug_tcler_init(mafun maFun, void *pContainer);
TLCTL	*otbug_shm_tcler_get(void *pShm);	/* arg: MSHM */
void	otbug_tcler_lock(TLCTL *pCtl);
void	otbug_tcler_unlock(TLCTL *pCtl);
void	otbug_tcler_dec(TLCTL *pCtl);
char	*otbug_tcler_date(TLCTL *pCtl);

void	otbug_tcler_inc_cnt(TLCTL *pCtl);
int	otbug_tcler_cnter(TLCTL *pCtl);
shmd	*otbug_shm_data_begin(void *shmStru);

shmd	*otbug_shm_data_begin(void *shmStru);	/* arg: MSHM */
SDT	*otbug_shm_data_fetch(SDT *pSdt);

void	otbug_tmpfile_lock(TLCTL *pCtl);
void	otbug_tmpfile_unlock(TLCTL *pCtl);
void	otbug_tmpfile_dec(TLCTL *pCtl);

#define	otbug_tcler_inc		otbug_tcler_unlock
#define	otbug_tmpfile_inc	otbug_tmpfile_unlock

/* sp_outbug_filectl.c */
int	otbug_filectl_init(void);
void	*otbug_filectl_entrance(void *toPara);
void	otbug_filectl_destroy(void *fileDes);

int	otbug_filectl_thread_creat(void);
void	otbug_filectl_thread_destroy(void);

/* sp_outbug_tool.c */
int	otbug_tools_start(void);


#endif
