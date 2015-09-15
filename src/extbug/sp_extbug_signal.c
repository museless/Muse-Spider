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
