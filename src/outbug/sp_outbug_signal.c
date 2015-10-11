/*------------------------------------------
	Source file content Five part

	Part Zero:	Include
	Part One:	Local data
	Part Two:	Local function
	Part Three:	Define

	Part Four:	Signal operate

--------------------------------------------*/

/*------------------------------------------
	Part Zero: Include
--------------------------------------------*/

#include "spinc.h"
#include "spdb.h"

#include "mpctl.h"

#include "spoglobal.h"
#include "spoutb.h"


/*------------------------------------------
	Part One: Local data
--------------------------------------------*/

static	pmut_t	sigIntLock = PTHREAD_MUTEX_INITIALIZER;
static	pmut_t	sigSegLock = PTHREAD_MUTEX_INITIALIZER;


/*------------------------------------------
	Part Two: Local function
--------------------------------------------*/

/* Part two */
static void otbug_signal_handler(int nSign);


/*------------------------------------------
	Part Four: Signal operate

	1. otbug_init_signal
	2. otbug_signal_handler

--------------------------------------------*/

/*-----otbug_init_signal-----*/
int otbug_init_signal(void)
{
	struct	sigaction	sigStru;
	sigset_t		sigMask;

	/* for signal sigint */
	sigemptyset(&sigMask);
	sigaddset(&sigMask, SIGINT);
	sigaddset(&sigMask, SIGSEGV);

	sigStru.sa_mask = sigMask;
	sigStru.sa_handler = otbug_signal_handler;
	sigStru.sa_flags = 0;

	if(sigaction(SIGINT, &sigStru, NULL) == FUN_RUN_FAIL) {
		perror("otbug_init_signal - sigaction - SIGINT");
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


/*-----otbug_signal_handler-----*/
static void otbug_signal_handler(int nSign)
{
	if(nSign == SIGINT) {
		pthread_mutex_lock(&sigIntLock);

		printf("Outbug---> caught SIGINT...\n");

		mpc_destroy(obThreadPool);
		otbug_filectl_thread_destroy();

		mgc_all_clean(obGarCol);
		mgc_one_clean(&otResCol);

		pthread_mutex_unlock(&sigIntLock);
		pthread_mutex_unlock(&sigSegLock);

		printf("Outbug---> quitting...\n");
		exit(FUN_RUN_FAIL);
	}

	if(nSign == SIGSEGV) {
		pthread_mutex_lock(&sigSegLock);

		printf("Outbug---> caught SIGSEGV\n");
		otbug_sig_error();
	}
}
