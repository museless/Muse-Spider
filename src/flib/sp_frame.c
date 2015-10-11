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
int sp_normal_init(char *pOwn, MGCH **garCol, MSGSET **msgSet, msginit minitFun, char *errLoc, int msgFd)
{
	snprintf(ownNameSave, OWN_NAME_LEN - 1, "%s", pOwn);

	if((*garCol = mgc_init()) == NULL) {
		printf("%s---> sp_normal_init - mgc_init: %s", pOwn, strerror(errno));
		exit(FUN_RUN_END);
	}

	if(mgc_add(*garCol, NULL_POINT, (gcfun)mc_conf_unload) == MGC_FAILED)
		printf("%s---> sp_normal_init - mgc_add - mc_conf: %s", pOwn, strerror(errno));

	if(msgFd) {
		if(!(*msgSet = minitFun(msgFd))) {
			printf("%s---> sp_normal_init - msg_init: %s", pOwn, strerror(errno));
			return	FUN_RUN_END;
		}
	
		if(mgc_add(*garCol, *msgSet, (gcfun)sp_msg_frame_destroy) == MGC_FAILED)
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
