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


/*******************************************
 * this tool use for formatting sort file
 *******************************************/

/*------------------------------------------
	Source file content Five part

	Part Zero:	Include
	Part One:	Define
	Part Two:	Local data
	Part Three:	Local function

	Part Four:	Format

--------------------------------------------*/

/*------------------------------------------
	Part Zero: Include
--------------------------------------------*/

#include "spinc.h"
#include "spdb.h"


/*------------------------------------------
	Part One: Define
--------------------------------------------*/

/* typedef */
typedef	struct	formatsave	FSV;

/* struct */
struct	formatsave {
	char	*fsv_str;
	int	fsv_len;

	int	fsv_cnt;
};


/*------------------------------------------
	Part Two: Local data
--------------------------------------------*/

static	char	*pFileSaver;
static	FSV	*formatSaveContainer;
static	int	nReadLine = 128;


/*------------------------------------------
	Part Three: Local function
--------------------------------------------*/

/* Part Four */
static void fsv_perror(char *errStr);


/*------------------------------------------
	Part Four: Format

	1. main
	2. fsv_perror

--------------------------------------------*/

/*-----main-----*/
int main(int argc, char **argv)
{
	char	*pBegin, *pMov;
	FSV	*pFsv;
	int	nCir;

	if(argc != 2) {
		printf("FSV---> usage: <file path>\n");
		exit(FUN_RUN_FAIL);
	}

	if(read_all_file(&pFileSaver, argv[1], 0xA) == FUN_RUN_FAIL)
		fsv_perror("main - read_all_file");

	if(!(pFsv = formatSaveContainer = malloc(nReadLine * sizeof(FSV))))
		fsv_perror("main - malloc");

	for(pMov = pBegin = pFileSaver, nCir = 0; nCir < nReadLine; nCir++, pFsv++) {
		pMov = strchr(pBegin, ' ') + 1;
		pFsv->fsv_str = pMov;

		pBegin = strchr(pMov, ' ');
		pFsv->fsv_len = pBegin - pMov;

		pFsv->fsv_cnt = atoi(++pBegin);

		pBegin = strchr(pBegin, '\n') + 1;
	}

	for(pFsv = formatSaveContainer; pFsv < formatSaveContainer + nReadLine; pFsv++)
		printf("%.*s\n", pFsv->fsv_len, pFsv->fsv_str);

	for(pFsv = formatSaveContainer; pFsv < formatSaveContainer + nReadLine; pFsv++)
		printf("%d\n", pFsv->fsv_cnt);

	free(pFileSaver);
	free(formatSaveContainer);

	exit(FUN_RUN_OK);
}


/*-----fsv_perror-----*/
static void fsv_perror(char *errStr)
{
	printf("FSV---> %s: %s\n", errStr, strerror(errno));
	exit(FUN_RUN_FAIL);
}
