#ifndef	_MMDPOOL_H
#define	_MMDPOOL_H


/* include */
#include "mmdperr.h"

/* typedef */
typedef	struct	dmp_handler	DMPH;
typedef	struct	dmp_body	DMPB;
typedef	struct	dmp_bigbody	DMPBB;

typedef	unsigned int	msize_t;
typedef	unsigned char	mpt_t;
typedef	unsigned short	merr_t;

/* struct */
struct	dmp_handler {
	DMPB	*mh_stru;

	DMPBB	*mh_big;
	DMPBB	*mh_blast;

	msize_t	mh_sizebor;	/* size border */

	msize_t	mh_cnt;		/* cnt for pool smaller than DEFAULT_BSIZE */
	merr_t	mh_err;		/* pool err num, one handler one err */
	MATOS	mh_biglock, mh_deflock;
};

struct	dmp_body {
	mpt_t	*mb_start;
	mpt_t	*mb_end;

	DMPB	*mb_next;

	msize_t	mb_size;
	msize_t	mb_left;
	msize_t	mb_taker;
};

struct	dmp_bigbody {
	mpt_t	*mbb_start;

	DMPBB	*mbb_fore;
	DMPBB	*mbb_next;
};

/* define */
#define	DEFAULT_BSIZE		0x400
#define	DEFALUT_MAX_BODY	0x10000	

/* API */
DMPH	*mmdp_create(int borderSize);
void	*mmdp_malloc(DMPH *mHand, msize_t maSize);
void	mmdp_free(DMPH *pHandler, mpt_t *pFree);
void	mmdp_free_pool(DMPH *pMfree);
void	mmdp_free_handler(DMPH *pMfree);
void	mmdp_free_all(DMPH *pMfree);
void	mmdp_reset_default(DMPH *pReset);
int	mmdp_show_size(DMPH *pMp);

#endif
