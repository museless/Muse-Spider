/*------------------------------------------
	Source file content Five part

	Part Zero:	Include
	Part One:	Define
	Part Two:	Local data
	Part Three:	Local function

	Part Four:	Muse atomic oper

--------------------------------------------*/

/*------------------------------------------
	Part Zero: Include
--------------------------------------------*/

#include "mato.h"


/*------------------------------------------
	Part Four: Muse atomic oper

	1. mato_init
	2. mato_inc
	3. mato_add
	4. mato_dec
	5. mato_sub
	6. mato_inc_and_test 
	7. mato_add_and_test
	8. mato_dec_and_test
	9. mato_sub_and_test

--------------------------------------------*/

/*-----mato_init-----*/
void mato_init(MATOS *pMato, int nSet)
{
	pMato->mato_cnt = nSet;
	return;
}


/*-----mato_inc-----*/
inline mar_t mato_inc(MATOS *pMato)
{
	return	__sync_fetch_and_add(&pMato->mato_cnt, 1);
}


/*-----mato_add-----*/
inline mar_t mato_add(MATOS *pMato, int nInc)
{
	return	__sync_fetch_and_add(&pMato->mato_cnt, (nInc));
}


/*-----mato_dec-----*/
inline mar_t mato_dec(MATOS *pMato)
{
	return	__sync_fetch_and_sub(&pMato->mato_cnt, 1);
}


/*-----mato_sub-----*/
inline mar_t mato_sub(MATOS *pMato, int nSub)
{
	return	__sync_fetch_and_sub(&pMato->mato_cnt, (nSub));
}


/*-----mato_inc_and_test-----*/
inline mar_t mato_inc_and_test(MATOS *pMato)
{
	return	((__sync_fetch_and_add(&pMato->mato_cnt, 1) + 1) ? MATO_FALSE : MATO_TRUE);
}


/*-----mato_add_and_test-----*/
inline mar_t mato_add_and_test(MATOS *pMato, int nInc)
{
	return	((__sync_fetch_and_add(&pMato->mato_cnt, (nInc)) + nInc) ? MATO_FALSE : MATO_TRUE);
}


/*-----mato_dec_and_test-----*/
inline mar_t mato_dec_and_test(MATOS *pMato)
{
	return	((__sync_fetch_and_sub(&pMato->mato_cnt, 1) - 1) ? MATO_FALSE : MATO_TRUE);
}


/*-----mato_sub_and_test-----*/
inline mar_t mato_sub_and_test(MATOS *pMato, int nSub)
{
	return	((__sync_fetch_and_sub(&pMato->mato_cnt, (nSub)) - nSub) ? MATO_FALSE : MATO_TRUE);
}
