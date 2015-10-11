/*------------------------------------------
	Source file content Seven part

	Part Zero:	Include
	Part One:	Local data
	Part Two:	Local function
	Part Three:	Define

	Part Four:	Segment main
	Part Five:	Wordlist control
	Part Six:	WordStru control

--------------------------------------------*/

/*------------------------------------------
	Part Zero: Include
--------------------------------------------*/

#include "spinc.h"
#include "spdb.h"
#include "mmdpool.h"
#include "spextb.h"
#include "speglobal.h"


/*------------------------------------------
	Part One: Local data
--------------------------------------------*/

static	SEPWORD	sepStrStore[] = {{"。"}, {"，"}, {"、"}, {"；"}, {"‘"}, {"’"}, {"“"}, 
			{"”"}, {"［"}, {"］"}, {"｛"}, {"｝"}};

static	int	nSepStr = sizeof(sepStrStore) / sizeof(sepStrStore[0]);


/*------------------------------------------
	Part Two: Local function
--------------------------------------------*/

/* Part Four */
static	void	exbug_simple_segment(WDCT *pCnt, const char *strBeg, int sLen);
static	int	exbug_check_word_head(const char *pHead);
static	WHEAD	*extbug_search_head(CLISTS *pList, const char *pCmp, int wLen);


/*------------------------------------------
	Part Four: Segment main

	1. exbug_segment_entrance
	2. exbug_simple_segment
	3. exbug_check_word_head
	4. extbug_search_head

--------------------------------------------*/

/*-----ext_segment_entrance-----*/
void exbug_segment_entrance(WDCT *wcStru, const char *pNews)
{
	const	char	*pMov, *pStr;
		int	nRet, newLen = strlen(pNews);

	for(pStr = pMov = pNews; *pMov && pMov < pNews + newLen; ) {
		if((nRet = exbug_check_word_head(pMov)) == FUN_RUN_FAIL) {
			pMov += UTF8_WORD_LEN;
			continue;
		}

		exbug_simple_segment(wcStru, pStr, pMov - pStr);

		pStr = pMov += ((nRet == FUN_RUN_OK) ? 1 : UTF8_WORD_LEN);
	}

	//exbug_word_print(wcStru);
}


/*-----exbug_simple_segment-----*/
static void exbug_simple_segment(WDCT *pCnt, const char *strBeg, int sLen)
{
	const	char	*strEnd;
		char	*strHead, *strMov;
		WHEAD	*headStru;
		uLong	*pList;
		int	wordSize, wBytes, nCnt, nOff;

	for(strEnd = strBeg + sLen; sLen > 0 && strBeg < strEnd; strBeg += wordSize) {
		wordSize = ((sLen > BYTE_CMP_MAX) ? WORD_CMP_MAX : (sLen / UTF8_WORD_LEN));

		for(; wordSize > 1; wordSize--) {
			if((headStru = extbug_search_head(&charHeadSave, strBeg, wordSize))) {
				strHead = (*((WDCB **)&charTermList + (wordSize - 2)))->wb_lterms;
				strMov = strHead + headStru->dc_off;
				wBytes = wordSize * UTF8_WORD_LEN;
			
				for(nCnt = 0; nCnt < headStru->dc_cnt; nCnt++, strMov += wBytes) {
					//printf("%#x - %#x - %d\n", strMov, strHead, wordSize);
					if(!strncmp(strMov, strBeg, wBytes))
						break;
				}

				if(nCnt < headStru->dc_cnt) {
					nOff = (strMov - strHead) / wBytes;
					pList = (*(((WDCB **)&charTermList + (wordSize - 2))))->wb_ltimes;
					nCnt = (pList) ? *(pList + nOff) : 0;
					break;
				}
			}
		}

		wordSize *= UTF8_WORD_LEN;
		sLen -= wordSize;

		/* wordSize will be all Bytes */
		exbug_word_add(pCnt, strBeg, wordSize, nCnt);
	}
}


/*-----exbug_check_word_head-----*/
static int exbug_check_word_head(const char *pHead)
{
	int	nCir;

	if((*pHead & UTF8_WHEAD) == UTF8_WHEAD) {
		for(nCir = 0; nCir < nSepStr; nCir++) {
			if(!strncmp(pHead, sepStrStore[nCir].sep_utf8, UTF8_WORD_LEN))
				return	FUN_RUN_END;
		}

		return	FUN_RUN_FAIL;
	}

	return	FUN_RUN_OK;
}


/*-----extbug_search_head-----*/
static WHEAD *extbug_search_head(CLISTS *headList, const char *pCmp, int wLen)
{
	WHEAD	*pWord;

	if(exbug_check_word_head(pCmp) != FUN_RUN_FAIL)
		return	NULL;

	for(pWord = *((WHEAD **)headList + (wLen - 2)); pWord->dc_cnt != -1 && pWord->dc_off != -1; pWord++) {
		if(!strncmp(pCmp, pWord->dc_utf8, UTF8_WORD_LEN))
			return	pWord;
	}

	return	NULL;
}


/*------------------------------------------
	Part Five: Wordlist control

	1. exbug_word_add
	2. exbug_word_print

--------------------------------------------*/

/*-----exbug_word_add-----*/
void exbug_word_add(WDCT *addCnt, const char *addStr, int addSize, int nTimes)
{
	WST	**wdList;

	if(addSize < TWO_U8WORD_LEN)
		return;

	for(wdList = &addCnt->wc_list; *wdList; wdList = &((*wdList)->ws_next)) {
		if((*wdList)->ws_bytes == addSize) {
			if(!strncmp((*wdList)->ws_buf, addStr, addSize))
				break;
		}
	}

	if(!(*wdList)) {
		if(!(*wdList = mmdp_malloc(threadMemPool, sizeof(WST)))) {
			elog_write("exbug_word_add - mmdp_malloc", "wdList", "failed");
			exbug_sig_error(PTHREAD_ERROR);
		}

		(*wdList)->ws_buf = (char *)addStr;
		(*wdList)->ws_next = NULL;
		(*wdList)->ws_bytes = addSize;
		(*wdList)->ws_cnt = 0;

		addCnt->wc_ndiff++;
		addCnt->wc_tbytes += addSize;
	}

	(*wdList)->ws_cnt++;
	addCnt->wc_total++;
}


/*-----exbug_word_print-----*/
void exbug_word_print(WDCT *printCnt)
{
	WST	*pList = printCnt->wc_list;

	for(; pList; pList = pList->ws_next)
		printf("%.*s - %d\n", pList->ws_bytes, pList->ws_buf, pList->ws_cnt);

	printf("\n\n");
}


/*------------------------------------------
	Part Six: WordStru control

	1. exbug_wordstru_setting

--------------------------------------------*/

/*-----exbug_wordstru_setting-----*/
void exbug_wordstru_setting(WDCT *setCnt)
{
	setCnt->wc_list = NULL;
	setCnt->wc_ndiff = setCnt->wc_total = 0;
	setCnt->wc_tbytes = 0;
}
