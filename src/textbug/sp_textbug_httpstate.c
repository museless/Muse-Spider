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

	Part Four:	Http state handling

--------------------------------------------*/

/*------------------------------------------
	Part Zero: Include
--------------------------------------------*/

#include "spinc.h"
#include "spdb.h"
#include "spnet.h"
#include "sphtml.h"

#include "spmpool.h"
#include "sptglobal.h"
#include "sptextb.h"


/*------------------------------------------
	Part Four: Http state handling

	1. txbug_dispose_http301

--------------------------------------------*/

/*-----txbug_dispose_http301-----*/
int txbug_dispose_http301(char *retBuf, TEXT *pText)
{
	char	*pEnter;

	if(!(retBuf = strstr(retBuf, MATCH_LOCA)))
		return	FUN_RUN_END;

	if(!(pEnter = strstr(retBuf += MLOCA_LEN, MATCH_LINKBRK)))
		return	FUN_RUN_END;

	if(!buff_size_enough(urlSaveBuf, SQL_PERCOM_MLEN)) {
		if(txbug_tran_db_news(&urlDataBase, urlSaveBuf)) {
			if(txbug_dberr_dispose(&urlDataBase, urlsDbName, 
			"txbug_dispose_http301 - txbug_tran_db_news") < FUN_RUN_OK)
				txbug_sig_error();
		}

		buff_stru_make_empty(urlSaveBuf);
	}

	buff_size_add(urlSaveBuf, ((buff_stru_empty(urlSaveBuf)) ? 
	sprintf(buff_place_end(urlSaveBuf), TRAN_URL_BEG, urlsTblName, (int)(pEnter - retBuf), retBuf, pText->wt_pattern) : 
	sprintf(buff_place_end(urlSaveBuf), TRAN_URL, (int)(pEnter - retBuf), retBuf, pText->wt_pattern)));

	return	FUN_RUN_OK;
}
