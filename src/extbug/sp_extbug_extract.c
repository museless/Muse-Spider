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
	Source file content Seven part

	Part Zero:	Include
	Part One:	Local data
	Part Two:	Local function
	Part Three:	Define

	Part Four:	Practise part
	Part Five:	Extract part

--------------------------------------------*/

/*------------------------------------------
	Part Zero: Include
--------------------------------------------*/

#include "spinc.h"
#include "spdb.h"
#include "spmsg.h"
#include "spmpool.h"
#include "spdict.h"

#include "mmdpool.h"

#include "spextb.h"
#include "speglobal.h"
#include <math.h>


/*------------------------------------------
	Part Two: Local function
--------------------------------------------*/


/*------------------------------------------
	Part Two: Local function
--------------------------------------------*/

/* Part Five */


/*------------------------------------------
	Part Three: Define
--------------------------------------------*/


/*------------------------------------------
	Part Four: Practise part

	1. exbug_extract_practise

--------------------------------------------*/

/*-----exbug_extract_practise-----*/
void exbug_extract_practise(WDCT *praDic)
{
	WST	*strList = praDic->wc_list;
	BUFF	*upBuf;

	if((upBuf = buff_stru_init(SQL_BCOM_LEN)) == NULL) {
		elog_write("exbug_extract_practise - buff_stru_init", FUNCTION_STR, ERROR_STR);
		exbug_sig_error(PTHREAD_ERROR);
	}

	for(; strList; strList = strList->ws_next) {
		if(!buff_size_enough(upBuf, PRACTICE_UP_MAX)) {
			while(!mato_dec_and_test(&dicDbLock))
				mato_inc(&dicDbLock);

			if(mysql_real_query(&dbDicHandler, buff_place_start(upBuf), buff_now_size(upBuf)) != FUN_RUN_END) {
				if(exbug_dberr_deal(&dbDicHandler, dbDicName, 
				"exbug_extract_practise - mysql_real_query") != FUN_RUN_OK) {
					buff_stru_free_all(upBuf);
					exbug_sig_error(PTHREAD_ERROR);
				}
			}

			buff_stru_make_empty(upBuf);
			mato_inc(&dicDbLock);
		}

		buff_size_add(upBuf, (buff_stru_empty(upBuf) ? 
		sprintf(buff_place_end(upBuf), UPDATE_WORD_IDF, tblWordName, strList->ws_bytes, strList->ws_buf) :
		sprintf(buff_place_end(upBuf), UPDATE_KEEP_WD, strList->ws_bytes, strList->ws_buf)));
	}

	buff_stru_free_all(upBuf);
	exbug_paper_num_inc();
}


/*------------------------------------------
	Part Five: Extract part

	1. exbug_extract_keyword
	2. exbug_update_terms

--------------------------------------------*/

/*-----exbug_extract_keyword-----*/
void exbug_extract_keyword(WDCT *extDic)
{
	//exbug_word_print(extDic);

	return;
}


/*-----exbug_update_terms-----*/
void exbug_update_terms(WDCT *upList, const char *pInd)
{
	WST	*pList = upList->wc_list;
	char	*upString;
	int	nOff, nCnt;

	if(!(upString = mmdp_malloc(threadMemPool, (upList->wc_tbytes + (upList->wc_ndiff * PER_WD_MORE)) + 1))) {
		elog_write("exbug_update_terms - mmdp_malloc", FUNCTION_STR, ERROR_STR);
		exbug_sig_error(PTHREAD_ERROR);
	}

	for(nOff = nCnt = 0; pList && nCnt < upMaxTerms; nCnt++, pList = pList->ws_next) {
		if(nOff + pList->ws_bytes > nKlistSize)
			break;	

		nOff += sprintf(&upString[nOff], TERMS_FORMAT, pList->ws_bytes, pList->ws_buf);
	}

	while(!mato_dec_and_test(&dicDbLock))
		mato_inc(&dicDbLock);

	if(!buff_size_enough(extSaveBuf, nOff + UPKEY_OTH_MAX)) {
		if(mysql_real_query(&dbKeysHandler, buff_place_start(extSaveBuf), buff_now_size(extSaveBuf))) {
			if(exbug_dberr_deal(&dbKeysHandler, dbKeysName, 
			"exbug_update_terms - mysql_real_query") != FUN_RUN_OK)
				exbug_sig_error(PTHREAD_ERROR);
		}

		buff_stru_make_empty(extSaveBuf);
	}

	buff_size_add(extSaveBuf, (buff_stru_empty(extSaveBuf) ? 
	sprintf(buff_place_start(extSaveBuf), INSERT_KEYWD, tblKeysName, pInd, nOff, upString, upList->wc_ndiff) :
	sprintf(buff_place_end(extSaveBuf), INSERT_KW_NEXT, pInd, nOff, upString, upList->wc_ndiff)));

	mato_inc(&dicDbLock);
}


/*------------------------------------------
	Part Six: Word list ctl

	1. sp_wordlist_mov_next
	2. sp_wordlist_cpy

--------------------------------------------*/

/*-----sp_wordlist_mov_next-----*/
inline WST *sp_wordlist_mov_next(WST *wStru, int nMov)
{
	for(; nMov > 0 && wStru; nMov--, wStru = wStru->ws_next)
		;	/* nothing */

	return	wStru;
}
