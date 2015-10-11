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
