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

	Part Four:	Print Help tip
	Part Five:	Disc module

--------------------------------------------*/


/*------------------------------------------
	Part Zero: Include
--------------------------------------------*/

#include "spinc.h"
#include "sphtml.h"
#include "spnet.h"
#include "spdb.h"
#include "spmpool.h"
#include "spmsg.h"

#include "spurlb.h"
#include "spuglobal.h"


/*------------------------------------------
	Part One: Local data
--------------------------------------------*/

static	int	datFileDes;

/* extern */
extern	inline	int	ubug_tran_disc_real(void);


/*------------------------------------------
	Part Four: Print Help tip

	1. ubug_print_help

--------------------------------------------*/

/*-----ubug_print_help-----*/
void ubug_print_help(void)
{
	struct	stat	stBuf;

	char	*hpCont;
	int	hpFd;

	if((hpFd = open("/MuseSp/src/urlbug/urlbug.hlp", O_RDWR)) == FUN_RUN_FAIL) {
		perror("ubug_print_help - open");
		exit(FUN_RUN_FAIL);
	}

	if(fstat(hpFd, &stBuf) == FUN_RUN_FAIL) {
		perror("ubug_print_help - fstat");
		exit(FUN_RUN_FAIL);	
	}

	if((hpCont = malloc(stBuf.st_size + 1)) == NULL) {
		perror("ubug_print_help - malloc");
		exit(FUN_RUN_FAIL);
	}

	hpCont[stBuf.st_size] = 0;
	if(readn(hpFd, hpCont, stBuf.st_size) == FUN_RUN_FAIL) {
		perror("ubug_print_help - readn");
		exit(FUN_RUN_FAIL);
	}

	printf("%s\n", hpCont);
	
	free(hpCont);
	close(hpFd);
}


/*------------------------------------------
	Part Five: Disc module

	1. ubug_disc_init
	2. ubug_tran_disc
	3. ubug_tran_disc_force
	4. ubug_tran_disc_real
	5. ubug_tran_disc_clean

--------------------------------------------*/

/*-----ubug_disc_init-----*/
void ubug_disc_init(void)
{
	char	datFile[PATH_LEN];

	if(mc_conf_read("default_tmpfile_locate", CONF_STR, datFile, PATH_LEN) == FUN_RUN_FAIL) {
		printf("urlbug - configure file setting wrong - default_tmpfile_locate\n");
		mc_conf_unload();
		exit(FUN_RUN_FAIL);
	}

	if((datFileDes = open(datFile, O_RDWR | O_TRUNC | O_CREAT)) == FUN_RUN_FAIL) {
		ubug_perror("ubug_disc_init - open", errno);
		mc_conf_unload();
		exit(FUN_RUN_FAIL);
	}
}


/*-----ubug_tran_disc-----*/
void ubug_tran_disc(char *pUrl, int uLen, int numPattern)
{
	while(!mato_dec_and_test(&writeStoreLock))
		mato_inc(&writeStoreLock);

	if(!buff_size_enough(urlBufStu, URL_LEN)) {
		if(ubug_tran_disc_real() == FUN_RUN_FAIL) {
			elog_write("ubug_tran_disc - writen", FUNCTION_STR, ERROR_STR);
			ubug_sig_error();
		}
	
		buff_stru_make_empty(urlBufStu);
	}

	buff_size_add(urlBufStu, sprintf(buff_place_end(urlBufStu), 
			"%.*s, %d\n", uLen, pUrl, numPattern)); 

	mato_inc(&writeStoreLock);
}


/*-----ubug_tran_disc_force-----*/
void ubug_tran_disc_force(void)
{
	while(!mato_dec_and_test(&writeStoreLock))
		mato_inc(&writeStoreLock);

	if(!buff_stru_empty(urlBufStu)) {
		if(ubug_tran_disc_real() == FUN_RUN_FAIL) {
			elog_write("ubug_tran_disc_force - ubug_tran_disc_real", FUNCTION_STR, ERROR_STR);
			ubug_sig_error();
		}
	
		buff_stru_make_empty(urlBufStu);
	}

	mato_inc(&writeStoreLock);
}


/*-----ubug_tran_disc_real-----*/
inline int ubug_tran_disc_real(void)
{
	return	(buff_check_exist(urlBufStu)) ? writen(datFileDes, buff_place_start(urlBufStu), 
		buff_now_size(urlBufStu)) : FUN_RUN_FAIL;
}


/*-----ubug_tran_disc_clean-----*/
void ubug_tran_disc_clean(void)
{
	
}
