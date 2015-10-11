/*------------------------------------------
	Source file content Six part

	Part Zero:	Include
	Part One:	Define
	Part Two:	Local data
	Part Three:	Local function

	Part Four:	Tools running

--------------------------------------------*/

/*------------------------------------------
	Part Zero: Include
--------------------------------------------*/

#include "spinc.h"
#include "spdb.h"

#include <sys/wait.h>

#include "spoglobal.h"
#include "spoutb.h"


/*------------------------------------------
	Part Four: Tools running

	1. otbug_tools_start

--------------------------------------------*/

/*-----otbug_tools_start-----*/
int otbug_tools_start(void)
{
	char	scriPath[PATH_LEN];
	pid_t	chPid;

	if(mc_conf_read("outbug_tool_script_loca", CONF_STR, scriPath, PATH_LEN)) {
		mc_conf_print_err("outbug_pthread_num");
		return	EINVAL;
	}

	if((chPid = fork()) < FUN_RET_NEG) {
		return	errno;

	} else if(chPid == 0) {
		if(execl(scriPath, scriPath, NULL) == FUN_RET_NEG)
			otbug_perror("otbug_tools_start - execl", errno);

		exit(FUN_RET_NEG);
	}

	if(waitpid(chPid, NULL, 0) == FUN_RET_NEG)
		return	errno;

	return	FUN_RUN_END;
}
