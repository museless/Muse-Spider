#include "spinc.h"
#include "mmdpool.h"


/* local data */


/* local function */
static	mpt_t	*mmdp_big_malloc(DMPH *hPoint, msize_t maSize);
static	mpt_t	*mmdp_default_malloc(DMPH *hStru, msize_t nMalloc);

static	DMPBB	*mmdp_big_search(DMPBB *hStru, mpt_t *pFind);
static	DMPB	*mmdp_default_size_search(DMPB *byStru, msize_t nSize);
static	DMPB	*mmdp_default_block_search(DMPB *begBlock, mpt_t *pLoct);


/*------------------------------------------
	Source file content Seven part

	Part Zero:	Define
	Part One:	API
	Part Two:	Slave Function
	Part Three:	Block search
	Part Four:	Error handler

--------------------------------------------*/

/*------------------------------------------
	Part Zero: Define
--------------------------------------------*/

#define	mmdp_handler_set_empty(hStru) { \
	hStru->mh_big = hStru->mh_blast = NULL; \
	hStru->mh_stru = NULL; \
	hStru->mh_err = MMDP_NO_ERROR; \
	hStru->mh_cnt = 0; \
}


#define	mmdp_block_end_adrr(pMpool) \
	(pMpool->mb_start + pMpool->mb_size)


#define	mmdp_reset_block(pHandler) { \
	pHandler->mb_end = pHandler->mb_start; \
	pHandler->mb_left = pHandler->mb_size; \
	pHandler->mb_taker = 0; \
}


/*------------------------------------------
	Part One: API

	1. mmdp_create
	2. mmdp_malloc
	3. mmdp_free
	4. mmdp_free_pool
	5. mmdp_free_handler
	6. mmdp_free_all
	7. mmdp_reset_default
	8. mmdp_show_size

--------------------------------------------*/

/*-----mmdp_create-----*/
DMPH *mmdp_create(int borderSize)
{
	DMPH	*pMph;

	if(!(pMph = malloc(sizeof(DMPH))))
		return	NULL;

	mmdp_handler_set_empty(pMph);

	pMph->mh_sizebor = (borderSize < DEFAULT_BSIZE) ? DEFAULT_BSIZE : (((borderSize) >>  1) << 1);

	mato_init(&pMph->mh_biglock, 1);
	mato_init(&pMph->mh_deflock, 1);

	return	pMph;
}


/*-----mmdp_malloc-----*/
void *mmdp_malloc(DMPH *mHand, msize_t maSize)
{
	mpt_t	*pMem;

	if(maSize > mHand->mh_sizebor)
		return	mmdp_big_malloc(mHand, maSize);

	while(!mato_dec_and_test(&mHand->mh_deflock))
		mato_inc(&mHand->mh_deflock);

	pMem = mmdp_default_malloc(mHand, maSize);

	mato_inc(&mHand->mh_deflock);

	return	pMem;
}


/*-----mmdp_free-----*/
void mmdp_free(DMPH *pHandler, mpt_t *pFree)
{
	DMPBB	*pBig;
	DMPB	*pBlock;

	if((pBig = mmdp_big_search(pHandler->mh_big, pFree))) {
		while(!mato_dec_and_test(&pHandler->mh_biglock))
			mato_inc(&pHandler->mh_biglock);

		if(pBig->mbb_fore)	pBig->mbb_fore->mbb_next = pBig->mbb_next;
			else		pHandler->mh_big = pBig->mbb_next;

		if(pBig->mbb_next)	pBig->mbb_next->mbb_fore = pBig->mbb_fore;
			else		pHandler->mh_blast = pBig->mbb_fore;

		mato_inc(&pHandler->mh_biglock);

		free(pBig->mbb_start);
		free(pBig);

		return;
	}

	if((pBlock = mmdp_default_block_search(pHandler->mh_stru, pFree))) {
		if(!(--pBlock->mb_taker)) {
			while(!mato_dec_and_test(&pHandler->mh_deflock))
				mato_inc(&pHandler->mh_deflock);

			mmdp_reset_block(pBlock);
			mato_inc(&pHandler->mh_deflock);
		}
	}
}


/*-----mmdp_free_pool-----*/
void mmdp_free_pool(DMPH *pMfree)
{
	DMPB	*pBnext, *pBmov;
	DMPBB	*bigNext, *bigMov;

	for(pBmov = pMfree->mh_stru; pBmov; pBmov = pBnext) {
		if(pBmov->mb_start)
			free(pBmov->mb_start);

		pBnext = pBmov->mb_next;
		free(pBmov);
	}

	for(bigMov = pMfree->mh_big; bigMov; bigMov = bigNext) {
		if(bigMov->mbb_start)
			free(bigMov->mbb_start);

		bigNext = bigMov->mbb_next;
		free(bigMov);
	}

	mmdp_handler_set_empty(pMfree);
}


/*-----mmdp_free_handler-----*/
void mmdp_free_handler(DMPH *pMfree)
{
	free(pMfree);
}


/*-----mmdp_free_all-----*/
void mmdp_free_all(DMPH *pMfree)
{
	mmdp_free_pool(pMfree);
	mmdp_free_handler(pMfree);
}


/*-----mmdp_reset_default-----*/
void mmdp_reset_default(DMPH *pReset)
{
	DMPB	*pMov;

	for(pMov = pReset->mh_stru; pMov; pMov = pMov->mb_next)
		mmdp_reset_block(pMov);
}


/*-----mmdp_show_size-----*/
int mmdp_show_size(DMPH *pMp)
{
	return	pMp->mh_sizebor;
}


/*------------------------------------------
	Part Two: Slave Function

	1. mmdp_big_malloc
	2. mmdp_default_malloc

--------------------------------------------*/

/*-----mmdp_big_malloc-----*/
static mpt_t *mmdp_big_malloc(DMPH *hPoint, msize_t maSize)
{
	DMPBB	*bigStru;

	if((bigStru = malloc(sizeof(DMPBB))) == NULL)
		return	NULL;

	if((bigStru->mbb_start = malloc(maSize)) == NULL)
		return	NULL;

	bigStru->mbb_next = NULL;

	while(!mato_dec_and_test(&hPoint->mh_biglock))
		mato_inc(&hPoint->mh_biglock);

	if(hPoint->mh_blast) {
		hPoint->mh_blast->mbb_next = bigStru;
		bigStru->mbb_fore = hPoint->mh_blast;
		hPoint->mh_blast = bigStru;

	} else {
		hPoint->mh_big = hPoint->mh_blast = bigStru;
		bigStru->mbb_fore = NULL;
	}

	mato_inc(&hPoint->mh_biglock);

	return	bigStru->mbb_start;
}


/*-----mmdp_default_malloc-----*/
static mpt_t *mmdp_default_malloc(DMPH *hStru, msize_t nMalloc)
{
	DMPB	*pBody;
	mpt_t	*pRet;

	if(!(pBody = mmdp_default_size_search(hStru->mh_stru, nMalloc))) {
		if(!(pBody = malloc(sizeof(DMPB))))
			return	NULL;

		if(!(pBody->mb_start = pBody->mb_end = malloc(hStru->mh_sizebor)))
			return	NULL;

		pBody->mb_left = pBody->mb_size = hStru->mh_sizebor;
		pBody->mb_taker = 0;
		pBody->mb_next = hStru->mh_stru;
		hStru->mh_stru = pBody;

		hStru->mh_cnt++;
	}

	pRet = pBody->mb_end;
	pBody->mb_end += nMalloc;
	pBody->mb_left -= nMalloc;
	pBody->mb_taker++;

	return	pRet;
}


/*------------------------------------------
	Part Three: Block search

	1. mmdp_big_search
	2. mmdp_default_size_search
	3. mmdp_default_block_search

--------------------------------------------*/

/*-----mmdp_big_search-----*/
static DMPBB *mmdp_big_search(DMPBB *hStru, mpt_t *pFind)
{
	while(hStru) {
		if(hStru->mbb_start == pFind)
			return	hStru;

		hStru = hStru->mbb_next;
	}

	return	NULL;
}


/*-----mmdp_default_size_search-----*/
static DMPB *mmdp_default_size_search(DMPB *byStru, msize_t nSize)
{
	while(byStru) {
		if(byStru->mb_left >= nSize)
			return	byStru;

		byStru = byStru->mb_next;
	}

	return	NULL;
}


/*-----mmdp_default_block_search-----*/
static DMPB *mmdp_default_block_search(DMPB *begBlock, mpt_t *pLoct)
{
	for(; begBlock; begBlock = begBlock->mb_next) {
		if(pLoct >= begBlock->mb_start && pLoct <= mmdp_block_end_adrr(begBlock))
			return	begBlock;
	}

	return	NULL;
}


/*------------------------------------------
	Part Four: Error handler

	1. 

--------------------------------------------*/

