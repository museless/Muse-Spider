#include "spinc.h"
#include "mgc.h"


/*------------------------------------------
	Part One: Mgc list control

	1. mgc_init
	2. mgc_add
	3. mgc_all_clean

--------------------------------------------*/

/*-----mgc_init-----*/
MGCH *mgc_init(void)
{
	MGCH	*newGc;

	if((newGc = sbrk(sizeof(MGCH))) == NULL)
		return	NULL;

	newGc->mgch_num = 0;
	newGc->mgch_end = newGc->mgch_list = NULL;

	return	newGc;
}


/*-----mgc_add-----*/
int mgc_add(MGCH *pMgch, void *pObj, gcfun pHandler)
{
	MGC	*pNew;

	if(!pObj || !pHandler || (pNew = sbrk(sizeof(MGC))) == NULL)
		return	MGC_FAILED;

	pNew->mgc_obj = pObj;
	pNew->mgc_cleaner = pHandler;
	pNew->mgc_next = NULL;
	pMgch->mgch_num++;

	if(!pMgch->mgch_list)	pMgch->mgch_list = pNew;
		else		pMgch->mgch_end->mgc_next = pNew;

	pMgch->mgch_end = pNew;

	return	MGC_OK;
}


/*-----mgc_all_clean-----*/
void mgc_all_clean(MGCH *pMgch)
{
	MGC	*pMov;
	int	nCir;

	for(pMov = pMgch->mgch_list, nCir = 0; pMov && nCir < pMgch->mgch_num; nCir++, pMov = pMov->mgc_next)
		pMov->mgc_cleaner(pMov->mgc_obj);
}


/*------------------------------------------
	Part Two: Mgc object control

	1. mgc_one_init
	2. mgc_one_add
	3. mgc_one_clean

--------------------------------------------*/

/*-----mgc_one_init-----*/
int mgc_one_init(MGCO *pMgco, gcfun pCleaner, int nLimit)
{
	if(!pMgco || !pCleaner)
		return	MGC_FAILED;

	pMgco->mgco_obj = NULL;
	pMgco->mgco_cleaner = pCleaner;
	pMgco->mgco_tlimit = nLimit;
	
	mato_init(&pMgco->mgco_lock, 1);

	return	MGC_OK;
}

/*-----mgc_one_add-----*/
int mgc_one_add(MGCO *pMgco, void *pObj)
{
	if(!pObj || !pMgco)
		return	MGC_FAILED;

	pMgco->mgco_obj = pObj;

	return	MGC_OK;
}


/*-----mgc_one_clean-----*/
void mgc_one_clean(MGCO *pMgco)
{
	int	nCount = 0;

	while(nCount++ < pMgco->mgco_tlimit && !mato_sub_and_test(&pMgco->mgco_lock, 1)) {
		mato_inc(&pMgco->mgco_lock);
		sleep(SLEEP_A_SEC);
	}

	if(nCount == pMgco->mgco_tlimit)
		return;

	if(pMgco->mgco_obj) {
		pMgco->mgco_cleaner(pMgco->mgco_obj);
		pMgco->mgco_obj = NULL;
	}

	mato_inc(&pMgco->mgco_lock);
}
