/*------------------------------------------
	Otbug keyword relate

	Source file content Five part

	Part Zero:	Include
	Part One:	Define
	Part Two:	Local data
	Part Three:	Local function

	Part Four:	Main
	Part Five:	Signal operate
	Part Six:	Plugins init
	Part Seven:	Database oper
	Part Eight:	Error handling

	Part Nine:	Relate entrance
	Part Ten:	Relate hash
	Part Eleven: 	Relate operation

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

#define	otbug_relate_align(pSaver, nMov) \
((TSAVC *)((char *)pSaver + nMov * sizeof(TSAVC *)))

#define	otbug_relate_align(pSaver, nMov) \
((TSAVC *)((char *)pSaver + nMov * sizeof(TSAVC *)))

/* typedef */
typedef	struct	termsav	TESAV;
typedef	struct	tsavctl	TSAVC;
typedef	struct	tsavpac	TSPA;

/* struct */
struct	termsav {
	char	*te_str;
	int	*te_pcnt;

	int	te_len;
};

struct	tsavctl {
	HBET	*tsl_hash;
	
	TSAVC	*tsl_next;
	TSPA	*tsl_prela;

	TESAV	tsl_term;
};

struct	tsavpac {
	TSAVC	*tp_avc;
	TSPA	*tp_next;

	int	*tp_cnt;
};


/*------------------------------------------
	Part Two: Local data
--------------------------------------------*/

static	MGCH	*orlGarCol;
static	MGCO	orlResultCol;

static	MSHM	*orlShmOwner;
static	DMPH	*orlMpHandler, *tmpMpHandler;
static	TLCTL	*orlShmCtler;
static	HBET	*orlHashBucket;
static	TSAVC	*orlTermList;

static	MYSQL	orlKeyDatabase;

static	int	orlDownFig, nRelateBuck;

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

/* Part Seven */
static	void	otbug_relate_inc_flags(char *pID);
static	void	otbug_relate_database_close(void);

/* Part Eight */
static	void	otbug_relate_error(char *errStr, int nError);
static	int	otbug_relate_dberr(MYSQL *mySql, char *errStr);

/* Part Nine */
static	void	otbug_relate_entrance(void);

/* Part Ten */
static	void	*otbug_relate_term_alloc(void *pTerms, int datLen, int *pCnt);
static	int	otbug_relate_term_cmp(void *beCmped, void *cmpEr, int cmpLen);
static	uLong	otbug_relate_term_hash(void *pData, int datLen);

static	void	*otbug_relate_relate_alloc(void *pTerms, int datLen, TSAVC *pTsavc);
static	int	otbug_relate_relate_cmp(void *beCmped, void *cmpEr, int cmpLen);

/* Part Eleven */
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
		if(otbug_relate_mpool_init() && otbug_relate_hbucket_init() && otbug_relate_database_init())
			otbug_relate_entrance();
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
	Part Six: Plugins init

	1. mainly_init
	2. otbug_relate_mpool_init
	3. otbug_relate_database_init
	4. otbug_relate_hbucket_init

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

	orlTermList = NULL;

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


/*------------------------------------------
	Part Seven: Database oper

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
	Part Eight: Error handling

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
	Part Nine: Relate entrance

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
		if((newCnt = otbug_tcler_cnter(orlShmCtler)) > oldCnt) {
			for(; oldCnt < newCnt; oldCnt++, sdtMov = otbug_shm_data_fetch(sdtMov)) {
				if(!hash_bucket_insert(orlHashBucket, &sdtMov->sd_data, 
				sdtMov->sd_tmcon.tc_length - (TC_FIG_LEN + TC_NULL_LEN), &sdtMov->sd_tmcon.tc_count))
					kill(getpid(), SIGUSR2);
			}
		}

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
	Part Ten: Relate hash

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

	pSaver->tsl_next = orlTermList;
	orlTermList = pSaver;

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
	Part Eleven: Relate operation

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
	TSPA	*pList;
	int	nCir;

	for(nCir = 0; nCir < nPrint; orlTermList = orlTermList->tsl_next) {
		if(*orlTermList->tsl_term.te_pcnt > 10) {
			printf("Term: %s - %d\n", orlTermList->tsl_term.te_str, *orlTermList->tsl_term.te_pcnt);
	
			for(pList = orlTermList->tsl_prela; pList; pList = pList->tp_next)
				printf("  Rela: %s - %d\n", pList->tp_avc->tsl_term.te_str, *pList->tp_cnt);
	
			printf("\n\n");
			nCir++;
		}
	}
}
