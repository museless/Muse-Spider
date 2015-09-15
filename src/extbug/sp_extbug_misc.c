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
	Source file content Six part

	Part Zero:	Include
	Part One:	Local data
	Part Two:	Local function
	Part Three:	Define

	Part Four:	Help print
	Part Five:	Total paper num ctl

--------------------------------------------*/

/*------------------------------------------
	Part Zero: Include
--------------------------------------------*/

#include "spinc.h"
#include "spdb.h"
#include "spextb.h"
#include "speglobal.h"


/*------------------------------------------
	Part One: Local data
--------------------------------------------*/

static	int	nPaperFd;


/*------------------------------------------
	Part Four: Help print

	1. exbug_print_help

--------------------------------------------*/

/*-----exbug_print_help-----*/
void exbug_print_help(void)
{
	char	hlpPath[PATH_LEN];
	char	*pStore;
	int	fSize;

	if(mc_conf_read("extbug_help_locate", CONF_STR, hlpPath, PATH_LEN) == FUN_RUN_FAIL) {
		mc_conf_print_err("extbug_help_locate");
		return;
	}

	if((fSize = read_all_file(&pStore, hlpPath, 0)) == FUN_RUN_FAIL) {
		printf("Extbug---> read_all_file: %s: %s\n", hlpPath, strerror(errno));
		return;
	}

	printf("%.*s\n\n", fSize, pStore);

	free(pStore);
}


/*------------------------------------------
	Part Five: Total paper num ctl

	1. exbug_paper_num_read
	2. exbug_paper_num_inc
	3. exbug_paper_sync

--------------------------------------------*/

/*-----exbug_paper_num_read-----*/
int exbug_paper_num_read(void)
{
	char	fileName[PATH_LEN];
	int	nRet = 0;

	if(mc_conf_read("extbug_paper_num_file", CONF_STR, fileName, PATH_LEN) == FUN_RUN_FAIL) {
		mc_conf_print_err("extbug_paper_num_file");
		return	FUN_RUN_END;
	}

	if((nPaperFd = open(fileName, O_RDWR | O_CREAT, FILE_AUTHOR)) == FUN_RUN_FAIL) {
		perror("Extbug---> exbug_paper_num_read - open");
		return	FUN_RUN_END;
	}

	if(read(nPaperFd, &nRet, sizeof(uLong)) == FUN_RUN_FAIL) {
		perror("Extbug---> exbug_paper_num_read - read");
		return	FUN_RUN_END;
	}

	tPaperNum = (nRet) ? nRet : 0;

	return	FUN_RUN_OK;
}


/*-----exbug_paper_num_inc-----*/
void exbug_paper_num_inc(void)
{
	while(!mato_dec_and_test(&nPaperLock))
		mato_inc(&nPaperLock);

	tPaperNum++;
	mato_inc(&nPaperLock);
}


/*-----exbug_paper_sync-----*/
void exbug_paper_sync(void)
{
	if(nPaperFd) {
		printf("Extbug---> total paper num: %ld\n", tPaperNum);

		if(lseek(nPaperFd, 0, SEEK_SET) == FUN_RUN_FAIL) {
			perror("Extbug---> exbug_paper_sync - lseek");
	
		} else {
			if(write(nPaperFd, &tPaperNum, sizeof(uLong)) == FUN_RUN_FAIL)
				perror("Extbug---> exbug_paper_sync - write");
		}
	
		close(nPaperFd);
	}
}

