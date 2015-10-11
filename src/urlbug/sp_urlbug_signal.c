#include "spinc.h"
#include "spdb.h"
#include "spnet.h"
#include "spmsg.h"
#include "spmpool.h"

#include "mpctl.h"

#include "spuglobal.h"
#include "spurlb.h"


/* local data */
static	pmut_t	sigIntLock = PTHREAD_MUTEX_INITIALIZER;
static	pmut_t	sigSegLock = PTHREAD_MUTEX_INITIALIZER;


/*------------------------------------------
	Part Zero: Signal operate

	1. ubug_init_signal
	2. ubug_signal_handler

--------------------------------------------*/

/*-----ubug_init_signal-----*/
void ubug_init_signal(void)
{
	struct	sigaction	sigStru;
	sigset_t		sigMask;

	/* for signal sigint */
	sigemptyset(&sigMask);
	sigaddset(&sigMask, SIGINT);
	sigaddset(&sigMask, SIGSEGV);

	sigStru.sa_mask = sigMask;
	sigStru.sa_handler = ubug_signal_handler;
	sigStru.sa_flags = 0;

	if(sigaction(SIGINT, &sigStru, NULL) == FUN_RUN_FAIL) {
		ubug_perror("ubug_init_signal - sigaction - SIGINT", errno);
		exit(FUN_RUN_FAIL);
	}

	/* for signal segv */
	sigdelset(&sigMask, SIGINT);
	sigaddset(&sigMask, SIGSEGV);
	sigStru.sa_mask = sigMask;

	if(sigaction(SIGSEGV, &sigStru, NULL) == FUN_RUN_FAIL) {
		ubug_perror("ubug_init_signal - sigaction - SIGSEGV", errno);
		exit(FUN_RUN_FAIL);
	}
}


/*-----ubug_signal_handler-----*/
void ubug_signal_handler(int nSign)
{
	if (nSign == SIGINT) {
		pthread_mutex_lock(&sigIntLock);

		printf("UrlBug---> caught SIGINT...\n");

		mpc_destroy(ubugThreadPool);

                if (urlRunSet.ubs_fstf)
		        urlRunSet.ubs_fstf();

		mgc_all_clean(urlGarCol);
		
		printf("UrlBug---> quitting...\n");
		exit(FUN_RUN_FAIL);
	}

	if (nSign == SIGSEGV) {
		pthread_mutex_lock(&sigSegLock);

		printf("Urlbug---> caught SIGSEGV\n");
		ubug_sig_error();
	}
}
