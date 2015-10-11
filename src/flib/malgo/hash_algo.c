/*------------------------------------------
	Source file content Thirteen part

	Part Zero:	Include
	Part One:	Define
	Part Two:	Local data
	Part Three:	Local function

	Part Four:	Hash bucket

--------------------------------------------*/

/*------------------------------------------
	Part Zero: Include
--------------------------------------------*/

#include "spinc.h"
#include "malgo.h"


/*------------------------------------------
	Part One: Define
--------------------------------------------*/

#define	hash_bucket_locate(pHbet, nKey) ((pHbet)->hb_head + nKey)
#define	hash_bucket_existed(pHbet)	((pHbet) ? 1 : 0)


/*------------------------------------------
	Part Three: Local function
--------------------------------------------*/

static	inline	int	hash_bucket_select(HBET *hbStru, void *extData, int dLen);
static		BUCKET	*hash_bucket_chose_bucket(HBET *hbStru, void *pData, int dataLen);


/*------------------------------------------
	Part Four: Hash bucket

	1. hash_bucket_init
	2. hash_bucket_find
	3. hash_bucket_insert
	4. hash_bucket_select
	5. hash_bucket_destroy

	6. hash_bucket_select_locate

--------------------------------------------*/

/*-----hash_bucket_init-----*/
HBET *hash_bucket_init(int nBucket, mafun funMalloc, mafree funFree, void *maHand,
			hbcreat funCreat, hbcmp funCmp, hbhash funHash)
{
	HBET	*pHb;
	BUCKET	*pBuck;
	int	nCnt;

	if(!funMalloc || !funCreat || !funCmp)
		return	NULL;

	if(!(pHb = malloc(sizeof(HBET))))
		return	NULL;

	if(!(pHb->hb_head = funMalloc(maHand, nBucket * sizeof(BUCKET))))
		return	NULL;

	for(pBuck = pHb->hb_head, nCnt = 0; nCnt < nBucket; nCnt++, pBuck++) {
		pBuck->bt_list = NULL;
		pBuck->bt_cnt = 0;
		mato_init(&pBuck->bt_lock, 1);
	}

	pHb->hb_mafun = funMalloc;
	pHb->hb_mafree = funFree;
	pHb->hb_mhand = maHand;

	pHb->hb_crtfun = funCreat;
	pHb->hb_cmpfun = funCmp;
	pHb->hb_hash = funHash;

	pHb->hb_nbuck = nBucket;

	return	pHb;
}


/*-----hash_bucket_insert-----*/
void *hash_bucket_insert(HBET *hbStru, void *pInsert, int datLen, void *datBuf)
{
	BELE	*newEle;
	BUCKET	*pBucket;

	if(!hash_bucket_existed(hbStru))
		return	NULL;

	if(!(newEle = hbStru->hb_mafun(hbStru->hb_mhand, sizeof(BELE))))
		return	NULL;

	if(!(newEle->be_ele = hbStru->hb_crtfun(pInsert, datLen, datBuf)))
		return	NULL;

	pBucket = hash_bucket_chose_bucket(hbStru, pInsert, datLen);

	while(!mato_dec_and_test(&pBucket->bt_lock))
		mato_inc(&pBucket->bt_lock);

	newEle->be_next = pBucket->bt_list;
	pBucket->bt_list = newEle;

	pBucket->bt_cnt++;

	mato_inc(&pBucket->bt_lock);

	return	newEle->be_ele;
}


/*-----hash_bucket_find-----*/
void *hash_bucket_find(HBET *hbFind, void *pFind, int nLen)
{
	BUCKET	*pBucket;
	BELE	*pMov;
	int	nCnt;

	if(!hash_bucket_existed(hbFind))
		return	NULL;

	pBucket = hash_bucket_chose_bucket(hbFind, pFind, nLen);

	while(!mato_dec_and_test(&pBucket->bt_lock))
		mato_inc(&pBucket->bt_lock);

	for(pMov = pBucket->bt_list, nCnt = 0; nCnt < pBucket->bt_cnt; nCnt++, pMov = pMov->be_next) {
		if(!hbFind->hb_cmpfun(pFind, pMov->be_ele, nLen)) {
			mato_inc(&pBucket->bt_lock);
			return	pMov->be_ele;
		}
	}

	mato_inc(&pBucket->bt_lock);

	return	NULL;
}


/*-----hash_bucket_select-----*/
static inline int hash_bucket_select(HBET *hbStru, void *extData, int dLen)
{
	return	((hbStru->hb_hash(extData, dLen)) % hbStru->hb_nbuck);
}


/*-----hash_bucket_destroy-----*/
void hash_bucket_destroy(HBET *freeBet)
{
	if(freeBet) {
		if(freeBet->hb_mafree)
			freeBet->hb_mafree(freeBet->hb_head);

		free(freeBet);
	}
}


/*-----hash_bucket_chose_bucket-----*/
static BUCKET *hash_bucket_chose_bucket(HBET *hbStru, void *pData, int dataLen)
{
	int	nKey = hash_bucket_select(hbStru, pData, dataLen);
	
	return	((nKey > hbStru->hb_nbuck) ? NULL : hash_bucket_locate(hbStru, nKey));
}
