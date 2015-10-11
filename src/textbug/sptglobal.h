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
extern	void	*txbugMsgSet;	/* MSGSET */
extern	void	*tbThreadPool;	/* PTHPOOL */

extern	MYSQL	urlDataBase;
extern	int	nTxbugPthread, nLoadLimit;
extern	int	textProcFd;
extern	short	nToleError, cSyncTime;
extern	char	urlsTblName[], urlsDbName[];
extern	char	newsTblName[], newsDbName[];
extern	char	charSetBuf[][CHARSET_LEN];

extern	MATOS	writeDataLock;
extern	TSET	textbugRunSet;
extern	MGCO	txbugResCol;

#endif
