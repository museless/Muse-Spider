#ifndef	_SPUBDATA_H
#define	_SPUBDATA_H


/*---------------------------
	typedef
-----------------------------*/

/* typedef struct */
typedef struct  fbword  FBSTR;
typedef struct  ubset   UBSET;

/* typedef function */
typedef void    (*fn_ent)(WEBIN *);
typedef int     (*fn_dway)(WEBIN *);
typedef void    (*fn_init)(void);               /* module init function */
typedef void    (*fn_st)(void *, char *, int); /* str = string tran */
typedef void    (*fn_stf)(void);                /* str = string tran force */
typedef int     (*fn_chk)(char *, int);

/*---------------------------
	struct
-----------------------------*/

struct fbword {
        char    *fb_str;
        int     fb_len;
};

/* urlbug setting */
struct ubset {
	fn_ent	ubs_fent;
	fn_dway	ubs_dway;

	fn_init	ubs_init;
	fn_st	ubs_fst;
	fn_stf	ubs_fstf;
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
extern	MSGSET	*urlMsgSet;

extern	void	*ubugThreadPool;	/* PTHPOOL (mpctl.h) */

extern	int	nFbWord, webPattNum;
extern	int	procCommuFd, nRunPthread, urlMaxLen;

extern	char	confNameBuf[], urlTabName[];

extern	MATOS	writeStoreLock;

extern	FBSTR	forbitStrList[];
extern	char	perWebPatt[][PATTERN_LEN];

#endif
