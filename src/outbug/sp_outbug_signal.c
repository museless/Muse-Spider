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
