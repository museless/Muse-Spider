#ifndef	_MSHM_H
#define	_MSHM_H


/*---------------------
	include
-----------------------*/

#include "msem.h"
#include <sys/shm.h>


/*---------------------
	define
-----------------------*/

#define	PG_SIZE_SHF	0xC


/*---------------------
	typedef
-----------------------*/

typedef	struct shmid_ds	SDDS;
typedef	struct mshm	MSHM;


/*---------------------
	struct
-----------------------*/

struct	mshm {
	void	*shm_start;
	void	*shm_mov;

	MSEM	*shm_sembuf;
	SDDS	shm_ds;

	int	shm_id;
};


/*---------------------
	function
-----------------------*/

MSHM	*mshm_create(char *shmFile, int nSize, int nProj);
MSHM	*mshm_link(char *shmFile, int nProj);
void	*mshm_malloc(MSHM *smStru, int nMalloc);
int	mshm_unlink(MSHM *deStru);
int	mshm_remove_id(MSHM *rmStru);
void	mshm_destroy(MSHM *desStru);


#endif
