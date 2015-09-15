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

/*-----otbug_script_start-----*/
int otbug_script_start(char *pSearch)
{
	char	scriPath[PATH_LEN];
	pid_t	chPid;

	if(mc_conf_read(pSearch, CONF_STR, scriPath, PATH_LEN) == FUN_RUN_FAIL) {
		mc_conf_print_err(pSearch);
		return	EINVAL;
	}

	if((chPid = vfork()) == FUN_RET_NEG) {
		return	errno;

	} else if(chPid == 0) {
		setsid();

		if(!chmod(scriPath, SCRIPT_AUTHOR)) {
			execl(scriPath, scriPath, NULL);
			otbug_perror("otbug_script_start - execl", errno);

		} else {
			otbug_perror("otbug_script_start - chmod", errno);
		}

		exit(FUN_RET_NEG);
	}

	if(waitpid(chPid, NULL, 0) == FUN_RET_NEG)
		return	errno;

	return	FUN_RUN_END;
}
