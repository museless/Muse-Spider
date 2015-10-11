/*------------------------------------------
	Source file content Six part

	Part Zero:	Include
	Part One:	Define
	Part Two:	Local data
	Part Three:	Local function

	Part Four:	Muse thpool API
	Part Five:	Muse thpool slave

--------------------------------------------*/

/*------------------------------------------
	Part Zero: Include
--------------------------------------------*/

#include "spinc.h"
#include "mpctl.h"


/*------------------------------------------
	Part One: Define
--------------------------------------------*/


/*------------------------------------------
	Part Two: Local data
--------------------------------------------*/

enum	PTHREAD_STATE {
	PTH_IS_WATCHER, PTH_IS_UNINITED, PTH_ALREADY_KILLED, PTH_WAS_KILLED, PTH_IS_READY, PTH_IS_BUSY
};


/*------------------------------------------
	Part Three: Local function
--------------------------------------------*/

/* Part Five */
static	void	*mpc_thread_entrance(void *paraStore);

static	PTHPELE	*mpc_search_empty(PTHPOOL *thPool);
static	int	mpc_thread_create(PTHPELE *pThread);
static	void	mpc_thread_cleanup(void *pPara);

static	int	mpc_thread_watcher(PTHPOOL *thPool);

static	void	mpc_thread_perror(char *errStr, int nErr);
static	void	mpc_thread_therror(PTHPELE *pThread, char *errStr, int nErr);


/*------------------------------------------
	Part Four: Muse thpool API

	1. mpc_create
	2. mpc_thread_wake
	3. mpc_thread_wait
	4. mpc_destroy

--------------------------------------------*/

/*-----mpc_create------*/
PTHPOOL *mpc_create(int nPthread)
{
	PTHPOOL	*pthPool;
	PTHPELE	*pThread;
	int	nCir;

	if(nPthread < 1 || !(pthPool = malloc(sizeof(PTHPOOL))))
		return	NULL;

	memset(pthPool, 0, sizeof(PTHPOOL));

	pthPool->pl_cnt = nPthread + 1;

	if(!(pthPool->pl_list = malloc(pthPool->pl_cnt * sizeof(PTHPELE)))) {
		mpc_destroy(pthPool);
		return	NULL;
	}

	/* create pthread: in pool */
	for(pThread = pthPool->pl_list, nCir = 0; nCir < pthPool->pl_cnt; nCir++, pThread++) {
		if(!mpc_thread_create(pThread)) {
			mpc_destroy(pthPool);
			return	NULL;
		}
	}

	/* wake and set the watcher pthread */
	if(!mpc_thread_wake(pthPool, (pthrun)mpc_thread_watcher, (void *)pthPool)) {
		mpc_destroy(pthPool);
		return	NULL;
	}

	return	pthPool;
}


/*-----mpc_thread_wake-----*/
int mpc_thread_wake(PTHPOOL *threadPool, pthrun runFun, void *pPara)
{
	PTHPELE	*pSele;

	if(!(pSele = mpc_search_empty(threadPool)))
		return	PTH_RUN_END;

	pSele->pe_run = runFun;
	pSele->pe_data = pPara;
	pSele->pe_flags = PTH_IS_BUSY;

	pthread_cond_signal(&pSele->pe_cond);
	pthread_mutex_unlock(&pSele->pe_mutex);

	return	PTH_RUN_OK;
}


/*-----mpc_thread_wait-----*/
void mpc_thread_wait(PTHPOOL *thPool)
{
	PTHPELE	*thList;

	if(thPool && thPool->pl_list) {
		/* first pass the watcher thread */
		for(thList = thPool->pl_list + 1; thList < thPool->pl_list + thPool->pl_cnt; thList++) {
			pthread_mutex_lock(&thList->pe_mutex);

			while(thList->pe_flags == PTH_IS_BUSY)
				pthread_cond_wait(&thList->pe_cond, &thList->pe_mutex);

			pthread_mutex_unlock(&thList->pe_mutex);
		}
	}
}


/*-----mpc_destroy-----*/
void mpc_destroy(PTHPOOL *thPool)
{
	PTHPELE	*thMov;
	TIMVAL	thClock;
	int	tStatus;

	if(thPool) {
		if(thPool->pl_list) {
			for(thMov = thPool->pl_list; thMov < thPool->pl_list + thPool->pl_cnt; thMov++) {
				if(thMov->pe_flags != PTH_WAS_KILLED) {
					/* just try the pthread was fucked or not */
					if(!pthread_tryjoin_np(thMov->pe_tid, NULL))
						continue;

					if(thMov->pe_flags == PTH_IS_BUSY) {
						thClock.tv_sec = 0;
						thClock.tv_usec = MILLSEC_500_MICROSEC;
						select(0, NULL, NULL, NULL, &thClock);

						tStatus = pthread_mutex_trylock(&thMov->pe_mutex);

						if(tStatus == EBUSY && thMov->pe_flags == PTH_IS_BUSY) {
							pthread_cancel(thMov->pe_tid);

							pthread_mutex_unlock(&thMov->pe_mutex);
							pthread_cond_destroy(&thMov->pe_cond);

							pthread_join(thMov->pe_tid, NULL);
							continue;
						}
					}

					thMov->pe_flags = PTH_ALREADY_KILLED;
					pthread_cond_signal(&thMov->pe_cond);
				}
			}

			free(thPool->pl_list);
		}

		free(thPool);
	}
}


/*------------------------------------------
	Part Five: Muse thpool slave

	1. mpc_thread_entrance
	2. mpc_search_empty
	3. mpc_thread_create
	4. mpc_thread_cleanup

	5. mpc_thread_watcher
	6. mpc_thread_perror
	7. mpc_thread_therror

--------------------------------------------*/

/*-----mpc_thread_entrance-----*/
static void *mpc_thread_entrance(void *paraStore)
{
	PTHPELE	*thPara = (PTHPELE *)paraStore;
	int	thStatus;

	/* some prepare job */
	if((thStatus = pthread_mutex_lock(&thPara->pe_mutex)))
		mpc_thread_therror(thPara, "mpc_thread_entrance - pthread_mutex_lock", thStatus);

	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

	if((thStatus = pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL)))
		mpc_thread_therror(thPara, "mpc_thread_entrance - pthread_setcanceltype", thStatus);

	thPara->pe_flags = PTH_IS_READY;

	if(pthread_cond_signal(&thPara->pe_cond))
		mpc_thread_therror(thPara, "mpc_thread_entrance - pthread_cond_signal", thStatus);

	if(pthread_mutex_unlock(&thPara->pe_mutex))
		mpc_thread_therror(thPara, "mpc_thread_entrance - pthread_mutex_unlock", thStatus);

	while(PTH_RUN_PERMANENT) {
		pthread_mutex_lock(&thPara->pe_mutex);

		while(thPara->pe_flags != PTH_IS_BUSY) {
			if(thPara->pe_flags == PTH_ALREADY_KILLED) {
				pthread_mutex_unlock(&thPara->pe_mutex);

				pthread_cond_destroy(&thPara->pe_cond);
				pthread_mutex_destroy(&thPara->pe_mutex);

				pthread_detach(pthread_self());
				pthread_exit(NULL);
			}

			pthread_cond_wait(&thPara->pe_cond, &thPara->pe_mutex);
		}

		/* routine running */
		pthread_cleanup_push(mpc_thread_cleanup, thPara);
		thPara->pe_run(thPara->pe_data);
		pthread_cleanup_pop(0);

		thPara->pe_flags = PTH_IS_READY;

		pthread_cond_signal(&thPara->pe_cond);
		pthread_mutex_unlock(&thPara->pe_mutex);
	}
}


/*-----mpc_search_empty-----*/
static PTHPELE *mpc_search_empty(PTHPOOL *thPool)
{
	PTHPELE	*eleMov;
	int	tStatus;

	for(eleMov = thPool->pl_list; eleMov < thPool->pl_list + thPool->pl_cnt; eleMov++) {
		if(eleMov->pe_flags == PTH_IS_READY) {
			if((tStatus = pthread_mutex_trylock(&eleMov->pe_mutex))) {
				if(tStatus == EBUSY)
					continue;

				return	NULL;
			}

			return	eleMov;
		}
	}

	return	NULL;
}


/*-----mpc_thread_create-----*/
static int mpc_thread_create(PTHPELE *pThread)
{
	int	thStatus;

	pThread->pe_data = NULL;
	pThread->pe_run = NULL;
	pThread->pe_flags = PTH_IS_UNINITED;

	if((thStatus = pthread_mutex_init(&pThread->pe_mutex, NULL))) {
		mpc_thread_perror("mpc_thread_create - pthread_mutex_init", thStatus);
		return	PTH_RUN_END;
	}

	if((thStatus = pthread_cond_init(&pThread->pe_cond, NULL))) {
		mpc_thread_perror("mpc_thread_create - pthread_cond_init", thStatus);
		return	PTH_RUN_END;
	}

	if((thStatus = pthread_create(&pThread->pe_tid, NULL, mpc_thread_entrance, (void *)pThread))) {
		mpc_thread_perror("mpc_thread_create - pthread_create", thStatus);
		return	PTH_RUN_END;
	}

	if((thStatus = pthread_mutex_lock(&pThread->pe_mutex))) {
		mpc_thread_perror("mpc_thread_create - pthread_mutex_lock", thStatus);
		return	PTH_RUN_END;
	}

	while(pThread->pe_flags != PTH_IS_READY) {
		if((thStatus = pthread_cond_wait(&pThread->pe_cond, &pThread->pe_mutex))) {
			mpc_thread_perror("mpc_thread_create - pthread_cond_wait", thStatus);
			return	PTH_RUN_END;
		}
	}

	if((thStatus = pthread_mutex_unlock(&pThread->pe_mutex))) {
		mpc_thread_perror("mpc_thread_create - pthread_mutex_unlock", thStatus);
		return	PTH_RUN_END;
	}

	return	PTH_RUN_OK;
}


/*-----mpc_thread_cleanup-----*/
static void mpc_thread_cleanup(void *pPara)
{
	PTHPELE	*elePoint = (PTHPELE *)pPara;

	pthread_mutex_unlock(&elePoint->pe_mutex);
	elePoint->pe_flags = PTH_WAS_KILLED;
}


/*-----mpc_thread_watcher-----*/
static int mpc_thread_watcher(PTHPOOL *thPool)
{
	while(PTH_RUN_PERMANENT)
		sleep(TAKE_A_LLSLP);        /* Just go to sleep, do not waste cpu */

	return	FUN_RUN_OK;
}


/*-----mpc_thread_perror-----*/
static void mpc_thread_perror(char *errStr, int nErr)
{
	printf("ThreadPool---> %s - %s\n", errStr, strerror(nErr));
}


/*-----mpc_thread_therror-----*/
static void mpc_thread_therror(PTHPELE *pThread, char *errStr, int nErr)
{
	mpc_thread_perror(errStr, nErr);

	pThread->pe_flags = PTH_WAS_KILLED;
	
	pthread_detach(pThread->pe_tid);
	pthread_cond_signal(&pThread->pe_cond);
	pthread_mutex_unlock(&pThread->pe_mutex);

	pthread_exit(NULL);
}
