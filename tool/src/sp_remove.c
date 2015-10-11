/*******************************
  using for remove same urls
  at every history file

********************************/

#include "spinc.h"

#include <dirent.h>


/* local function */
static	void	tool_check_file(char *fName);

static	char	*strstrn(char *pBuf, char *pFind, int fLimit);


/*----main-----*/
int main(void)
{
	struct	dirent	*pEntry;
	DIR		*pDir;

	if(chdir("/Cai/history") == -1) {
		perror("main - chdir");
		exit(-1);
	}

	if((pDir = opendir("/Cai/history")) == NULL) {
		perror("main - opendir");
		exit(-1);
	}

	while((pEntry = readdir(pDir))) {
		if(pEntry->d_name[0] != '.')
			tool_check_file(pEntry->d_name);
	}

	exit(-1);
}


/*-----tool_check_file-----*/
static void tool_check_file(char *fName)
{
	struct	stat	stBuf;
	char		*tmpBuf, *oldBuf;
	int		openFd, tmpFd;
	int		tbSize = 0;

	if((openFd = open(fName, O_RDWR)) == -1) {
		perror("tool_check_file - open");
		exit(-1);
	}

	if((tmpFd = open("tmp", O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) == -1) {
		perror("tool_check_file - open - tmpFd");
		exit(-1);
	}

	if(fstat(openFd, &stBuf) == -1) {
		perror("tool_check_file - fstat");
		exit(-1);
	}

	if((tmpBuf = calloc(1, stBuf.st_size + 1)) == NULL) {
		perror("tool_check_file - malloc - tmpBuf");
		exit(-1);
	}

	if((oldBuf = malloc(stBuf.st_size + 1)) == NULL) {
		perror("tool_check_file - malloc - oldBuf");
		exit(-1);
	}

	oldBuf[stBuf.st_size] = 0;
	read(openFd, oldBuf, stBuf.st_size);

	char	*pChk, *pEnd, *poMov = oldBuf;

	for(; poMov < oldBuf + stBuf.st_size; poMov++) {
		for(; isdigit(*poMov); poMov++)
			;	/* nothing */

		if(!(pEnd = strchr(poMov, '\n'))) {
			printf("%s wrong %.32s\n", fName, poMov);
			exit(-1);
		}

		if(!strncmp(poMov, "http", 4)) {
			tbSize += sprintf(tmpBuf + tbSize, "%.*s\n", (int)(pEnd - poMov), poMov);
			pChk = pEnd;

			while((pChk = strstrn(pChk + 1, poMov, pEnd - poMov)))
				*pChk = 'f';
		}

		poMov = pEnd;
	}

	write(tmpFd, tmpBuf, tbSize);
	
	free(tmpBuf);
	free(oldBuf);
	close(tmpFd);
	close(openFd);

	remove(fName);
	rename("tmp", fName);
}


/*-----strstrn-----*/
static char *strstrn(char *pBuf, char *pFind, int fLimit)
{
	char	*pRet;
	char	saveCh;

	saveCh = pFind[fLimit];
	pFind[fLimit] = 0;

	pRet = strstr(pBuf, pFind);

	pFind[fLimit] = saveCh;

	return	pRet;
}

