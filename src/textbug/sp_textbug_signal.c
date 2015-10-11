/*------------------------------------------
	Source file content Five part

	Part Zero:	Include
	Part One:	Define
	Part Two:	Local data
	Part Three:	Local function

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

#include "sptglobal.h"
#include "sptextb.h"


/*------------------------------------------
	Part Two: Local data
--------------------------------------------*/

static	pmut_t	sigIntLock = PTHREAD_MUTEX_INITIALIZER;
static	pmut_t	sigSegLock = PTHREAD_MUTEX_INITIALIZER;


/*------------------------------------------
	Part Four: Signal handle

	1. txbug_signal_init
	2. txbug_signal_handler

--------------------------------------------*/

/*-----txbug_signal_init-----*/
void txbug_signal_init(void)
{
	struct	sigaction	sigStru;
	sigset_t		sigMask;

	/* init SIGINT */
	sigemptyset(&sigMask);
	sigaddset(&sigMask, SIGINT);

	sigStru.sa_handler = txbug_signal_handler;
	sigStru.sa_mask = sigMask;
	sigStru.sa_flags = 0;

	if(sigaction(SIGINT, &sigStru, NULL) == FUN_RUN_FAIL) {
		txbug_perror("txbug_signal_init - sigaction - SIGINT", errno);
		exit(FUN_RUN_FAIL);
	}

	/* init SIGSEGV */
	sigemptyset(&sigMask);
	sigaddset(&sigMask, SIGSEGV);

	sigStru.sa_handler = txbug_signal_handler;
	sigStru.sa_mask = sigMask;
	sigStru.sa_flags = 0;

	if(sigaction(SIGSEGV, &sigStru, NULL) == FUN_RUN_FAIL) {
		txbug_perror("txbug_signal_init - sigaction - SIGINT", errno);
		exit(FUN_RUN_FAIL);
	}
}


/*-----txbug_signal_handler-----*/
void txbug_signal_handler(int nSignal)
{
	if(nSignal == SIGINT) {
		pthread_mutex_lock(&sigIntLock);

		printf("Textbug---> inside SIGINT\n");

		mpc_destroy(tbThreadPool);

		textbugRunSet.ts_wtr(&txTranHand.tx_sql, contStoreBuf);
		mgc_all_clean(textGarCol);
		mgc_one_clean(&txbugResCol);
		
		printf("TextBug---> quitting\n");
		exit(FUN_RUN_FAIL);
	}

	if(nSignal == SIGSEGV) {
		pthread_mutex_lock(&sigSegLock);

		printf("Textbug---> inside SIGSEGV\n");

		txbug_sig_error();
	}
}
