/*------------------------------------------
	Source file content Five part

	Part Zero:	Include
	Part One:	Local data
	Part Two:	Local function
	Part Three:	Define

	Part Four:	Signal handle

--------------------------------------------*/

/*------------------------------------------
	Part Zero: Include
--------------------------------------------*/

#include "spinc.h"
#include "spmpool.h"
#include "spnet.h"
#include "spdb.h"

#include "mpctl.h"

#include "spextb.h"
#include "speglobal.h"


/*------------------------------------------
	Part One: Local data
--------------------------------------------*/

static	pmut_t	sigIntLock = PTHREAD_MUTEX_INITIALIZER;
static	pmut_t	sigSegLock = PTHREAD_MUTEX_INITIALIZER;


/*------------------------------------------
	Part Two: Local function
--------------------------------------------*/

/* Part Four */
static	void	exbug_signal_handler(int nSignal);

/*------------------------------------------
	Part Four: Signal handle

	1. exbug_signal_init
	2. exbug_signal_handler

--------------------------------------------*/

/*-----exbug_signal_init-----*/
int exbug_signal_init(void)
{
	struct	sigaction	sigStru;
	sigset_t		sigMask;

	/* for signal int */
	sigemptyset(&sigMask);
	sigaddset(&sigMask, SIGINT);

	sigStru.sa_handler = exbug_signal_handler;
	sigStru.sa_mask = sigMask;
	sigStru.sa_flags = 0;

	if(sigaction(SIGINT, &sigStru, NULL) == FUN_RUN_FAIL) {
		perror("Extbug---> exbug_signal_init - sigaction - SIGINT");
		return	FUN_RUN_END;
	}

	/* for signal segv */
	sigdelset(&sigMask, SIGINT);
	sigStru.sa_mask = sigMask;

	if(sigaction(SIGSEGV, &sigStru, NULL) == FUN_RUN_FAIL) {
		perror("otbug_init_signal - sigaction - SIGSEGV");
		return	FUN_RUN_END;
	}

	return	FUN_RUN_OK;
}


/*-----exbug_signal_handler-----*/
static void exbug_signal_handler(int nSignal)
{
	int	nTimes = 0;

	if(nSignal == SIGINT) {
		pthread_mutex_lock(&sigIntLock);

		printf("Extbug---> inside SIGINT...\n");
		
		while(nTimes++ < nExbugPthead && !mato_sub_and_test(&pthreadCtlLock, 0))
			sleep(TAKE_A_SEC);

		exbug_paper_sync();
		exbug_data_sync();
		mgc_all_clean(exbGarCol);
		mgc_one_clean(&extResCol);

		printf("Extbug---> quitting...\n");
		exit(FUN_RUN_FAIL);
	}

	if(nSignal == SIGSEGV) {
		pthread_mutex_lock(&sigSegLock);

		printf("Extbug---> caught SIGSEGV\n");
		exbug_sig_error(PTHREAD_ERROR);
	}
}
