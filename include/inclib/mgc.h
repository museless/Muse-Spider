#ifndef	_MUSE_GC_H
#define	_MUSE_GC_H


/* define */
#define	MGC_OK		1
#define	MGC_END		0
#define	MGC_FAILED	-1

#define	TRY_A_LIMIT	0x8
#define	TRY_MORE	0x10
#define	TRY_BIGGER	0x20

#define	SLEEP_A_SEC	0x1

#define	NULL_POINT	((void *)1)

/* typedef */
typedef	void	(*gcfun)(void *);
typedef	struct	musegch	MGCH;
typedef	struct	musegc	MGC;

typedef	struct	musegco	MGCO;

/* struct */
struct	musegch {
	int	mgch_num;
	MGC	*mgch_list;
	MGC	*mgch_end;
};

struct	musegc {
	MGC	*mgc_next;

	void	*mgc_obj;
	gcfun	mgc_cleaner;
};

struct	musegco {
	void	*mgco_obj;
	gcfun	mgco_cleaner;

	MATOS	mgco_lock;
	int	mgco_tlimit;
};

/* global function */
MGCH	*mgc_init(void);
int	mgc_add(MGCH *pMgch, void *pObj, gcfun pHandler);
void	mgc_all_clean(MGCH *pMgch);

int	mgc_one_init(MGCO *pMgco, gcfun pCleaner, int nLimit);
int	mgc_one_add(MGCO *pMgco, void *pObj);
void	mgc_one_clean(MGCO *pMgco);

#endif
