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
	Part One:	Local data
	Part Two:	Local function
	Part Three:	Define

	Part Four:	Share memory main

--------------------------------------------*/

/*------------------------------------------
	Part Zero: Include
--------------------------------------------*/

#include "spinc.h"
#include "mipc.h"


/*------------------------------------------
	Part Four: Share memory main

	1. mshm_create
	2. mshm_link
	3. mshm_malloc
	4. mshm_unlink
	5. mshm_remove_id
	6. mshm_destroy

--------------------------------------------*/

/*-----mshm_create-----*/
MSHM *mshm_create(char *shmFile, int nSize, int nProj)
{
	MSHM	*pShare;

	if(!shmFile || !(pShare = malloc(sizeof(MSHM))))
		return	NULL;

	if(!(pShare->shm_sembuf = msem_create(shmFile, 1, nProj)))
		return	NULL;

	nSize = ((nSize >> PG_SIZE_SHF) + 1) << PG_SIZE_SHF;

	if((pShare->shm_id = shmget(ftok(shmFile, nProj), nSize, IPC_CREAT | USR_RDWR)) == MIPC_FAIL)
		return	NULL;

	if(shmctl(pShare->shm_id, IPC_STAT, &pShare->shm_ds) == MIPC_FAIL)
		return	NULL;

	if(!(pShare->shm_mov = pShare->shm_start = shmat(pShare->shm_id, NULL, 0)))
		return	NULL;

	return	pShare;
}


/*-----mshm_link-----*/
MSHM *mshm_link(char *shmFile, int nProj)
{
	MSHM	*pShared;

	if(!shmFile || !(pShared = malloc(sizeof(MSHM))))
		return	NULL;

	if(!(pShared->shm_sembuf = msem_link(shmFile, nProj)))
		return	NULL;

	if((pShared->shm_id = shmget(ftok(shmFile, nProj), 0, USR_READ)) == MIPC_FAIL)
		return	NULL;

	if(shmctl(pShared->shm_id, IPC_STAT, &pShared->shm_ds) == MIPC_FAIL)
		return	NULL;

	if(!(pShared->shm_mov = pShared->shm_start = shmat(pShared->shm_id, NULL, 0)))
		return	NULL;

	return	pShared;
}


/*-----mshm_malloc-----*/
void *mshm_malloc(MSHM *smStru, int nMalloc)
{
	char	*pMem;

	if(!(smStru->shm_ds.shm_perm.mode & ALL_WRITE))
		return	NULL;

	msem_wait(smStru->shm_sembuf);

	pMem = (char *)smStru->shm_mov;

	if(pMem + nMalloc > (char *)smStru->shm_start + smStru->shm_ds.shm_segsz) {
		msem_wake(smStru->shm_sembuf);
		return	NULL;
	}

	smStru->shm_mov = (char *)smStru->shm_mov + nMalloc;

	msem_wake(smStru->shm_sembuf);

	return	pMem;
}


/*-----mshm_unlink-----*/
int mshm_unlink(MSHM *deStru)
{
	return	shmdt(deStru->shm_start);
}


/*-----mshm_remove_id-----*/
int mshm_remove_id(MSHM *rmStru)
{
	return	shmctl(rmStru->shm_id, IPC_RMID, &rmStru->shm_ds);
}


/*-----mshm_destroy-----*/
void mshm_destroy(MSHM *desStru)
{
	if(desStru) {
		msem_destroy(desStru->shm_sembuf);

		mshm_unlink(desStru);
		mshm_remove_id(desStru);

		free(desStru);
	}
}
