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

	Part Four:	Disc module

--------------------------------------------*/


/*------------------------------------------
	Part Zero: Include
--------------------------------------------*/

#include "spinc.h"
#include "spmpool.h"
#include "spnet.h"
#include "spdb.h"
#include "sptextb.h"
#include "sptglobal.h"


/*------------------------------------------
	Part Two: Local data
--------------------------------------------*/


/*------------------------------------------
	Part Three: Local function
--------------------------------------------*/

extern	inline	int	txbug_tran_disc_real(void *writeFd, BUFF *pBuff);


/*------------------------------------------
	Part Four: Disc module

	1. module_disc_init
	2. txbug_tran_disc
	3. txbug_tran_disc_force
	4. txbug_tran_disc_real
	5. txbug_print_help

--------------------------------------------*/

/*-----module_disc_init-----*/
void module_disc_init(void)
{
	char	datFile[PATH_LEN];

	if(mc_conf_read("default_texttmp_file", CONF_STR, datFile, PATH_LEN) == FUN_RUN_FAIL) {
		printf("textbug - configure file setting wrong - default_texttmp_file\n");
		txbug_sig_error();
	}

	if((txTranHand.tx_fd = open(datFile, O_RDWR | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP)) == FUN_RUN_FAIL) {
		txbug_perror("module_disc_init - open", errno);
		txbug_sig_error();
	}
}


/*-----txbug_tran_disc-----*/
void txbug_tran_disc(void *pText)
{
	TEXT	*pTxt = (TEXT *)pText;

	while(!mato_dec_and_test(&writeDataLock))
		mato_inc(&writeDataLock);

	if(!buff_size_enough(contStoreBuf, strlen(urlConfigSave[pTxt->wt_pattern].c_srcstr) +
	pTxt->wt_plsize + pTxt->wt_tlen + WEB_INDEX_LEN + WEB_TIME_LEN + strlen(pTxt->wt_url))) {
		if(txbug_tran_disc_real(&txTranHand.tx_fd, contStoreBuf) == FUN_RUN_FAIL)
			elog_write("txbug_tran_disc - writen", FUNCTION_STR, ERROR_STR);

		buff_stru_make_empty(contStoreBuf);
	}

	buff_size_add(contStoreBuf, sprintf(buff_place_end(contStoreBuf), "%.*s %s %s\n%s\n%.*s\n",
		pTxt->wt_tlen, pTxt->wt_title, urlConfigSave[pTxt->wt_pattern].c_srcstr,
		pTxt->wt_time, pTxt->wt_url, pTxt->wt_plsize, pTxt->wt_pool));

	mato_inc(&writeDataLock);
}


/*-----txbug_tran_disc_force-----*/
void txbug_tran_disc_force(void *writeFd, MATOS *atoLock, BUFF *pBuff)
{
	while(!mato_dec_and_test(atoLock))
		mato_inc(atoLock);

	if(!buff_stru_empty(pBuff)) {
		if(txbug_tran_disc_real(writeFd, pBuff) == FUN_RUN_FAIL)
			elog_write("txbug_tran_disc_force - writen", FUNCTION_STR, ERROR_STR);
			
		buff_stru_make_empty(pBuff);
	}

	mato_inc(atoLock);
}


/*-----txbug_tran_disc_real-----*/
inline int txbug_tran_disc_real(void *writeFd, BUFF *pBuff)
{
	return	(buff_check_exist(pBuff)) ? 
		writen(*(int *)writeFd, buff_place_start(pBuff), buff_now_size(pBuff)) : FUN_RUN_FAIL;	
}


/*-----txbug_print_help-----*/
void txbug_print_help(void)
{
	char	*hpCont;
	char	hlPath[PATH_LEN];

	if(mc_conf_read("textbug_help_document", CONF_STR, hlPath, PATH_LEN) == FUN_RUN_FAIL) {
		mc_conf_print_err("textbug_help_document");
		exit(FUN_RUN_FAIL);
	}

	if(read_all_file(&hpCont, hlPath, 0) == FUN_RUN_FAIL) {
		txbug_perror("txbug_print_help - read_all_file", errno);
		exit(FUN_RUN_FAIL);
	}

	printf("%s\n\n", hpCont);
	
	free(hpCont);
}
