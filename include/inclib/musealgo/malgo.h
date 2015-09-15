#ifndef	MALGO_H
#define	MALGO_H

/*-----------------------------
-------------------------------

	Sort algorithm

-------------------------------
-------------------------------*/

/*-----------------------------
	Shell sort
-------------------------------*/

/* define */
#define	SS_MIN_INPUT	0x2

/* typedef */
typedef	struct	shellsort	SSORTS;

typedef	void	*(*ssmov)(void *pHandler, void *pData, int nMov);
typedef	void	*(*ssass)(void *pHandler);
typedef	void	*(*ssprog)(void **pHandler, void *pData);
typedef	void	(*sscmre)(void *pSrc, void *pCmp);	/* cmp and replace */

/* struct */
struct	shellsort {
	ssmov	sst_mv;
	ssass	sst_assign;
	ssprog	sst_progress;
	sscmre	sst_cmprep;

	mafree	sst_free;
};

/* global function */
SSORTS	*sp_shell_sort_init(ssmov smFun, ssass smAssign, ssprog smProgress, sscmre smCmprep,
			mafun fMalloc, mafree freeFun, void *mHandler);
void	sp_shell_sort(SSORTS *ssStru, void *sortBuf, int nLimit);
void	sp_shell_sort_destroy(void *ssStru);


/*-----------------------------
-------------------------------

	Hash algorithm

-------------------------------
-------------------------------*/

/*-----------------------------
	Hash bucket
-------------------------------*/

/* typedef */
typedef	void	*(*hbcreat)(void *, int, void *);

/* the first was cmped, the second is cmpper */
typedef	int	(*hbcmp)(void *, void *, int);
typedef	uLong	(*hbhash)(void *, int);

typedef	struct	belement	BELE;
typedef	struct	bucket		BUCKET;
typedef	struct	hashbucket	HBET;

/* struct */
struct	belement {
	void	*be_ele;
	BELE	*be_next;
};

struct	bucket {
	BELE	*bt_list;

	MATOS	bt_lock;
	int	bt_cnt;
};

struct	hashbucket {
	BUCKET	*hb_head;

	mafun	hb_mafun;
	mafree	hb_mafree;
	void	*hb_mhand;

	hbcreat	hb_crtfun;
	hbcmp	hb_cmpfun;
	hbhash	hb_hash;

	int	hb_nbuck;
};

/* global function */
HBET	*hash_bucket_init(int nBucket, mafun funMalloc, mafree funFree, void *maHand,
			hbcreat funCreat, hbcmp funCmp, hbhash funHash);
void	*hash_bucket_insert(HBET *hbStru, void *pInsert, int datLen, void *datBuf);
void	*hash_bucket_find(HBET *hbFind, void *pFind, int nLen);
void	hash_bucket_destroy(HBET *freeBet);


#endif
