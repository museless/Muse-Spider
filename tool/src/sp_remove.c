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

