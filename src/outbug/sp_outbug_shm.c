/*------------------------------------------
	Source file content Seven part

	Part Zero:	Include
	Part One:	Define
	Part Two:	Local data
	Part Three:	Local function

	Part Four:	Outbug shm
	Part Five:	Shm methods
	Part Six:	Tmpfile shm

--------------------------------------------*/

/*------------------------------------------
	Part Zero: Include
--------------------------------------------*/

#include "spinc.h"
#include "spdb.h"
#include "mipc.h"
#include "spoglobal.h"
#include "spoutb.h"


/*------------------------------------------
	Part One: Define
--------------------------------------------*/

#define	otbug_shm_datlen(tLen)	(tLen - TC_FIG_LEN - TC_NULL_LEN)


/*------------------------------------------
	Part Three: Local function
--------------------------------------------*/

/* Part Four */
static	void	otbug_shm_counting(void);


/*------------------------------------------
	Part Four: Outbug shm

	1. otbug_shm_alloc
	2. otbug_shm_cmp
	3. otbug_shm_hash
	4. otbug_shm_counting
	5. otbug_shm_mov_byte

--------------------------------------------*/

/*-----otbug_shm_alloc-----*/
void *otbug_shm_alloc(void *pTerms, int datLen, int *numSave)
{
	char	*pMemory;
	int	tSize = datLen + TC_FIG_LEN + TC_NULL_LEN;

	if(!(pMemory = mshm_malloc(obShmContainer, tSize)))
		return	NULL;

	/* the length */
	*((int *)pMemory) = tSize;
	/* the index - numSave[0] was current times */
	*((int *)(pMemory + TC_LENGTH_LEN)) = numSave[0];
	/* the cnt */
	*((int *)(pMemory + TC_LENGTH_LEN + TC_INDEX_LEN)) = 1;

	sprintf(pMemory + TC_FIG_LEN, "%.*s", datLen, (char *)pTerms);

	otbug_shm_counting();

	return	pMemory;
}


/*-----otbug_shm_cmp-----*/
int otbug_shm_cmp(void *beCmped, void *cmpEr, int cmpLen)
{
	char	*tmContain = (char *)cmpEr;
	int	cmperLen = *((int *)tmContain) - TC_FIG_LEN - TC_NULL_LEN;

	if(cmpLen != cmperLen)
		return	cmpLen - cmperLen;

	return	(strncmp(beCmped, tmContain + TC_FIG_LEN, cmpLen));
}


/*-----otbug_shm_hash-----*/
uLong otbug_shm_hash(void *pData, int datLen)
{
	uChar	*hashStr = (uChar *)pData;
	uLong	nCir, nTotal;

	for(nTotal = nCir = 0; nCir < datLen; nCir++)
		nTotal += hashStr[nCir];

	return	nTotal;
}


/*-----otbug_shm_counting-----*/
static void otbug_shm_counting(void)
{
	((TLCTL *)obShmCtler)->tcl_cnter++;
}


/*-----otbug_shm_mov_next-----*/
TMCON *otbug_shm_mov_next(TMCON *movTm)
{
	return	(TMCON *)((uChar *)movTm + movTm->tc_length);
}


/*------------------------------------------
	Part Five: Shm methods

	1. otbug_tcler_init
	2. otbug_shm_tcler_get
	3. otbug_tcler_lock
	4. otbug_tcler_unlock
	5. otbug_tcler_dec
	6. otbug_tcler_inc_cnt
	7. otbug_tcler_cnter
	8. otbug_shm_data_begin
	9. otbug_shm_data_fetch
	10. otbug_tcler_date

--------------------------------------------*/

/*-----otbug_tcler_init-----*/
TLCTL *otbug_tcler_init(mafun maFun, void *pContainer)
{
	TLCTL	*pCtler;

	if(!(pCtler = maFun(pContainer, sizeof(TLCTL))))
		return	NULL;

	mato_init(&pCtler->tcl_lock, 1);
	mato_init(&pCtler->tcl_flck, 1);
	pCtler->tcl_cnter = 0;

	sprintf(pCtler->tcl_date, "%s", &keysTblName[1]);

	return	pCtler;
}


/*-----otbug_shm_tcler_get-----*/
inline TLCTL *otbug_shm_tcler_get(void *pShm)
{
	return	((MSHM *)pShm)->shm_start;
}


/*-----otbug_tcler_lock-----*/
void otbug_tcler_lock(TLCTL *pCtl)
{
	while(!mato_dec_and_test(&pCtl->tcl_lock))
		mato_inc(&pCtl->tcl_lock);
}


/*-----otbug_tcler_unlock-----*/
void otbug_tcler_unlock(TLCTL *pCtl)
{
	mato_inc(&pCtl->tcl_lock);
}


/*-----otbug_tcler_dec-----*/
void otbug_tcler_dec(TLCTL *pCtl)
{
	mato_dec(&pCtl->tcl_lock);
}


/*-----otbug_tcler_inc_cnt-----*/
void otbug_tcler_inc_cnt(TLCTL *pCtl)
{
	otbug_tcler_lock(pCtl);
	pCtl->tcl_cnter++;
	otbug_tcler_unlock(pCtl);
}


/*-----otbug_tcler_cnter-----*/
inline int otbug_tcler_cnter(TLCTL *pCtl)
{
	return	pCtl->tcl_cnter;
}


/*-----otbug_shm_data_begin-----*/
inline shmd *otbug_shm_data_begin(void *shmStru)
{
	return	(((MSHM *)shmStru)->shm_start + sizeof(TLCTL));
}


/*-----otbug_shm_data_fetch-----*/
SDT *otbug_shm_data_fetch(SDT *pSdt)
{
	return	(pSdt = (SDT *)((char *)pSdt + pSdt->sd_tmcon.tc_length));
}


/*-----otbug_tcler_date-----*/
char *otbug_tcler_date(TLCTL *pCtl)
{
	return	pCtl->tcl_date;
}


/*------------------------------------------
	Part Six: Tmpfile shm

	1. otbug_tmpfile_lock
	2. otbug_tmpfile_unlock
	3. otbug_tmpfile_dec

--------------------------------------------*/

/*-----otbug_tmpfile_lock-----*/
void otbug_tmpfile_lock(TLCTL *pCtl)
{
	while(!mato_dec_and_test(&pCtl->tcl_flck))
		mato_inc(&pCtl->tcl_flck);
}


/*-----otbug_tmpfile_unlock-----*/
void otbug_tmpfile_unlock(TLCTL *pCtl)
{
	mato_inc(&pCtl->tcl_flck);
}


/*-----otbug_tmpfile_dec-----*/
void otbug_tmpfile_dec(TLCTL *pCtl)
{
	mato_dec(&pCtl->tcl_flck);
}
