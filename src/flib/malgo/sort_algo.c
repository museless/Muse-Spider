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

	Part Four:	Shell sort

--------------------------------------------*/


/*------------------------------------------
	Part Zero: Include
--------------------------------------------*/

#include "spinc.h"
#include "malgo.h"


/*------------------------------------------
	Part One: Define
--------------------------------------------*/


/*------------------------------------------
	Part Two: Local data
--------------------------------------------*/


/*------------------------------------------
	Part Three: Local function
--------------------------------------------*/


/*------------------------------------------
	Part Four: Shell sort

	1. sp_shell_sort_init
	2. sp_shell_sort
	3. sp_shell_sort_destroy
	4. sp_shell_sort_seleinc

--------------------------------------------*/

/*-----sp_shell_sort_init-----*/
SSORTS	*sp_shell_sort_init(ssmov smFun, ssass smAssign, ssprog smProgress, sscmre smCmprep,
			mafun fMalloc, mafree freeFun, void *mHandler)
{
	SSORTS	*pStru;

	if(!(pStru = fMalloc(mHandler, sizeof(SSORTS))))
		return	NULL;

	pStru->sst_mv = smFun;
	pStru->sst_assign = smAssign;
	pStru->sst_progress = smProgress;
	pStru->sst_cmprep = smCmprep;
	pStru->sst_free = freeFun;

	return	pStru;
}


/*-----sp_shell_sort-----*/
void sp_shell_sort(SSORTS *ssStru, void *sortBuf, int nLimit)
{
	void	*pCmp, *pSort, *pHandler;
	int	nBeg, nCir;

	if(nLimit < SS_MIN_INPUT)
		return;

	for(nCir = nLimit >> 1; nCir > 0; nCir--) {
		pSort = ssStru->sst_assign((pHandler = sortBuf));

		for(nBeg = 0; nBeg + nCir < nLimit && pSort; nBeg++, pSort = ssStru->sst_progress(&pHandler, pSort)) {
			if(!(pCmp = ssStru->sst_mv(pHandler, pSort, nCir)))
				continue;

			ssStru->sst_cmprep(pSort, pCmp);
		}
	}
}


/*-----sp_shell_sort_destroy-----*/
void sp_shell_sort_destroy(void *ssStru)
{
	mafree	freeFun;

	if(ssStru) {
		freeFun = ((SSORTS *)ssStru)->sst_free;

		if(freeFun)
			freeFun(ssStru);
	}
}

