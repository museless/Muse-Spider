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
	Source file content Eight part

	Part Zero:	Include
	Part One:	Define
	Part Two:	Local data
	Part Three:	Local function

	Part Four:	Configure file control

--------------------------------------------*/


/*------------------------------------------
	Part Zero: Include
--------------------------------------------*/

#include "spinc.h"
#include <sys/user.h>


/*------------------------------------------
	Part Two: Local data
--------------------------------------------*/

static	int	confStoreFd;
static	BUFF	confctlBuff;
static	char	userName[MIDDLE_BUF + 1];


/*------------------------------------------
	Part Four: Configure file control

	1. mc_conf_load
	2. mc_conf_read
	3. mc_conf_unload
	4. mc_conf_print_err

--------------------------------------------*/

/*-----mc_conf_load-----*/
int mc_conf_load(char *pUser, char *confPath)
{
	struct	stat	stBuf;

	sprintf(userName, "%.*s", MIDDLE_BUF, pUser);

	if((confStoreFd = open(confPath, O_RDWR)) == FUN_RUN_FAIL)
		return	FUN_RUN_FAIL;

	if(fstat(confStoreFd, &stBuf) == FUN_RUN_FAIL)
		return	FUN_RUN_FAIL;

	confctlBuff.b_size = confctlBuff.b_cap = stBuf.st_size + 1;

	if((confctlBuff.b_start = malloc(confctlBuff.b_cap)) == NULL)
		return	FUN_RUN_FAIL;

	if(readn(confStoreFd, confctlBuff.b_start, stBuf.st_size) == FUN_RUN_FAIL)
		return	FUN_RUN_FAIL;

	((char *)confctlBuff.b_start)[stBuf.st_size] = 0;

	return	FUN_RUN_OK;
}


/*-----mc_conf_read-----*/
int mc_conf_read(char *findStr, int dType, void *dBuf, int dLen)
{
	char	*pStr, *pEnd;
	int	nMul = 0;

	if(!(pStr = strstr(confctlBuff.b_start, findStr)))
		return	FUN_RUN_FAIL;

	if((pStr = strchr(pStr, '=')) == NULL)
		return	FUN_RUN_FAIL;

	for(pStr += 1; isspace(*pStr); pStr++)
		;	/* nothing */

	if((pEnd = strchr(pStr, '\n')) == NULL)
		return	FUN_RUN_FAIL;

	if(dType == CONF_NUM) {
		for(; isspace(*pEnd) || isalpha(*pEnd); pEnd--)
			;	/* nothing */
	
		pEnd++;

		if(*pEnd == 'M'|| *pEnd == 'm')
			nMul = 20;
		else if(*pEnd == 'K'|| *pEnd == 'k')
			nMul = 10;

		*(int *)dBuf = ((int)strtol(pStr, NULL, 0)) << nMul;

	} else if(dType == CONF_STR) {
		dLen = (((pEnd - pStr) > dLen) ? dLen : pEnd - pStr);
		strncpy((char *)dBuf, pStr, dLen);
		((char *)dBuf)[dLen] = 0;

	} else {
		return	FUN_RUN_FAIL;
	}

	return	FUN_RUN_OK;
}


/*-----mc_conf_unload-----*/
void mc_conf_unload(void)
{
	if(confStoreFd < FUN_RUN_END)
		close(confStoreFd);
	
	if(confctlBuff.b_start)
		free(confctlBuff.b_start);
}


/*-----mc_conf_print_err-----*/
void mc_conf_print_err(char *pFind)
{
	printf("%s---> configure read [%s] failed\n", userName, pFind);
}
