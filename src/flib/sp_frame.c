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
	Part One:	Define
	Part Two:	Local data
	Part Three:	Local function

	Part Four:	Sp init frame

--------------------------------------------*/

/*------------------------------------------
	Part Zero: Include
--------------------------------------------*/

#include "spinc.h"
#include "spmsg.h"
#include "spframe.h"


/*------------------------------------------
	Part Two: Local data
--------------------------------------------*/

static	char	ownNameSave[OWN_NAME_LEN];


/*------------------------------------------
	Part Four: Sp init frame

	1. sp_normal_init

--------------------------------------------*/

/*-----sp_normal_init-----*/
int sp_normal_init(char *pOwn, MGCH **garCol, SOSET **msgSet, msginit minitFun, char *errLoc, int msgFlags)
{
	snprintf(ownNameSave, OWN_NAME_LEN - 1, "%s", pOwn);

	if((*garCol = mgc_init()) == NULL) {
		printf("%s---> sp_normal_init - mgc_init: %s", pOwn, strerror(errno));
		exit(FUN_RUN_END);
	}

	if(mgc_add(*garCol, NULL_POINT, (gcfun)mc_conf_unload) == MGC_FAILED)
		printf("%s---> sp_normal_init - mgc_add - mc_conf: %s", pOwn, strerror(errno));

	if(msgFlags) {
		if(!(*msgSet = minitFun())) {
			printf("%s---> sp_normal_init - msg_init: %s", pOwn, strerror(errno));
			return	FUN_RUN_END;
		}
	
		if(mgc_add(*garCol, *msgSet, (gcfun)sp_msgs_frame_destroy) == MGC_FAILED)
			printf("%s---> sp_normal_init - mgc_add - msg_init: %s", pOwn, strerror(errno));
	}

	/* elog init */
	if(elog_init(errLoc) == FUN_RUN_FAIL) {
		printf("%s---> sp_normal_init - elog_init - %s: %s", pOwn, errLoc, strerror(errno));
		return	FUN_RUN_END;
	}

	if(mgc_add(*garCol, NULL_POINT, (gcfun)elog_destroy) == MGC_FAILED)
		printf("%s---> sp_normal_init - mgc_add - elog: %s", pOwn, strerror(errno));

	return	FUN_RUN_OK;
}
