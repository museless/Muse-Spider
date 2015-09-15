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
	Otbug keyword relate and sort

	Source file content Thirteen part

	Part Zero:	Include
	Part One:	Define
	Part Two:	Local data
	Part Three:	Local function

	Part Four:	Main
	Part Five:	Signal operate

	Part Six:	Relate init
	Part Seven:	Sort operate

	Part Eight:	Database oper
	Part Nine:	Error handling

	Part Ten:	Relate entrance
	Part Eleven:	Relate hash
	Part Twelve: 	Relate operation

--------------------------------------------*/

/*------------------------------------------
	Part Zero: Include
--------------------------------------------*/

#include "spotool.h"
#include "malgo.h"
#include "spextb.h"


/*------------------------------------------
	Part One: Define
--------------------------------------------*/

/* Define */
#define	GET_KWLIST	"select Ind, Klist, Keynum from %s where Keyflags=1 limit %d"
#define	UPDATE_KFLAGS	"update %s set Keyflags=Keyflags+1 where Ind=\"%s\""

#define	TMP_BUF_SIZE	0x200

#define	DEF_TCBUF_SIZE	0xC80
#define	DEF_TCBUF_OFF	0x3

#define	NTERMS_LEN_MAX	0xA

#define	otbug_relate_align(pSaver, nMov) \
((TSAVC *)((char *)pSaver + nMov * sizeof(TSAVC *)))

#define	otbug_relate_align(pSaver, nMov) \
((TSAVC *)((char *)pSaver + nMov * sizeof(TSAVC *)))

/* typedef */
typedef	struct	termsav	TESAV;
typedef	struct	tsavctl	TSAVC;
typedef	struct	tsavpac	TSPA;

typedef	struct	tsortbf	TSBUF;

/* struct */
struct	termsav {
	char	*te_str;
	int	*te_pcnt;

	int	te_len;
};

struct	tsavctl {
	HBET	*tsl_hash;
	TSPA	*tsl_prela;

	TESAV	tsl_term;
};

struct	tsavpac {
	TSAVC	*tp_avc;
	TSPA	*tp_next;

	int	*tp_cnt;	/* the appear time */
};

struct	tsortbf {
	TSAVC	**tb_buf;	/* point to buffer start */
	TSAVC	**tb_cur;	/* point to current */

	uLong	tb_max;		/* max term count */
	uLong	tb_cnt;		/* current term count */
};


/*------------------------------------------
	Part Two: Local data
--------------------------------------------*/

static	MGCH	*orlGarCol;
static	MGCO	orlResultCol;

static	BUFF	*orlTermBuf;
static	MSHM	*orlShmOwner;
static	DMPH	*orlMpHandler, *tmpMpHandler;
static	TLCTL	*orlShmCtler;
static	HBET	*orlHashBucket;
static	TSBUF	orlTsBuf;

static	MYSQL	orlKeyDatabase;

static	char	cntLogPath[PATH_LEN];

static	int	orlDownFig, nRelateBuck, rankFileDesc, nTopTerms;
static	uLong	cntLogSave[DEF_TCBUF_OFF];

static	char	tblKeyName[SQL_TABNAME_LEN], dbKeyName[SQL_DBNAME_LEN];


/*------------------------------------------
	Part Three: Local function
--------------------------------------------*/

/* Part Five */
static	void	otbug_relate_signal_handler(int nSign);

/* Part Six */
static	int	mainly_init(void);
static	int	otbug_relate_mpool_init(void);
static	int	otbug_relate_database_init(void);
static	int	otbug_relate_hbucket_init(void);
static	int	otbug_relate_buff_init(void);

/* Part Seven */
static	int	otbug_sort_init(char *rankPath);
static	int	otbug_sort_insert(TSAVC *pSaver);
static	void	otbug_sort_sorting(void);
static	void	otbug_sort_destroy(void);
static	TSPA	*otbug_relate_search_linker(TSAVC *pCtl);

/* Part Eight */
static	void	otbug_relate_inc_flags(char *pID);
static	void	otbug_relate_database_close(void);

/* Part Nine */
static	void	otbug_relate_error(char *errStr, int nError);
static	int	otbug_relate_dberr(MYSQL *mySql, char *errStr);

/* Part Ten */
static	void	otbug_relate_entrance(void);

/* Part Eleven */
static	void	*otbug_relate_term_alloc(void *pTerms, int datLen, int *pCnt);
static	int	otbug_relate_term_cmp(void *beCmped, void *cmpEr, int cmpLen);
static	uLong	otbug_relate_term_hash(void *pData, int datLen);

static	void	*otbug_relate_relate_alloc(void *pTerms, int datLen, TSAVC *pTsavc);
static	int	otbug_relate_relate_cmp(void *beCmped, void *cmpEr, int cmpLen);

/* Part Twelve */
static	int	otbug_relate_work(MSLROW datRow);
static	void	otbug_relate_output(int nPrint);


/*------------------------------------------
	Part Four: Main

	1. main

--------------------------------------------*/

/*-----main------*/
int main(void)
{
	if(mainly_init() && otbug_tool_signal_init(otbug_relate_signal_handler, otbug_relate_error)) {
		if(otbug_relate_mpool_init() && otbug_relate_hbucket_init() && otbug_relate_database_init()) {
			if(otbug_relate_buff_init())
				otbug_relate_entrance();
		}
	}

	mgc_all_clean(orlGarCol);

	return	FUN_RUN_END;
}


/*------------------------------------------
	Part Five: Signal operate

	1. otbug_relate_signal_handler

--------------------------------------------*/

/*-----otbug_relate_signal_handler-----*/
static void otbug_relate_signal_handler(int nSign)
{
	if(nSign == SIGINT) {
		printf("ORL---> catch a SIGINT\n");
		otbug_relate_output(40);

	} else if(nSign == SIGUSR1) {
		return;
	
	} else if(nSign == SIGUSR2) {
		printf("ORL---> relate running was killed by himself\n");

	} else if(nSign == SIGSEGV) {
		printf("ORL---> catch a SIGSEGV\n");

	} else {
		printf("ORL---> unknown signal: %d\n", nSign);
	}

	mgc_all_clean(orlGarCol);
	mgc_one_clean(&orlResultCol);

	printf("ORL---> quitting\n");

	exit(FUN_RUN_FAIL);
}


/*------------------------------------------
	Part Six: Relate init

	1. mainly_init
	2. otbug_relate_mpool_init
	3. otbug_relate_database_init
	4. otbug_relate_hbucket_init
	5. otbug_relate_buff_init

--------------------------------------------*/

/*-----mainly_init-----*/
static int mainly_init(void)
{
	char	strPath[PATH_LEN];

	if(!otbug_tool_frame_init("ORL", &orlGarCol, &orlShmOwner, otbug_relate_error))
		return	FUN_RUN_END;

	/* mgc one init */
	if(mgc_one_init(&orlResultCol, (gcfun)mysql_free_result, TRY_A_LIMIT) == MGC_FAILED) {
		otbug_relate_error("mainly_init - mgc_one_init", errno);
		return	FUN_RUN_END;
	}

	/* read shm reader */
	orlShmCtler = otbug_shm_tcler_get(orlShmOwner);

	/* read pthread limit num */
	if(mc_conf_read("orl_sql_down_num", CONF_NUM, &orlDownFig, sizeof(int)) == FUN_RUN_FAIL) {
		mc_conf_print_err("orl_sql_down_num");
		return	FUN_RUN_END;
	}

	/* sign up */
	otbug_tool_signup(strPath, orlShmCtler, otbug_relate_error);

	/* sorting init */
	if(!(otbug_sort_init(strPath))) {
		otbug_relate_error("mainly_init - otbug_sort_init", errno);
		return	FUN_RUN_END;
	}

	if(mgc_add(orlGarCol, NULL_POINT, (gcfun)otbug_sort_destroy) == MGC_FAILED)
		otbug_relate_error("mainly_init - mgc_add", errno);

	return	FUN_RUN_OK;
}


/*-----otbug_relate_mpool_init-----*/
static int otbug_relate_mpool_init(void)
{
	/* memory pool init */
	if(!(orlMpHandler = mmdp_create(OTBUG_MP_HASH))) {
		otbug_relate_error("otbug_relate_mpool_init - mmdp_create", errno);
		return	FUN_RUN_END;
	}

	if(mgc_add(orlGarCol, orlMpHandler, (gcfun)mmdp_free_all) == MGC_FAILED)
		otbug_relate_error("otbug_relate_mpool_init - mgc_add - orl", errno);

	if(!(tmpMpHandler = mmdp_create(TMP_BUF_SIZE))) {
		otbug_relate_error("otbug_relate_mpool_init - mmdp_create - tmp", errno);
		return	FUN_RUN_END;
	}

	if(mgc_add(orlGarCol, tmpMpHandler, (gcfun)mmdp_free_all) == MGC_FAILED)
		otbug_relate_error("otbug_relate_mpool_init - mgc_add - tmp", errno);

	return	FUN_RUN_OK;
}


/*-----otbug_relate_database_init-----*/
static int otbug_relate_database_init(void)
{
	mysql_library_init(nrOpt, sqlOpt, NULL);

	if(!mysql_init(&keysDataBase)) {
		printf("ORL---> otbug_relate_database_init - mysql_init - failed\n");
		kill(getpid(), SIGUSR2);
	}

	if(mc_conf_read("keys_database_name", CONF_STR, dbKeyName, SQL_DBNAME_LEN) == FUN_RUN_FAIL) {
		mc_conf_print_err("urls_database_name");
		return	FUN_RUN_END;
	}

	if(!mysql_real_connect(&orlKeyDatabase, NULL, DBUSRNAME, DBUSRKEY, dbKeyName, 0, NULL, 0)) {
		printf("ORL---> otbug_relate_database_init - mysql_real_connect - keydb - failed\n");
		return	FUN_RUN_END;
	}

	if(mgc_add(orlGarCol, &keysDataBase, (gcfun)otbug_relate_database_close) == MGC_FAILED)
		otbug_relate_error("otbug_relate_database_init - mgc_add", errno);

	sprintf(tblKeyName, "K%s", otbug_tcler_date(orlShmCtler));

	return	FUN_RUN_OK;
}


/*-----otbug_relate_hbucket_init-----*/
static int otbug_relate_hbucket_init(void)
{
	int	nBucket;

	if(mc_conf_read("outbug_hash_bucket_num", CONF_NUM, &nBucket, sizeof(int)) == FUN_RUN_FAIL) {
		mc_conf_print_err("outbug_hash_bucket_num");
		return	FUN_RUN_END;
	}

	if(!(orlHashBucket = hash_bucket_init(nBucket, (mafun)mmdp_malloc, NULL,
	orlMpHandler, (hbcreat)otbug_relate_term_alloc, otbug_relate_term_cmp, otbug_relate_term_hash))) {
		otbug_relate_error("otbug_relate_hbucket_init - hash_bucket_init", errno);
		return	FUN_RUN_END;
	}

	if(mgc_add(orlGarCol, orlHashBucket, (gcfun)hash_bucket_destroy) == MGC_FAILED)
		otbug_relate_error("otbug_relate_hbucket_init - mgc_add - hashbucket", errno);

	nBucket >>= 1;
	nRelateBuck += (nBucket % 2) ? 0 : 1;

	return	FUN_RUN_OK;
}


/*-----otbug_relate_buff_init-----*/
static int otbug_relate_buff_init(void)
{
	/**************************************
	 * 1. BYTE_CMP_MAX at src/extbug/spextb.h
	 * 2. 3 means two space and one enter
	 * 3. the last (NTERMS_LEN_MAX + 1) means
	 *   one num cnt and one enter at file begin
	 **************************************/

	if(mc_conf_read("orl_rank_top_num", CONF_NUM, &nTopTerms, sizeof(int)) == MGC_FAILED) {
		mc_conf_print_err("orl_rank_top_num");
		return	FUN_RUN_END;
	}

	if(!(orlTermBuf = buff_stru_init((((BYTE_CMP_MAX + NTERMS_LEN_MAX + 3 + sizeof(int)) * 2) * nTopTerms) + 
	(NTERMS_LEN_MAX + 1)))) {
		otbug_relate_error("otbug_relate_buff_init - buff_stru_init", errno);
		return	FUN_RUN_END;
	}

	if(mgc_add(orlGarCol, orlTermBuf, (gcfun)buff_stru_free_all) == MGC_FAILED)
		otbug_relate_error("otbug_relate_buff_init - mgc_add - orlTermBuf", errno);

	return	FUN_RUN_OK;
}


/*------------------------------------------
	Part Seven: Sort operate

	1. otbug_sort_init
	2. otbug_sort_insert
	3. otbug_sort_sorting
	4. otbug_sort_destroy
	5. otbug_sort_output
	6. otbug_relate_search_linker

--------------------------------------------*/

/*-----otbug_sort_init-----*/
static int otbug_sort_init(char *rankPath)
{
	char	*pBuf, *pMov;
	int	fSize, nCir, nTotal;

	/* open ranking file and sign to garbage collector */
	sprintf(rankPath + strlen(rankPath), "rankFile");

	if((rankFileDesc = open(rankPath, O_RDWR | O_CREAT | O_TRUNC, FILE_AUTHOR)) == FUN_RET_NEG)
		return	FUN_RUN_END;

	if(mgc_add(orlGarCol, ((void *)&rankFileDesc), mgc_close) == MGC_FAILED)
		otbug_relate_error("otbug_sort_init - mgc_add - rankFileDesc", errno);

	/* start to read the cntlog */
	if(mc_conf_read("orl_cntlog_locate", CONF_STR, cntLogPath, PATH_LEN) == FUN_RUN_FAIL) {
		mc_conf_print_err("orl_cntlog_locate");
		return	FUN_RUN_END;
	}

	for(nTotal = nCir = 0; nCir < DEF_TCBUF_OFF; nCir++)
		cntLogSave[nCir] = 0;

	if((fSize = read_all_file(&pBuf, cntLogPath, 0))) {
		if(fSize >= DEF_TCBUF_OFF * (sizeof(uLong) + 1)) {
			for(pMov = pBuf, nCir = 0; pMov && nCir < DEF_TCBUF_OFF; nCir++, pMov++) {
				cntLogSave[nCir] = *(int *)pMov;
				nTotal += cntLogSave[nCir];

				if(!(pMov = strchr(pMov, '\n')))
					break;
			}
		}
	}

	free(pBuf);

	/* init term sort buffer */
	orlTsBuf.tb_max = (!nTotal ? DEF_TCBUF_SIZE : (nTotal / DEF_TCBUF_OFF));
	orlTsBuf.tb_cnt = 0;

	if(!(orlTsBuf.tb_buf = orlTsBuf.tb_cur = malloc(orlTsBuf.tb_max * sizeof(TSAVC *))))
		return	FUN_RUN_END;

	return	FUN_RUN_OK;
}


/*-----otbug_sort_insert-----*/
static int otbug_sort_insert(TSAVC *pSaver)
{
	if(orlTsBuf.tb_max == orlTsBuf.tb_cnt) {
		orlTsBuf.tb_max <<= 1;

		if(!(orlTsBuf.tb_buf = realloc(orlTsBuf.tb_buf, orlTsBuf.tb_max * sizeof(TSAVC *))))
			return	FUN_RUN_END;

		orlTsBuf.tb_cur = orlTsBuf.tb_buf + orlTsBuf.tb_cnt;
	}

	(*orlTsBuf.tb_cur) = pSaver;
	orlTsBuf.tb_cur++;
	orlTsBuf.tb_cnt++;

	return	FUN_RUN_OK;
}


/*-----otbug_sort_sorting-----*/
static void otbug_sort_sorting(void)
{
	TSAVC	*pCmp, *pHandler, **pSort;
	int	nBeg, nCir;

	for(nCir = orlTsBuf.tb_cnt >> 1; nCir > 0; nCir--) {
		pSort = orlTsBuf.tb_buf;

		for(nBeg = 0; nBeg + nCir < orlTsBuf.tb_cnt; nBeg++, pSort++) {
			pHandler = *pSort;

			if(pSort + nCir >= orlTsBuf.tb_cur)
				continue;

			pCmp = *(pSort + nCir);

			if(*(pHandler->tsl_term.te_pcnt) < *(pCmp->tsl_term.te_pcnt)) {
				*pSort = pCmp;
				*(pSort + nCir) = pHandler;
			}
		}
	}
}


/*-----otbug_sort_destroy-----*/
static void otbug_sort_destroy(void)
{
	int	nFd, nCir;
	char	cEnter = '\n';

	if(orlTsBuf.tb_buf)
		free(orlTsBuf.tb_buf);

	if((nFd = open(cntLogPath, O_RDWR)) == FUN_RUN_FAIL) {
		otbug_relate_error("otbug_sort_destroy - open", errno);
		return;
	}

	write(nFd, &orlTsBuf.tb_max, sizeof(uLong));
	write(nFd, &cEnter, sizeof(char));

	for(nCir = 0; nCir < DEF_TCBUF_OFF - 1; nCir++) {
		write(nFd, &cntLogSave[nCir], sizeof(uLong));
		write(nFd, &cEnter, sizeof(char));
	}

	close(nFd);
}


/*-----otbug_sort_output-----*/
static void otbug_sort_outbug(void)
{
	TSAVC	**pMov;
	TSPA	*pRelate;
	int	nLoop, nCir;

	if(orlTsBuf.tb_cnt) {
		buff_size_add(orlTermBuf, sprintf(buff_place_end(orlTermBuf), "%-*d\n", NTERMS_LEN_MAX, nTopTerms));
	
		nCir = (orlTsBuf.tb_cnt > nTopTerms) ? nTopTerms : orlTsBuf.tb_cnt;
		pMov = orlTsBuf.tb_buf;

		for(nLoop = 1; nCir > 0; pMov++, nCir--, nLoop++) {
			pRelate = otbug_relate_search_linker(*pMov);

			buff_size_add(orlTermBuf, ((pRelate) ? (sprintf(buff_place_end(orlTermBuf), "%d %.*s %d - %.*s %d\n", 
				nLoop, BYTE_CMP_MAX, (*pMov)->tsl_term.te_str, *((*pMov)->tsl_term.te_pcnt),
				BYTE_CMP_MAX, pRelate->tp_avc->tsl_term.te_str, *(pRelate->tp_cnt))) :

				(sprintf(buff_place_end(orlTermBuf), "%d %.*s %d", nLoop, BYTE_CMP_MAX, 
				(*pMov)->tsl_term.te_str, *((*pMov)->tsl_term.te_pcnt)))));
		}

		if(pwrite(rankFileDesc, buff_place_start(orlTermBuf), buff_now_size(orlTermBuf), 0) == FUN_RET_NEG) {
			otbug_relate_error("otbug_sort_outbug - pwrite", errno);
			otbug_tcler_dec(orlShmCtler);
			kill(getpid(), SIGUSR2);
		}

		buff_stru_make_empty(orlTermBuf);
	}
}


/*-----otbug_relate_search_linker-----*/
static TSPA *otbug_relate_search_linker(TSAVC *pCtl)
{
	TSPA	*pRet, *pFind;

	if(!(pFind = pCtl->tsl_prela))
		return	NULL;

	for(pRet = pFind, pFind = pFind->tp_next; pFind; pFind = pFind->tp_next) {
		if(*(pFind->tp_cnt) > *(pRet->tp_cnt))
			pRet = pFind;
	}

	return	pRet;
}


/*------------------------------------------
	Part Eight: Database oper

	1. otbug_relate_inc_flags
	2. otbug_relate_database_close

--------------------------------------------*/

/*-----otbug_relate_inc_flags-----*/
static void otbug_relate_inc_flags(char *pID)
{
	char	updateSql[SQL_TCOM_LEN];
	int	sqlLen;

	sqlLen = sprintf(updateSql, UPDATE_KFLAGS, tblKeyName, pID);

	if(mysql_real_query(&orlKeyDatabase, updateSql, sqlLen)) {
		if(!otbug_relate_dberr(&orlKeyDatabase, "otbug_relate_inc_flags - mysql_real_query"))
			kill(getpid(), SIGUSR2);
	}
}


/*-----otbug_relate_database_close-----*/
static void otbug_relate_database_close(void)
{
	mysql_close(&orlKeyDatabase);
	mysql_library_end();
}


/*------------------------------------------
	Part Nine: Error handling

	1. otbug_relate_error
	2. otbug_relate_dberr

--------------------------------------------*/

/*-----otbug_relate_error-----*/
static void otbug_relate_error(char *errStr, int nError)
{
	printf("ORL---> %s - %s\n", errStr, strerror(nError));
}


/*-----otbug_relate_dberr-----*/
static int otbug_relate_dberr(MYSQL *mySql, char *errStr)
{
	uInt	dbErr = mysql_errno(mySql);

	printf("ORL---> %s - mysql: %s\n", errStr, mysql_error(mySql));

	if(dbErr == CR_SERVER_LOST || dbErr == CR_SERVER_GONE_ERROR) {
		printf("ORL---> mysql dropping - reconnect now\n");

		if(mysql_real_connect(mySql, NULL, DBUSRNAME, DBUSRKEY, dbKeyName, 0, NULL, 0)) {
			printf("---> mysql_server - reconnect ok\n");
			return	FUN_RUN_OK;
		}

		printf("ORL---> reconnect failed: %s\n", mysql_error(mySql));
	}

	return	FUN_RUN_END;
}


/*------------------------------------------
	Part Ten: Relate entrance

	1. otbug_relate_entrance

--------------------------------------------*/

/*-----otbug_relate_entrance-----*/
static void otbug_relate_entrance(void)
{
	MSLRES	*pResult;
	MSLROW	uRow;
	SDT	*sdtMov;
	char	dnSql[SQL_LICOM_LEN];
	int	oldCnt, newCnt, dnLen;

	newCnt = oldCnt = 0;
	sdtMov = (SDT *)otbug_shm_data_begin(orlShmOwner);

	dnLen = sprintf(dnSql, GET_KWLIST, tblKeyName, orlDownFig);

	while(FUN_RUN_OK) {
		otbug_tcler_inc(orlShmCtler);

		if((newCnt = otbug_tcler_cnter(orlShmCtler)) > oldCnt) {
			for(; oldCnt < newCnt; oldCnt++, sdtMov = otbug_shm_data_fetch(sdtMov)) {
				if(!hash_bucket_insert(orlHashBucket, &sdtMov->sd_data, 
				sdtMov->sd_tmcon.tc_length - (TC_FIG_LEN + TC_NULL_LEN), &sdtMov->sd_tmcon.tc_count))
					kill(getpid(), SIGUSR2);
			}
		}

		/* sorting & outbug */
		otbug_sort_sorting();
		otbug_sort_outbug();

		otbug_tcler_dec(orlShmCtler);

		/* start relating */
		if(mysql_real_query(&orlKeyDatabase, dnSql, dnLen)) {
			if(!otbug_relate_dberr(&orlKeyDatabase, "otbug_relate_entrance - mysql_real_query"))
				kill(getpid(), SIGUSR2);
		}

		if(!(pResult = mysql_store_result(&orlKeyDatabase))) {
			if(mysql_errno(&keysDataBase))
				if(!otbug_relate_dberr(&orlKeyDatabase, "otbug_relate_entrance - mysql_store_result"))
					kill(getpid(), SIGUSR2);
		}

		mgc_one_add(&orlResultCol, pResult);

		while((uRow = mysql_fetch_row(pResult)))
			otbug_relate_work(uRow);

		mgc_one_clean(&orlResultCol);
	}
}


/*------------------------------------------
	Part Eleven: Relate hash

	1. otbug_relate_term_alloc
	2. otbug_relate_term_cmp
	3. otbug_relate_term_hash
	4. otbug_relate_relate_alloc
	5. otbug_relate_relate_cmp

--------------------------------------------*/

/*-----otbug_relate_term_alloc-----*/
static void *otbug_relate_term_alloc(void *pTerms, int datLen, int *pCnt)
{
	TSAVC	*pSaver;

	if(!(pSaver = mmdp_malloc(orlMpHandler, sizeof(TSAVC))))
		return	NULL;

	if(!otbug_sort_insert(pSaver)) {
		otbug_relate_error("otbug_relate_term_alloc - otbug_sort_insert", errno);
		return	NULL;
	}

	pSaver->tsl_prela = NULL;

	pSaver->tsl_term.te_str = (char *)pTerms;
	pSaver->tsl_term.te_pcnt = pCnt;
	pSaver->tsl_term.te_len = datLen;

	if(!(pSaver->tsl_hash = hash_bucket_init(nRelateBuck, (mafun)mmdp_malloc, NULL,
	orlMpHandler, (hbcreat)otbug_relate_relate_alloc, otbug_relate_relate_cmp, otbug_relate_term_hash))) {
		otbug_relate_error("otbug_relate_term_alloc - hash_bucket_init", errno);
		return	NULL;
	}

	if(mgc_add(orlGarCol, pSaver->tsl_hash, (gcfun)hash_bucket_destroy) == MGC_FAILED)
		otbug_relate_error("otbug_relate_hbucket_init - mgc_add - hashbucket", errno);

	return	pSaver;
}


/*-----otbug_relate_term_cmp-----*/
static int otbug_relate_term_cmp(void *beCmped, void *cmpEr, int cmpLen)
{
	TSAVC	*tmSaver = (TSAVC *)cmpEr;

	if(cmpLen != tmSaver->tsl_term.te_len)
		return	cmpLen - tmSaver->tsl_term.te_len;

	return	(strncmp(beCmped, tmSaver->tsl_term.te_str, cmpLen));
}


/*-----otbug_relate_term_hash-----*/
static uLong otbug_relate_term_hash(void *pData, int datLen)
{
	uChar	*hashStr = (uChar *)pData;
	uLong	nCir, nTotal;

	for(nTotal = nCir = 0; nCir < datLen; nCir++)
		nTotal += hashStr[nCir];

	return	nTotal;
}


/*-----otbug_relate_relate_alloc-----*/
static void *otbug_relate_relate_alloc(void *pTerms, int datLen, TSAVC *pTsavc)
{
	TSPA	*tsPoint;

	if(!(tsPoint = mmdp_malloc(orlMpHandler, sizeof(TSPA))))
		return	NULL;

	tsPoint->tp_avc = pTsavc;

	return	tsPoint;
}


/*-----otbug_relate_relate_cmp-----*/
static int otbug_relate_relate_cmp(void *beCmped, void *cmpEr, int cmpLen)
{
	TSPA	*tsPoint = (TSPA *)cmpEr;

	if(cmpLen != tsPoint->tp_avc->tsl_term.te_len)
		return	cmpLen - tsPoint->tp_avc->tsl_term.te_len;

	return	(strncmp(beCmped, tsPoint->tp_avc->tsl_term.te_str, cmpLen));
}


/*------------------------------------------
	Part Twelve: Relate operation

	1. otbug_relate_work
	2. otbug_relate_output

--------------------------------------------*/

/*-----otbug_relate_work-----*/
static int otbug_relate_work(MSLROW datRow)
{
	char	*pStr, *pColon;
	TSAVC	**tSaver, *tmMain, *tmRela;
	TSPA	*tsBlock[2];
	int	nCir, nTerms;

	nTerms = atoi((char *)datRow[2]);

	if(!(tSaver = mmdp_malloc(tmpMpHandler, nTerms * sizeof(TSAVC *)))) {
		otbug_relate_error("otbug_relate_work - mmdp_malloc", errno);
		return	FUN_RUN_END;
	}

	for(pStr = (char *)datRow[1], nCir = 0; nCir < nTerms; nCir++, pStr = pColon + 1) {
		if(!(pColon = strchr(pStr, ';'))) {
			nTerms = nCir;
			break;
		}

		tSaver[nCir] = hash_bucket_find(orlHashBucket, pStr, pColon - pStr);
	}

	for(nTerms--; nTerms > 0; nTerms--) {
		for(nCir = nTerms - 1; nCir >= 0; nCir--) {
			tmMain = tSaver[nTerms];
			tmRela = tSaver[nCir];

			if(!tmMain || !tmRela)
				continue;

			if(!(tsBlock[0] = hash_bucket_find(tmMain->tsl_hash, 
			tmRela->tsl_term.te_str, tmRela->tsl_term.te_len))) {
				if(!(tsBlock[0] = hash_bucket_insert(tmMain->tsl_hash, 
				tmRela->tsl_term.te_str, tmRela->tsl_term.te_len, tmRela))) {
					otbug_relate_error("otbug_relate_work - hash_bucket_insert", errno);
					kill(getpid(), SIGUSR2);
				}

				/* relating */
				tsBlock[0]->tp_next = tmMain->tsl_prela;
				tmMain->tsl_prela = tsBlock[0];

				if(!(tsBlock[1] = hash_bucket_insert(tmRela->tsl_hash, 
				tmMain->tsl_term.te_str, tmMain->tsl_term.te_len, tmMain))) {
					otbug_relate_error("otbug_relate_work - hash_bucket_insert", errno);
					kill(getpid(), SIGUSR2);
				}

				/* relating */
				tsBlock[1]->tp_next = tmRela->tsl_prela;
				tmRela->tsl_prela = tsBlock[1];

				if(!(tsBlock[0]->tp_cnt = tsBlock[1]->tp_cnt = mmdp_malloc(orlMpHandler, sizeof(int)))) {
					otbug_relate_error("otbug_relate_work - mmdp_malloc", errno);
					kill(getpid(), SIGUSR2);
				}

				(*tsBlock[0]->tp_cnt) = 0;
			}

			(*tsBlock[0]->tp_cnt)++;
		}
	}

	mmdp_reset_default(tmpMpHandler);

	otbug_relate_inc_flags((char *)datRow[0]);

	return	FUN_RUN_OK;
}


/*-----otbug_relate_output-----*/
static void otbug_relate_output(int nPrint)
{

}
