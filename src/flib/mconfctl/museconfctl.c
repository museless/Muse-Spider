#include "spinc.h"
#include <sys/user.h>


/* local data */
static	int	confStoreFd;
static	BUFF	confctlBuff;
static	char	userName[MIDDLE_BUF + 1];


/*------------------------------------------
	Part Zero: Configure file control

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
	printf("%s - configure read [%s] failed\n", userName, pFind);
}
