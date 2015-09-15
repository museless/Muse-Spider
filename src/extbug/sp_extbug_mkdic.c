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
	Source file content Sixteen part

	Part Zero:	Include
	Part One:	Local data
	Part Two:	Local function
	Part Three:	Define
	Part Four:	Mkdic main
	Part Five:	Initialization
	Part Six:	Help
	Part Seven:	Public use
	Part Eight:	Misc

	Part Nine:	Relist
	Part Ten:	Add part
	Part Eleven:	Add slave part
	Part Twelve:	Down part
	Part Thirteen:	Down slave part
	Part Fourteen:	Make script part
	Part Fiveteen:	Delete part

--------------------------------------------*/

/*------------------------------------------
	Part Zero: Include
--------------------------------------------*/

#include "spinc.h"
#include "spdb.h"
#include "spdict.h"
#include "mmdpool.h"
#include <dirent.h>


/*------------------------------------------
	Part One: Local data
--------------------------------------------*/

/* Typedef */
typedef	struct	dirent	DIRENT;

/* Data */
static	MGCH	*pExtGc;
static	DMPH	*extMemPool;
static	char	*pDicStore;
static	MKRUN	*mkRunSet;

static	char	dicBufName[NAMBUF_LEN];
static	char	tblNameStore[NAMBUF_LEN];
static	MYSQL	mkdicSqlHand;


/*------------------------------------------
	Part Two: Local function
--------------------------------------------*/

/* Part Four */
static	int	mk_command_analyst(char **pComm, int nPara);
static	int	mk_comm_detail_analyst(char **pComm, int nPatt, int *nOff, int nLimit);

/* Part Five */
static	int	mk_database_init(MYSQL *pSql);

/* Part Six */
static	void	mk_mkdic_print_help(void);

/* Part Seven */
static	void	mk_mysql_close(MYSQL *pSql);
static	void	mk_mysql_show_error(MYSQL *errSql);
static	int	mk_check_tran_word(MYSQL *pSql, BUFF *pBuff, char *wordBuf, int wLen);
static	int	mk_force_tran_word(MYSQL *pSql, BUFF *pBuff);
static	MKRUN	*mk_run_init(void);

/* Part Eight */
static	inline	int	mk_utf8_compare(char *cmpOne, char *cmpTwo);
static	inline	void	mk_unicode_to_utf8(uChar *pUnic, uChar *uEnd, uChar *pUtf8);

/* Part Nine */
static	void	mk_relist_run(int nChose);
static	int	mk_relist_init(void);
static	int	mk_relist_work(char *pName);
static	int	mk_relist_utfbuf_init(WORDS *wPoint);
static	int	mk_relist_word_init(int *pDic, int *pIndex, char *strConf, char *namBeg);
static	void	mk_findex_write(char *pFindex, int *indexBuf);

/* Part Ten */
static	void	mk_add_run(int nChose);
static	int	mk_add_extract_chose(MKRUN *mkRun, int nChose);
static	void	mk_extract_scel(int bufSize);
static	int	mk_word_check_exist(MYSQL *chkSql, BUFF *bStru, char *pUword, int nLen);
static	void	mk_normal_update(int fileSize);

/* Part Eleven */
static	inline	int	mk_withs_sprint_beg(char *strBeg, int termSize, char *pTerm, int tLen);
static	inline	int	mk_withs_sprint_next(char *strBeg, int termSize, char *pTerm, int tLen);
static	inline	int	mk_nots_sprint_beg(char *strBeg, int termSize, char *pTerm, int tLen);
static	inline	int	mk_nots_sprint_next(char *strBeg, int termSize, char *pTerm, int tLen);

/* Part Twelve */
static	void	mk_down_run(int nChose);
static	int	mk_down_extract_chose(MKRUN *downRun, int nChose);
static	void	mk_down_word(int writeFd);

/* Part Thirteen */
static	inline	int	mk_down_withs(char *strBeg, MYSQL_ROW wRow);
static	inline	int	mk_down_nots(char *strBeg, MYSQL_ROW wRow);

/* Part Fourteen */
static	void	mk_mscript_run(void);

/* Part Fiveteen */
static	void	mk_delete_run(void);
static	int	mk_delete_init(void);
static	void	mk_delete_extract(int fileSize);


/*------------------------------------------
	Part Three: Define
--------------------------------------------*/

#define	MAX_WRITE_LEN	0x38

#define SCRIPT_STRING	"/MuseSp/bin/mkdic --add noun scel 1 %s/%s\n"


/*------------------------------------------
	Part Four: Mkdic main

	1. main
	2. mk_command_analyst
	3. mk_comm_detail_analyst

--------------------------------------------*/

/*-----main-----*/
int main(int argc, char **argv)
{
	int	nChose;

	if((pExtGc = mgc_init()) == NULL) {
		printf("---> extbug Gc init failed\n");
		exit(FUN_RUN_FAIL);
	}

	switch((nChose = mk_command_analyst(argv, argc)) & LEVEL_MASK) {
	  case	DR_ADD:
	  	mk_add_run(nChose);
		break;

	  case	DR_DOWN:
	  	mk_down_run(nChose);
		break;

	  case	DR_RELIST:
	  	mk_relist_run(nChose);
		break;

	  case	DR_MSCRIPT:
	  	mk_mscript_run();
	  	break;

	  case	DR_DELETE:
		mk_delete_run();
		break;

	  default: 
		printf("Mkdic---> parameters is not complete\n\n");
	}

	mgc_all_clean(pExtGc);

	exit(FUN_RUN_FAIL);
}


/*-----mk_command_analyst-----*/
static int mk_command_analyst(char **pComm, int nPara)
{
	int	nCir, nDoor, hlpFlags, confOff;

	for(hlpFlags = confOff = nDoor = 0, nCir = 1; nCir < nPara; nCir++) {
		if(!strcmp(pComm[nCir], "--help") || !strcmp(pComm[nCir], "-h")) {
			hlpFlags = 1;

		} else if(!strcmp(pComm[nCir], "--add") && !nDoor) {
			nDoor = (nCir + ADD_LIMIT <= nPara) ?
				mk_comm_detail_analyst(pComm, DR_ADD, &nCir, nCir + ADD_LIMIT) : DR_WRONG;

		} else if(!strcmp(pComm[nCir], "--down") && !nDoor) {
			nDoor = (nCir + DOWN_LIMIT <= nPara) ?
				mk_comm_detail_analyst(pComm, DR_DOWN, &nCir, nCir + DOWN_LIMIT) : DR_WRONG;

		} else if(!strcmp(pComm[nCir], "--relist") && !nDoor) {
			nDoor = (nCir + RELIST_LIMIT <= nPara) ?
				mk_comm_detail_analyst(pComm, DR_RELIST, &nCir, nCir + RELIST_LIMIT) : DR_WRONG;

		} else if(!strcmp(pComm[nCir], "--make_script") && !nDoor) {
			snprintf(dicBufName, NAMBUF_LEN, "%s", pComm[++nCir]);
			nDoor = DR_MSCRIPT;

		} else if(!strcmp(pComm[nCir], "--delete") && !nDoor) {
			nDoor = DR_DELETE;

		}else if(!strcmp(pComm[nCir], "--conf")) {
			confOff = ++nCir;

		} else {
			printf("Mkdic---> wrong parameters: %s\n \
			\r--->please try \"-h\" or \"--help\"\n\n", pComm[nCir]);
			exit(FUN_RUN_END);
		}
	}

	if(mc_conf_load("Mkdic", ((confOff) ? pComm[confOff] : "/MuseSp/conf/dictool.cnf")) == FUN_RUN_FAIL) {
		printf("Mkdic---> load configure failed\n");
		perror("Mkdic---> mk_command_analyst - mc_conf_load");
		mc_conf_unload();
		return	DR_WRONG;
	}

	if(nCir == 1 || hlpFlags) {
		mk_mkdic_print_help();
		mc_conf_unload();
		exit(FUN_RUN_OK);
	}

	if(mgc_add(pExtGc, NULL_POINT, (gcfun)mc_conf_unload) == MGC_FAILED)
		perror("Mkdic---> mk_command_analyst - mgc_add - mc_conf_unload");

	return	nDoor;
}


/*-----mk_comm_detail_analyst-----*/
static int mk_comm_detail_analyst(char **pComm, int nPatt, int *nOff, int nLimit)
{
	int	nSave, nChose = nPatt;

	if(nPatt == DR_ADD || nPatt == DR_DOWN) {
		/* character type */
		(*nOff)++;

		if(!strcmp(pComm[(*nOff)], "all")) {
			nSave = DR_ALL_PHRASE;

		} else if(!strcmp(pComm[(*nOff)], "noun")) {
			nSave = DR_NOUN_PHRASE;

		} else {
			printf("Mkdic---> wrong character: %s\n", pComm[(*nOff)]);
			return	DR_WRONG;
		}

		nChose |= nSave;

		if(nPatt == DR_ADD) {
			/* file type */
			(*nOff)++;
	
			if(!strcmp(pComm[(*nOff)], "normal")) {
				nSave = DR_NORMAL_FILE;
	
			} else if(!strcmp(pComm[(*nOff)], "scel")) {
				nSave = DR_SCEL_FILE;
	
			} else {
				printf("Mkdic---> wrong file type: %s\n", pComm[(*nOff)]);
				return	DR_WRONG;
			}
	
			nChose |= nSave;
		}

		/* state */
		nChose |= (atoi(pComm[++(*nOff)]) << LEVEL_FOUR_SHIFT);

		/* src or dst */
 		snprintf(dicBufName, NAMBUF_LEN, "%s", pComm[++(*nOff)]);
		++(*nOff);

	} else if(nPatt == DR_RELIST) {
		(*nOff)++;

		if(!strcmp(pComm[(*nOff)], "all")) {
			nSave = DR_ALL_PHRASE;

		} else if(!strcmp(pComm[(*nOff)], "noun")) {
			nSave = DR_NOUN_PHRASE;

		} else {
			printf("Mkdic---> wrong character: %s\n", pComm[(*nOff)]);
			return	DR_WRONG;
		}

		nChose |= nSave;

	} else {
		nChose = DR_WRONG;
	}

	return	nChose;
}


/*------------------------------------------
	Part Five: Initialization

	1. mk_database_init

--------------------------------------------*/

/*-----mk_database_init-----*/
static int mk_database_init(MYSQL *pSql)
{
	char	dbName[SQL_DBNAME_LEN];

	if(mc_conf_read("word_list_dbname", CONF_STR, dbName, SQL_DBNAME_LEN) == FUN_RUN_FAIL) {
		mc_conf_print_err("word_list_dbname");
		return	FUN_RUN_FAIL;
	}

	mysql_library_init(nrOpt, sqlOpt, NULL);

	if(!mysql_init(pSql)) {
		perror("Mkdic---> mk_database_init - mysql_init");
		return	FUN_RUN_FAIL;
	}

	if(!mysql_real_connect(pSql, NULL, DBUSRNAME, DBUSRKEY, dbName, 0, NULL, 0)) {
		printf("Mkdic---> mk_database_init - mysql_real_connect failed\n");
		return	FUN_RUN_FAIL;
	}

	if(mgc_add(pExtGc, pSql, (gcfun)mk_mysql_close) == FUN_RUN_FAIL)
		printf("Mkdic---> mk_database_init - mgc_add Sql failed\n");

	return	FUN_RUN_OK;
}


/*------------------------------------------
	Part Six: Help

	1. mk_mkdic_print_help

--------------------------------------------*/

/*-----mk_mkdic_print_help-----*/
static void mk_mkdic_print_help(void)
{
	char	*hpCont;
	char	hlPath[PATH_LEN];

	if(mc_conf_read("mkdic_help_path", CONF_STR, hlPath, PATH_LEN) == FUN_RUN_FAIL) {
		mc_conf_print_err("mkdic_help_path");
		exit(FUN_RUN_FAIL);
	}

	if(read_all_file(&hpCont, hlPath, 0) == FUN_RUN_FAIL) {
		perror("Mkdic---> mk_mkdic_print_help - read_all_file");
		exit(FUN_RUN_FAIL);
	}

	printf("%s\n\n", hpCont);
	
	free(hpCont);
}


/*------------------------------------------
	Part Seven: Public use

	1. mk_mysql_close
	2. mk_mysql_show_error
	3. mk_check_tran_word
	4. mk_force_tran_word
	5. mk_run_init	

--------------------------------------------*/

/*-----mk_mysql_close-----*/
static void mk_mysql_close(MYSQL *pSql)
{
	mysql_close(pSql);
	mysql_library_end();
}


/*-----mk_mysql_show_error-----*/
static void mk_mysql_show_error(MYSQL *errSql)
{
	printf("Mkdic---> mk_mysql_show_error - mysql_query: %d - %s\n", mysql_errno(errSql), mysql_error(errSql));
}


/*-----mk_check_tran_word-----*/
static int mk_check_tran_word(MYSQL *pSql, BUFF *pBuff, char *wordBuf, int wLen)
{
	int	fRet, tSize = wLen * UTF8_WORD_LEN;

	if((fRet = mk_word_check_exist(pSql, pBuff, wordBuf, tSize)) != FUN_RUN_FAIL) {
		if(fRet == 0) {
			if(!buff_size_enough(pBuff, SQL_TCOM_LEN)) {
				if(mysql_real_query(pSql, buff_place_start(pBuff), buff_now_size(pBuff))) {
					mk_mysql_show_error(pSql);
					buff_stru_make_empty(pBuff);

					if(mysql_errno(pSql) != ER_DUP_ENTRY)
						return	FUN_RUN_FAIL;
				}
		
				buff_stru_make_empty(pBuff);
			}
		
			buff_size_add(pBuff, ((buff_stru_empty(pBuff) == FUN_RUN_OK) ? 
			mkRunSet->ar_spbeg(buff_place_start(pBuff), tSize, wordBuf, wLen) :
			mkRunSet->ar_spnext(buff_place_end(pBuff), tSize, wordBuf, wLen)));
		}

		return	FUN_RUN_OK;
	}

	return	fRet;
}


/*-----mk_force_tran_word-----*/
static int mk_force_tran_word(MYSQL *pSql, BUFF *pBuff)
{
	if(buff_stru_empty(pBuff) == FUN_RUN_END) {
		if(mysql_real_query(pSql, buff_place_start(pBuff), buff_now_size(pBuff)) != FUN_RUN_END) {
			mk_mysql_show_error(pSql);
			buff_stru_make_empty(pBuff);

			if(mysql_errno(pSql) != 1062)
				return	FUN_RUN_FAIL;
		}
		
		buff_stru_make_empty(pBuff);
	}

	return	FUN_RUN_OK;
}


/*-----mk_run_init-----*/
static MKRUN *mk_run_init(void)
{
	MKRUN	*tmpRun;

	if(!(tmpRun = malloc(sizeof(MKRUN)))) {
		perror("Mkdic---> mk_run_init - malloc");
		return	NULL;
	}

	if(mgc_add(pExtGc, tmpRun, free) == FUN_RUN_FAIL)
		perror("Mkdic---> mk_run_init - mgc_add");

	return	tmpRun;
}


/*------------------------------------------
	Part Eight: Misc

	1. mk_utf8_compare
	2. mk_unicode_to_utf8

--------------------------------------------*/

/*-----mk_utf8_compare-----*/
static inline int mk_utf8_compare(char *cmpOne, char *cmpTwo)
{
	int	nDiff;

	if(!(nDiff = (uChar)cmpOne[0] - (uChar)cmpTwo[0])) {
		if(!(nDiff = (uChar)cmpOne[1] - (uChar)cmpTwo[1]))
			nDiff = (uChar)cmpOne[2] - (uChar)cmpTwo[2];
	}

	return	(nDiff == 0) ? 0 : ((nDiff < 0) ? -1 : 1);
}


/*-----mk_unicode_to_utf8-----*/
static inline void mk_unicode_to_utf8(uChar *pUnic, uChar *uEnd, uChar *pUtf8)
{
	/* we must be careful for signed and unsigned, 
	   because when you >> or <<, at gcc, moving signed 
	   will bring the signed mark move together */

	for(; pUnic < uEnd; pUnic += 2, pUtf8 += 3) {
		pUtf8[0] = 0xE0 | (pUnic[1] >> 4);
		pUtf8[1] = 0x80 | ((pUnic[1] & 0xF) << 2) | (pUnic[0] >> 6);
		pUtf8[2] = 0x80 | (pUnic[0] & 0x3F);
	}

	*pUtf8 = 0;
}


/*------------------------------------------
	Part Nine: Relist

	1. mk_relist_run
	2. mk_relist_init
	3. mk_relist_work
	4. mk_relist_utfbuf_init
	5. mk_relist_word_init
	6. mk_findex_write

--------------------------------------------*/

/*-----mk_relist_run-----*/
static void mk_relist_run(int nChose)
{
	if(mk_relist_init() != FUN_RUN_FAIL) {
		switch(nChose & LEVEL_TWO_MASK) {
		  case	DR_ALL_PHRASE:
			mk_relist_work("all");
			break;

		  case	DR_NOUN_PHRASE:
			mk_relist_work("noun");
			break;
		}

		free(pDicStore);
	}
}


/*-----mk_relist_init-----*/
static int mk_relist_init(void)
{
	char	strBuf[MIDDLE_BUF];

	if(mc_conf_read("char_path", CONF_STR, strBuf, MIDDLE_BUF) == FUN_RUN_FAIL) {
		mc_conf_print_err("char_path");
		return	FUN_RUN_FAIL;
	}

	if(read_all_file(&pDicStore, strBuf, 0) == FUN_RUN_FAIL) {
		perror("Mkdic---> mk_relist_init - read_all_file");
		return	FUN_RUN_FAIL;
	}

	if((extMemPool = mmdp_create(PER_MP_BLOCK_SIZE)) == NULL) {
		perror("Mkdic---> mk_relist_init - mmdp_create");
		return	FUN_RUN_FAIL;
	}

	if(mgc_add(pExtGc, extMemPool, (gcfun)mmdp_free_all) == FUN_RUN_FAIL) {
		printf("Mkdic---> mk_relist_init - mgc_add - pExtGc failed\n");
		return	FUN_RUN_FAIL;
	}

	if(mk_database_init(&mkdicSqlHand) != FUN_RUN_OK)
		return	FUN_RUN_FAIL;

	if(mgc_add(pExtGc, &mkdicSqlHand, (gcfun)mk_mysql_close) == FUN_RUN_FAIL) {
		printf("Mkdic---> mk_relist_init - mgc_add - mkdicSqlHand failed\n");
		return	FUN_RUN_FAIL;
	}

	return	FUN_RUN_OK;
}


/*-----mk_relist_work-----*/
static int mk_relist_work(char *pName)
{
	WORDS		uWordBuf[UTF8_WORD_CNT];
	char		bufStore[SQL_TCOM_LEN];
	int		fdSave[WORD_LEN_LIMIT];
	int		indexSave[WORD_LEN_LIMIT];
	int		offSave[WORD_LEN_LIMIT], cntSave[WORD_LEN_LIMIT];
	MYSQL_RES	*extRes;
	MYSQL_ROW	extRow;
	WLISTS		wordList;
	WSTRS		*strWord, **pList, **pEnd;
	int		nRet, actLen, nCount, nStr, nTotal, tOff;

	/* relist work init */
	actLen = mk_relist_utfbuf_init(uWordBuf);

	/* dbtable init */
	sprintf(bufStore, "%s_word_table", pName);

	if(mc_conf_read(bufStore, CONF_STR, tblNameStore, NAMBUF_LEN) == FUN_RUN_FAIL) {
		mc_conf_print_err(bufStore);
		return	FUN_RUN_FAIL;
	}

	/* dict save path init */
	sprintf(bufStore, "%s_save_path", pName);

	if(mk_relist_word_init(fdSave, indexSave, bufStore, pName) != FUN_RUN_OK)
		return	FUN_RUN_FAIL;

	memset(offSave, 0, WORD_LEN_LIMIT * sizeof(int));
	memset(cntSave, 0, WORD_LEN_LIMIT * sizeof(int));

	/* start working */
	for(nCount = 0; nCount < actLen; nCount++) {
		nRet = sprintf(bufStore, GET_LIMIT_WORD, tblNameStore, uWordBuf[nCount].w_utf8);
		memset(&wordList, 0, sizeof(WLISTS));

		if(mysql_real_query(&mkdicSqlHand, bufStore, nRet) != FUN_RUN_END) {
			printf("Mkdic---> mk_relist_word - mysql_real_query failed");
			return	FUN_RUN_FAIL;
		}

		if(!(extRes = mysql_store_result(&mkdicSqlHand))) {
			printf("Mkdic---> mk_relist_word - mysql_store_result failed\n");
			return	FUN_RUN_FAIL;
		}

		uWordBuf[nCount].w_smax = 0;

		while((extRow = mysql_fetch_row(extRes))) {
			nStr = atoi(extRow[1]);
			pList = (WSTRS **)&wordList.w_n2 + (nStr - 2);

			if(nStr > uWordBuf[nCount].w_smax)
				uWordBuf[nCount].w_smax = nStr;

			if((strWord = mmdp_malloc(extMemPool, sizeof(WSTRS))) == NULL) {
				perror("Mkdic---> mk_relist_work - mmdp_malloc - newWord");
				return	FUN_RUN_FAIL;
			}

			strWord->w_str = (uChar *)extRow[0];
			strWord->w_next = *pList;
			*pList = strWord;
		}

		if(uWordBuf[nCount].w_smax) {
			pList = (WSTRS **)&wordList.w_n2;
			pEnd = (WSTRS **)&wordList.w_n2 + (uWordBuf[nCount].w_smax - 1);

			for(nStr = 2; pList < pEnd; pList++, nStr++) {
				if(!pList)
					continue;

				for(tOff = nTotal = 0, strWord = *pList; strWord; strWord = strWord->w_next, nTotal++) {
					nRet = sprintf(bufStore, "%.*s", nStr * 3, strWord->w_str);
					tOff += nRet;
					write(fdSave[nStr - 2], bufStore, nRet);
				}

				if(nTotal) {
					nRet = sprintf(bufStore, INDEX_STRING, 
						uWordBuf[nCount].w_utf8, offSave[nStr - 2], nTotal);
					write(indexSave[nStr - 2], bufStore, nRet);
					offSave[nStr - 2] += tOff;
					cntSave[nStr - 2] += nTotal;
				}
			}
		}

		mysql_free_result(extRes);
	}

	/* write findex */
	sprintf(bufStore, "%s_findex_path", pName);
	mk_findex_write(bufStore, cntSave);

	return	FUN_RUN_OK;
}


/*-----mk_relist_utfbuf_init-----*/
static int mk_relist_utfbuf_init(WORDS *wPoint)
{
	char	*pMov = pDicStore;
	int	nCnt;

	for(nCnt = 0; nCnt < UTF8_WORD_CNT && *pMov; nCnt++, pMov++) {
		strncpy((char *)wPoint[nCnt].w_utf8, pMov, UTF8_WORD_LEN);
		wPoint[nCnt].w_smax = 0;

		if(!(pMov = strchr(pMov, '\n')))
			break;
	}

	return	nCnt;
}


/*-----mk_relist_word_init-----*/
static int mk_relist_word_init(int *pDic, int *pIndex, char *strConf, char *namBeg)
{
	char	nameBuf[PATH_LEN];
	char	pathName[PATH_LEN];
	int	nCnt;

	if(mc_conf_read(strConf, CONF_STR, pathName, PATH_LEN) == FUN_RUN_FAIL) {
		mc_conf_print_err(strConf);
		return	FUN_RUN_FAIL;
	}

	/* deleting the word '/' */
	nCnt = strlen(pathName);
	if(pathName[nCnt - 1] == '/')
		pathName[nCnt - 1] = 0;

	mkdir(pathName, FILE_AUTHOR);

	for(nCnt = 0; nCnt < WORD_LEN_LIMIT; nCnt++) {
		sprintf(nameBuf, "%s/%s%d", pathName, namBeg, nCnt + 2);

		if((pDic[nCnt] = open(nameBuf, O_RDWR | O_CREAT | O_TRUNC, FILE_AUTHOR)) == FUN_RUN_FAIL) {
			perror("Mkdic---> mk_relist_word_init - open - Char");
			return	FUN_RUN_FAIL;
		}

		sprintf(nameBuf, "%s/Index%d", pathName, nCnt + 2);

		if((pIndex[nCnt] = open(nameBuf, O_RDWR | O_CREAT | O_TRUNC, FILE_AUTHOR)) == FUN_RUN_FAIL) {
			perror("Mkdic---> mk_relist_word_init - open - Index");
			return	FUN_RUN_FAIL;
		}
	}

	return	FUN_RUN_OK;
}


/*-----mk_findex_write-----*/
static void mk_findex_write(char *pFindex, int *indexBuf)
{
	char	indexFile[PATH_LEN];
	int	indexFd, nCnt, nRet;

	if(mc_conf_read(pFindex, CONF_STR, indexFile, PATH_LEN) == FUN_RUN_FAIL) {
		mc_conf_print_err(pFindex);
		return;
	}

	if((indexFd = open(indexFile, O_RDWR | O_TRUNC | O_CREAT, FILE_AUTHOR)) == FUN_RUN_FAIL) {
		perror("mk_findex_write - open");
		return;
	}

	for(nCnt = 0; nCnt < WORD_LEN_LIMIT; nCnt++) {
		nRet = sprintf(indexFile, "%d\n", indexBuf[nCnt]);
		write(indexFd, indexFile, nRet);
	}

	close(indexFd);
	return;
}


/*------------------------------------------
	Part Ten: Add part

	1. mk_add_run
	2. mk_add_extract_chose
	3. mk_extract_scel
	4. mk_word_check_exist
	5. mk_normal_update

--------------------------------------------*/

/*-----mk_add_run-----*/
static void mk_add_run(int nChose)
{
	int	fileSize;

	if(!(mkRunSet = mk_run_init()))
		return;

	if(mk_add_extract_chose(mkRunSet, nChose) == FUN_RUN_OK) {
		if((fileSize = read_all_file(&pDicStore, dicBufName, mkRunSet->ar_cutlen)) == FUN_RUN_FAIL) {
			perror("Mkdic---> mk_add_run - read_all_file");
			return;
		}

		if(mk_database_init(&mkdicSqlHand) != FUN_RUN_OK)
			return;

		if(mkRunSet->ar_extract && fileSize > 0)
			mkRunSet->ar_extract(fileSize);
	}
}


/*-----mk_add_extract_chose-----*/
static int mk_add_extract_chose(MKRUN *mkRun, int nChose)
{
	char	confSave[MIDDLE_BUF];

	/* level two */
	switch(nChose & LEVEL_TWO_MASK) {
	  case	DR_ALL_PHRASE:
		sprintf(confSave, "all_word_table");

		if((mkRun->ar_state = (nChose & LEVEL_FOUR_MASK) >> LEVEL_FOUR_SHIFT) == DR_NO_STATE)
			mkRun->ar_state = DR_UNDETE_STATE;

		break;

	  case	DR_NOUN_PHRASE:
		sprintf(confSave, "noun_word_table");
		mkRun->ar_state = DR_NO_STATE;
		break;

	  default:
	  	return	FUN_RUN_FAIL;
	}

	/* read the table name */
	if(mc_conf_read(confSave, CONF_STR, tblNameStore, NAMBUF_LEN) == FUN_RUN_FAIL) {
		mc_conf_print_err(confSave);
		return	FUN_RUN_FAIL;
	}

	/* level three */
	switch(nChose & LEVEL_THREE_MASK) {
	  case	DR_NORMAL_FILE:
		mkRun->ar_cutlen = 0;
		mkRun->ar_extract = mk_normal_update;
		break;

	  case	DR_SCEL_FILE:
		mkRun->ar_cutlen = FSCEL_WORD_OFF;
		mkRun->ar_extract = mk_extract_scel;

		if(mkRun->ar_state == DR_IN_STATE)
			mkRun->ar_state = DR_UNDETE_STATE;

		break;

	  default:
		return	FUN_RUN_FAIL;
	}

	mkRun->ar_spbeg = (mkRun->ar_state == DR_NO_STATE) ?
	mk_nots_sprint_beg : mk_withs_sprint_beg;

	mkRun->ar_spnext = (mkRun->ar_state == DR_NO_STATE) ?
	mk_nots_sprint_next: mk_withs_sprint_next;

	return	FUN_RUN_OK;
}


/*-----mk_extract_scel-----*/
static void mk_extract_scel(int bufSize)
{
	char	*pDict = pDicStore;
	char	u8Word[UTF8_BUF_SIZE + 1];
	BUFF	*newBuf;
	int	wSize;
	short	tSame;

	if(!(newBuf = buff_stru_init(RECE_DATA))) {
		perror("Mkdic---> mk_extract_scel - buff_stru_init");
		return;
	}

	while(pDict < (pDicStore + bufSize)) {
		tSame = *((short *)pDict);
		pDict += 0x2;
		wSize = *((short *)pDict);
		pDict += (wSize + 2);
	
		while(tSame--) {
			wSize = *((short *)pDict);
			pDict += 2;

			if(wSize <= MAX_UNI_LEN) {
				mk_unicode_to_utf8((uChar *)pDict, (uChar *)pDict + wSize, (uChar *)u8Word);
				if(mk_check_tran_word(&mkdicSqlHand, newBuf, u8Word, wSize >> 1) == FUN_RUN_FAIL)
					return;
			}
	
			pDict += (wSize + 0xC);
		}
	}

	mk_force_tran_word(&mkdicSqlHand, newBuf);

	free(pDicStore);
	mk_mysql_close(&mkdicSqlHand);
}


/*-----mk_word_check_exist-----*/
static int mk_word_check_exist(MYSQL *chkSql, BUFF *bStru, char *pUword, int nLen)
{
	char	seleCom[SQL_TCOM_LEN];
	int	fRet;

	sprintf(seleCom, REVIEW_WORD, tblNameStore, nLen, pUword);
		
	if((fRet = mysql_string_exist_check(chkSql, seleCom)) == FUN_RUN_FAIL) {
		printf("Mkdic---> mk_word_check_exist - db oper failed\n");
		return	FUN_RUN_FAIL;
	}

	if(!fRet) {
		sprintf(seleCom, "\"%.*s\"", nLen, pUword);

		if(buff_stru_strstr(bStru, seleCom))
			fRet = 1;
	}

	return	(fRet) ? 1 : 0;
}


/*-----mk_normal_update-----*/
static void mk_normal_update(int fileSize)
{
	BUFF	*extBuff;
	char	*pEnd, *pEnter, *pMov = pDicStore;

	if((extBuff = buff_stru_init(SQL_BCOM_LEN)) == NULL) {
		perror("Mkdic---> mk_normal_update - buff_stru_init");
		return;
	}

	for(; *pMov && pMov < pDicStore + fileSize; pMov = pEnter + 1) {
		if((pEnd = pEnter = strchr(pMov, '\n')) == NULL)
			break;

		if(mkRunSet->ar_state == DR_IN_STATE) {
			if((pEnd = strchr(pMov, '\t')) == NULL)
				break;

			mkRunSet->ar_instate = atoi(pEnd + 1);
			if(mkRunSet->ar_instate == DR_NO_STATE || mkRunSet->ar_instate == DR_IN_STATE)
				mkRunSet->ar_instate = DR_UNDETE_STATE;
		}

		if(mk_check_tran_word(&mkdicSqlHand, extBuff, pMov, (pEnd - pMov) / UTF8_WORD_LEN) == FUN_RUN_FAIL) {
			mk_mysql_show_error(&mkdicSqlHand);
			break;
		}
	}

	mk_force_tran_word(&mkdicSqlHand, extBuff);

	buff_stru_free_all(extBuff);
	free(pDicStore);
}


/*------------------------------------------
	Part Eleven: Add slave part

	1. mk_withs_sprint_beg
	2. mk_withs_sprint_next
	3. mk_nots_sprint_beg
	4. mk_nots_sprint_next

--------------------------------------------*/

/*-----mk_withs_sprint_beg-----*/
static inline int mk_withs_sprint_beg(char *strBeg, int termSize, char *pTerm, int tLen)
{
	int	nState;

	nState = (mkRunSet->ar_state == DR_IN_STATE) ? mkRunSet->ar_instate : mkRunSet->ar_state;

	return	sprintf(strBeg, TRAN_WD_BEG_ST, tblNameStore, termSize, pTerm, tLen, nState);
}


/*-----mk_withs_sprint_next-----*/
static inline int mk_withs_sprint_next(char *strBeg, int termSize, char *pTerm, int tLen)
{
	int	nState;

	nState = (mkRunSet->ar_state == DR_IN_STATE) ? mkRunSet->ar_instate : mkRunSet->ar_state;

	return	sprintf(strBeg, TRAN_WD_ST, termSize, pTerm, tLen, nState);
}


/*-----mk_nots_sprint_beg-----*/
static inline int mk_nots_sprint_beg(char *strBeg, int termSize, char *pTerm, int tLen)
{
	return	sprintf(strBeg, TRAN_WD_BEG, tblNameStore, termSize, pTerm, tLen);
}


/*-----mk_nots_sprint_next-----*/
static inline int mk_nots_sprint_next(char *strBeg, int termSize, char *pTerm, int tLen)
{
	return	sprintf(strBeg, TRAN_WD, termSize, pTerm, tLen);
}


/*------------------------------------------
	Part Twelve: Down part

	1. mk_down_run
	2. mk_down_extract_chose

--------------------------------------------*/

/*-----mk_down_run-----*/
static void mk_down_run(int nChose)
{
	int	nFd;

	if(!(mkRunSet = mk_run_init()))
		return;

	if(mk_down_extract_chose(mkRunSet, nChose) == FUN_RUN_OK) {
		if((nFd = open(dicBufName, O_CREAT | O_TRUNC | O_RDWR , FILE_AUTHOR)) == FUN_RUN_FAIL) {
			perror("Mkdic---> mk_down_run - open");
			return;
		}

		if(mk_database_init(&mkdicSqlHand) != FUN_RUN_OK)
			return;

		mk_down_word(nFd);
		close(nFd);
	}
}


/*-----mk_down_extract_chose-----*/
static int mk_down_extract_chose(MKRUN *downRun, int nChose)
{
	char	confSave[MIDDLE_BUF];

	/* level two */
	switch(nChose & LEVEL_TWO_MASK) {
	  case	DR_ALL_PHRASE:
		sprintf(confSave, "all_word_table");
		downRun->ar_state = ((nChose >> LEVEL_FOUR_SHIFT) & LEVEL_MASK);
		break;

	  case	DR_NOUN_PHRASE:
		sprintf(confSave, "noun_word_table");
		downRun->ar_state = DR_WITHOUT_STATE;
		break;

	  default:
	  	return	FUN_RUN_FAIL;
	}

	/* read the table name */
	if(mc_conf_read(confSave, CONF_STR, tblNameStore, NAMBUF_LEN) == FUN_RUN_FAIL) {
		mc_conf_print_err(confSave);
		return	FUN_RUN_FAIL;
	}

	downRun->ar_selesent = (downRun->ar_state == DR_WITHOUT_STATE) ?
	DOWN_WD_NO_ST : DOWN_WD_ST;

	downRun->ar_dnfun = (downRun->ar_state == DR_WITHOUT_STATE) ?
	(dnfun)mk_down_nots : (dnfun)mk_down_withs;

	return	FUN_RUN_OK;
}


/*-----mk_down_word-----*/
static void mk_down_word(int writeFd)
{
	MSLRES	*pRes;
	MSLROW	pRow;
	BUFF	*dnBuff;
	char	seleCom[SQL_TCOM_LEN];

	if(!(dnBuff = buff_stru_init(HUGE_BUF))) {
		perror("Mkdic---> mk_down_word - buff_stru_init");
		return;
	}

	if(mysql_real_query(&mkdicSqlHand, seleCom, sprintf(seleCom, mkRunSet->ar_selesent, tblNameStore))) {
		mk_mysql_show_error(&mkdicSqlHand);
		return;
	}

	if(!(pRes = mysql_use_result(&mkdicSqlHand))) {
		mk_mysql_show_error(&mkdicSqlHand);
		return;
	}

	while((pRow = mysql_fetch_row(pRes))) {
		if(!buff_size_enough(dnBuff, MAX_WRITE_LEN)) {
			if(write(writeFd, buff_place_start(dnBuff), buff_now_size(dnBuff)) == FUN_RUN_FAIL) {
				buff_stru_make_empty(dnBuff);
				perror("Mkdic---> mk_down_word - write");
			}
		
			buff_stru_make_empty(dnBuff);
		}
		
		buff_size_add(dnBuff, mkRunSet->ar_dnfun(buff_place_end(dnBuff), pRow));
	}

	mysql_free_result(pRes);
	buff_stru_free_all(dnBuff);
}


/*------------------------------------------
	Part Thirteen: Down slave part

	1. mk_down_withs
	2. mk_down_nots

--------------------------------------------*/

/*-----mk_down_withs-----*/
static inline int mk_down_withs(char *strBeg, MYSQL_ROW wRow)
{
	return	sprintf(strBeg, "%s\t%s\n", wRow[0], wRow[1]);
}


/*-----mk_down_nots_init-----*/
static inline int mk_down_nots(char *strBeg, MYSQL_ROW wRow)
{
	return	sprintf(strBeg, "%s\n", wRow[0]);
}


/*------------------------------------------
	Part Fourteen: Make script part

	1. mk_mscript_run

--------------------------------------------*/

/*-----mk_mscript_run-----*/
static void mk_mscript_run(void)
{
	DIR	*readDir, *chDir;
	DIRENT	*chDirent, *inDirent;
	FILE	*scrFile;

	if(mc_conf_read("add_keyword_script", CONF_STR, tblNameStore, NAMBUF_LEN) == FUN_RUN_FAIL) {
		mc_conf_print_err("add_keyword_script");
		return;
	}

	if(!(scrFile = fopen(tblNameStore, "w+"))) {
		printf("Mkdic---> mk_mscript_run - fopen - %s: %s\n", tblNameStore, strerror(errno));
		return;
	}

	fprintf(scrFile, "# add_keyword.sh !/bin/bash\n\n");

	if(!(readDir = opendir(dicBufName))) {
		printf("Mkdic---> mk_mscript_run - opendir - %s: %s\n", dicBufName, strerror(errno));
		return;
	}

	while((chDirent = readdir(readDir))) {
		if(chDirent->d_name[0] != '.') {
			snprintf(tblNameStore, NAMBUF_LEN, "%s/%s", dicBufName, chDirent->d_name);

			if(!(chDir = opendir(tblNameStore))) {
				printf("Mkdic---> mk_mscript_run - opendir - %s: %s\n", 
				chDirent->d_name, strerror(errno));
				break;
			}

			while((inDirent = readdir(chDir))) {
				if(inDirent->d_name[0] != '.')
					fprintf(scrFile, SCRIPT_STRING, tblNameStore, inDirent->d_name);
			}

			closedir(chDir);
		}
	}

	fclose(scrFile);
	closedir(readDir);
}


/*------------------------------------------
	Part Fiveteen: Delete part

	1. mk_delete_run
	2. mk_delete_init
	3. mk_delete_extract

--------------------------------------------*/

/*-----mk_delete_run-----*/
static void mk_delete_run(void)
{
	int	fileSize;

	if((fileSize = mk_delete_init()) != FUN_RUN_FAIL)
		mk_delete_extract(fileSize);
}


/*-----mk_delete_init-----*/
static int mk_delete_init(void)
{
	int	fileSize;

	/* read delete file */
	if(mc_conf_read("delete_save_path", CONF_STR, dicBufName, NAMBUF_LEN) == FUN_RUN_FAIL) {
		mc_conf_print_err("delete_save_path");
		return	FUN_RUN_FAIL;
	}

	if((fileSize = read_all_file(&pDicStore, dicBufName, 0)) == FUN_RUN_FAIL) {
		perror("Mkdic---> mk_delete_run - read_all_file");
		return	FUN_RUN_FAIL;
	}

	if(mgc_add(pExtGc, pDicStore, free) == FUN_RUN_FAIL)
		perror("Mkdic---> mk_delete_run - mgc_add");

	/* read noun table name */
	if(mc_conf_read("noun_word_table", CONF_STR, tblNameStore, NAMBUF_LEN) == FUN_RUN_FAIL) {
		mc_conf_print_err("noun_word_table");
		return	FUN_RUN_FAIL;
	}

	/* init database */
	if(mk_database_init(&mkdicSqlHand) != FUN_RUN_OK)
		return	FUN_RUN_FAIL;

	if(mgc_add(pExtGc, &mkdicSqlHand, (gcfun)mk_mysql_close) == FUN_RUN_FAIL) {
		printf("Mkdic---> mk_relist_init - mgc_add - mkdicSqlHand failed\n");
		return	FUN_RUN_FAIL;
	}

	return	fileSize;
}


/*-----mk_delete_extract-----*/
static void mk_delete_extract(int fileSize)
{
	char	*pEnter, *pMov = pDicStore;
	int	nLen;

	for(; *pMov && pMov < pDicStore + fileSize; pMov = pEnter + 1) {
		if((pEnter = strchr(pMov, '\n')) == NULL)
			break;

		nLen = snprintf(dicBufName, NAMBUF_LEN, DELETE_KEYWORD, tblNameStore, (int)(pEnter - pMov), pMov);

		if(mysql_real_query(&mkdicSqlHand, dicBufName, nLen))
			mk_mysql_show_error(&mkdicSqlHand);
	}
}
