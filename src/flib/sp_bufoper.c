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
	Part One:	Define
	Part Two:	Local data
	Part Three:	Local function

	Part Four:	Inline function
	Part Five:	Buff operate
	Part Six:	Buff clter

--------------------------------------------*/

/*------------------------------------------
	Part Zero: Include
--------------------------------------------*/

#include "spinc.h"


/*------------------------------------------
	Part One: Define
--------------------------------------------*/

#define	buff_stru_is_existed(bufStru) \
	(bufStru && bufStru->b_start)


/*------------------------------------------
	Part Four: Inline function

	1. buff_stru_empty
	2. buff_stru_make_empty
	3. buff_size_enough
	4. buff_check_exist
	5. buff_now_size
	6. buff_place_locate
	7. buff_place_end
	8. buff_place_start
	9. buff_size_add
	
--------------------------------------------*/

/*-----buff_stru_empty-----*/
inline int buff_stru_empty(BUFF *pBuff)
{
	if(buff_stru_is_existed(pBuff))
		return	(pBuff->b_size == 0);

	return	FUN_RUN_FAIL;
}


/*-----buff_stru_make_empty-----*/
inline void buff_stru_make_empty(BUFF *pBuff)
{
	if(buff_stru_is_existed(pBuff))
		pBuff->b_size = 0;
}


/*-----buff_size_enough-----*/
inline int buff_size_enough(BUFF *pBuff, int nCheck)
{
	if(buff_stru_is_existed(pBuff))
		return	(pBuff->b_size < pBuff->b_cap - nCheck);

	return	FUN_RUN_FAIL;
}


/*-----buff_check_exist-----*/
inline int buff_check_exist(BUFF *cBuff)
{
	return	buff_stru_is_existed(cBuff);
}


/*-----buff_now_size-----*/
inline int buff_now_size(BUFF *pBuff)
{
	return	pBuff->b_size;
}


/*-----buff_place_locate-----*/
inline char *buff_place_locate(BUFF *pBuff, int nSize)
{
	return	(char *)pBuff->b_start + nSize;
}


/*-----buff_place_end-----*/
inline char *buff_place_end(BUFF *pBuff)
{
	return	(char *)pBuff->b_start + pBuff->b_size;
}


/*-----buff_place_start-----*/
inline char *buff_place_start(BUFF *pBuff)
{
	return	pBuff->b_start;
}


/*-----buff_size_add-----*/
inline void buff_size_add(BUFF *pBuff, int addSize)
{
	pBuff->b_size += addSize;
}


/*------------------------------------------
	Part Five: Buff operate

	1. buff_stru_init
	2. buff_stru_all_free

--------------------------------------------*/

/*-----buff_stru_init-----*/
BUFF *buff_stru_init(int nMalloc)
{
	BUFF	*pSbuf;

	if((pSbuf = malloc(sizeof(BUFF))) == NULL)
		return	NULL;

	if((pSbuf->b_start = malloc(nMalloc)) == NULL)
		return	NULL;

	pSbuf->b_cap = nMalloc;
	pSbuf->b_size = 0;

	((char *)pSbuf->b_start)[0] = 0;

	return	pSbuf;
}


/*-----buff_stru_free_all-----*/
void buff_stru_free_all(void *bufStru)
{
	BUFF	*tranStru = (BUFF *)bufStru;

	if(tranStru) {
		if(tranStru->b_start)
			free(tranStru->b_start);

		free(tranStru);
	}
}


/*------------------------------------------
	Part Six: Buff clter

	1. buff_stru_strstr

--------------------------------------------*/

/*-----buff_stru_strstr-----*/
char *buff_stru_strstr(BUFF *strBuf, char *needStr)
{
	return	(buff_stru_is_existed(strBuf)) ?
		strnstr((char *)strBuf->b_start, needStr, strBuf->b_size) : NULL;
}
