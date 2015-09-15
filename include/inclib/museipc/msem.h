#ifndef	_MSEM_H
#define	_MSEM_H


/*---------------------
	include
-----------------------*/

#include <sys/sem.h>


/*---------------------
	typedef
-----------------------*/

typedef	struct sembuf	SEMBUF;
typedef struct semsave	MSEM;
typedef	union semun	SEMUN;


/*---------------------
	struct
-----------------------*/

union	semun {
	struct	semid_ds	*sem_buf;
	struct	seminfo		*__buf;
	unsigned short		*sem_array;
	int			sem_val;
};

struct semsave {
	SEMBUF	sem_wait, sem_wake;
	int	sem_id;
};


/*---------------------
	function
-----------------------*/

MSEM	*msem_create(char *semFile, int nShares, int nProj);
MSEM	*msem_link(char *semFile, int nProj);
int	msem_wait_empty(MSEM *smHand, int nOperator);
int	msem_wait(MSEM *waitSem);
int	msem_wake(MSEM *wakeSem);
void	msem_destroy(MSEM *desSem);


#endif
