/* Copyright (c) 2015, William Muse
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */


/*------------------------------------------
	Source file content Five part

	Part Zero:	Include
	Part One:	Define
	Part Two:	Local data
	Part Three:	Local function

	Part Four:	Mempool API

--------------------------------------------*/

/*------------------------------------------
	Part Zero: Include
--------------------------------------------*/

#include "spinc.h"
#include "spmpool.h"


/*------------------------------------------
	Part Four: Mempool API

	1. wmpool_create
	2. wmpool_malloc
	3. wmpool_free
	4. wmpool_destroy

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

