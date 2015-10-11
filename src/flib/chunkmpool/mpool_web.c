#include "spinc.h"
#include "spmpool.h"


/*------------------------------------------
	Part Zero: Mempool API

	1. wmpool_create
	2. wmpool_malloc
	3. wmpool_free
	4. wmpool_destroy

	macro function (spmpool.h)
	5. wmpool_bit_is_block

--------------------------------------------*/

/*-----wmpool_create-----*/
WPOOL *wmpool_create(int nClip, int cSize)
{
	WPOOL	*pStru;
	int	nMap;

	if(nClip < 1 || cSize <= 0) {
		printf("wmpool_create nClip: %d - cSize: %d - failed\n", nClip, cSize);
		return	NULL;
	}

	if(!(pStru = malloc(sizeof(WPOOL))))
		return	NULL;

	pStru->wmp_psize = cSize;
	pStru->wmp_clip = nClip;

	if(!(pStru->wmp_point = malloc(pStru->wmp_psize * pStru->wmp_clip)))
		return	NULL;

	nMap = nClip / WMP_MAPBIT;
	nMap += ((nClip % WMP_MAPBIT) ? 1 : 0);

	if(!(pStru->wmp_map = calloc(nMap, sizeof(int)))) {
		free(pStru->wmp_point);
		return	NULL;
	}

	return	pStru;
}


/*-----wmpool_malloc-----*/
void *wmpool_malloc(WPOOL *pHandler)
{
	int	*pMap = pHandler->wmp_map;
	int	nCir, bNum;

	for(bNum = nCir = 0; bNum < pHandler->wmp_clip; nCir++, bNum++) {
		if(nCir == WMP_MAPBIT) {
			nCir = 0;
			pMap++;
		}

		if(!wmpool_bit_is_block(*pMap, nCir)) {
			*pMap |= (WMP_BIT_BLOCK << nCir);
			break;
		}
	}

	return	(bNum != pHandler->wmp_clip) ? (bNum * pHandler->wmp_psize + pHandler->wmp_point) : NULL;
}


/*-----wmpool_calloc-----*/
void *wmpool_calloc(WPOOL *pHandler)
{
	void	*pRet;

	pRet = wmpool_malloc(pHandler);
	memset(pRet, 0, pHandler->wmp_psize);

	return	pRet;
}


/*-----wmpool_free-----*/
void wmpool_free(WPOOL *pHandler, void *aPoint)
{
	int	nBit, *nMap;

	if((char *)aPoint > (pHandler->wmp_point + (pHandler->wmp_psize * (pHandler->wmp_clip - 1))))
		return;

	nBit = (unsigned int)(((char *)aPoint - pHandler->wmp_point) / pHandler->wmp_psize);
	nMap = pHandler->wmp_map + (nBit >> WMP_MAPBIT_SHI);
	*nMap &= (~(WMP_BIT_BLOCK << (nBit % WMP_MAPBIT)));
}


/*-----wmpool_destroy-----*/
void wmpool_destroy(void *pHandler)
{
	WPOOL	*pFree = (WPOOL *)pHandler;

	if(pFree) {
		if(pFree->wmp_point)
			free(pFree->wmp_point);

		if(pFree->wmp_map)
			free(pFree->wmp_map);

		free(pFree);
	}
}

