#include "spinc.h"
#include "spmpool.h"
#include "spnet.h"
#include "spdb.h"
#include "sptextb.h"
#include "sptglobal.h"


/* define */
#define	CREAT_VIEW_ERRT	0x3

/* extern */
extern	inline	int	txbug_tran_db_news(void *dbHandler, BUFF *pBuff);


/*------------------------------------------
	Part Three: Database operate

	1. txbug_database_init
	2. txbug_db_rewind_state
	3. txbug_dberr_dispose

--------------------------------------------*/

/*-----txbug_database_init-----*/
void txbug_database_init(void)
{
	mysql_library_init(nrOpt, sqlOpt, NULL);

	if(!mysql_init(&urlDataBase)) {
		printf("txbug_database_init - mysql_init - failed\n");
		txbug_sig_error();
	}

	if(mc_conf_read("urls_database_name", CONF_STR, urlsDbName, SQL_DBNAME_LEN) == FUN_RUN_FAIL) {
		mc_conf_print_err("urls_database_name");
		txbug_sig_error();
	}

	if(!mysql_real_connect(&urlDataBase, NULL, DBUSRNAME, DBUSRKEY, urlsDbName, 0, NULL, 0)) {
		printf("txbug_database_init - mysql_real_connect - urlDataBase - failed\n");
		txbug_sig_error();
	}

	if(mgc_add(textGarCol, NULL_POINT, (gcfun)txbug_db_unlink_url) == MGC_FAILED)
		elog_write("txbug_database_init - mgc_add", FUNCTION_STR, ERROR_STR);
}


/*-----txbug_db_rewind_state-----*/
void txbug_db_rewind_state(char *wStr, char *pID)
{
	char	updateSql[SQL_TCOM_LEN];

	while(!mato_dec_and_test(&writeDataLock))
		mato_inc(&writeDataLock);

	sprintf(updateSql, wStr, urlsTblName, pID);

	if(mysql_query(&urlDataBase, updateSql) != FUN_RUN_END) {
		if(txbug_dberr_dispose(&urlDataBase, urlsDbName, 
		"txbug_db_rewind_state - mysql_query") < FUN_RUN_OK)
			txbug_sig_error();
	}

	mato_inc(&writeDataLock);
}


/*-----txbug_dberr_dispose-----*/
int txbug_dberr_dispose(void *sqlHand, char *dbName, char *pHead)
{
	MYSQL		*pSql = (MYSQL *)sqlHand;
	unsigned int	myErrno = mysql_errno(pSql);
	char		meBuf[32];

	sprintf(meBuf, "%d", mysql_errno(pSql));

	elog_write(pHead, meBuf, (char *)mysql_error(pSql));

	if(myErrno == CR_SERVER_LOST || myErrno == CR_SERVER_GONE_ERROR) {
		printf("Txbug---> mysql dropping - reconnect now\n");

		if(mysql_real_connect(pSql, NULL, DBUSRNAME, DBUSRKEY, dbName, 0, NULL, 0)) {
			printf("Txbug---> mysql_server - reconnect ok\n");
			return	FUN_RUN_OK;
		}

		printf("Txbug---> reconnect failed: %s\n", mysql_error(pSql));

	} else if(myErrno == ER_NO_SUCH_TABLE) {
		return	FUN_RUN_END;
	}

	return	FUN_RUN_FAIL;
}


/*------------------------------------------
	Part Zero: Database module

	1. module_database_init
	2. txbug_tran_db
	3. txbug_tran_db_force
	4. txbug_create_news_table
	5. txbug_create_view
	6. txbug_tran_db_news
	7. txbug_create_nv

--------------------------------------------*/

/*-----module_database_init-----*/
void module_database_init(void)
{
	if(!mysql_init(&txTranHand.tx_sql)) {
		printf("module_database_init - mysql_init - txTranHand.tx_sql - failed\n");
		exit(FUN_RUN_FAIL);
	}

	if(mc_conf_read("news_database_name", CONF_STR, newsDbName, SQL_DBNAME_LEN) == FUN_RUN_FAIL) {
		printf("textbug - configure file setting wrong - news_database_name\n");
		exit(FUN_RUN_FAIL);
	}

	if(!mysql_real_connect(&txTranHand.tx_sql, NULL, DBUSRNAME, DBUSRKEY, newsDbName, 0, NULL, 0)) {
		printf("module_database_init - mysql_real_connect - txTranHand.tx_sql - failed\n");
		exit(FUN_RUN_FAIL);
	}

	if(mgc_add(textGarCol, NULL_POINT, (gcfun)txbug_db_unlink_cont) == MGC_FAILED)
		elog_write("module_database_init - mgc_add", FUNCTION_STR, ERROR_STR);

	txbug_create_nv();
}


/*-----txbug_tran_db-----*/
void txbug_tran_db(void *txtStru)
{
	TEXT	*pTxt = (TEXT *)txtStru;

	while(!mato_dec_and_test(&writeDataLock))
		mato_inc(&writeDataLock);

	/* news buff */
	if(!buff_size_enough(contStoreBuf, strlen(urlConfigSave[pTxt->wt_pattern].c_srcstr) + 
	pTxt->wt_tlen + WEB_COUNT_LEN + strlen(pTxt->wt_url) + pTxt->wt_plsize)) {
		if(txbug_tran_db_news(&txTranHand.tx_sql, contStoreBuf)) {
			if(txbug_dberr_dispose(&txTranHand.tx_sql, newsDbName, 
			"txbug_tran_db - txbug_tran_db_news") < FUN_RUN_OK)
				txbug_sig_error();

			if(!txbug_tran_db_news(&txTranHand.tx_sql, contStoreBuf))
				txbug_sig_error();
		}

		buff_stru_make_empty(contStoreBuf);
	}

	buff_size_add(contStoreBuf, buff_stru_empty(contStoreBuf) ?
	sprintf(buff_place_end(contStoreBuf), TRAN_NEWS_BEG, newsTblName, pTxt->wt_id, 
		&urlsTblName[1], pTxt->wt_time, urlConfigSave[pTxt->wt_pattern].c_srcstr, pTxt->wt_url) :
	sprintf(buff_place_end(contStoreBuf), TRAN_NEWS, pTxt->wt_id, &urlsTblName[1],
		pTxt->wt_time, urlConfigSave[pTxt->wt_pattern].c_srcstr, pTxt->wt_url));

	buff_size_add(contStoreBuf, mysql_real_escape_string(&txTranHand.tx_sql, buff_place_end(contStoreBuf),
		pTxt->wt_title, pTxt->wt_tlen));

	buff_size_add(contStoreBuf, sprintf(buff_place_end(contStoreBuf), "\", \""));

	buff_size_add(contStoreBuf, mysql_real_escape_string(&txTranHand.tx_sql, buff_place_end(contStoreBuf),
		pTxt->wt_pool, pTxt->wt_plsize));

	buff_size_add(contStoreBuf, sprintf(buff_place_end(contStoreBuf), "\")"));

	mato_inc(&writeDataLock);
}


/*-----txbug_tran_db_force-----*/
void txbug_tran_db_force(void *dbHandler, MATOS *atoLock, BUFF *bufStru)
{
	while(!mato_dec_and_test(atoLock))
		mato_inc(atoLock);

	if(!buff_stru_empty(bufStru)) {
		if(!txbug_tran_db_news(dbHandler, bufStru))
			elog_write("txbug_db_force - txbug_tran_db_news", FUNCTION_STR, (char *)mysql_error(dbHandler));

		buff_stru_make_empty(bufStru);
	}

	mato_inc(atoLock);
}


/*-----txbug_tran_db_news-----*/
inline int txbug_tran_db_news(void *dbHandler, BUFF *pBuff)
{
	return	(buff_check_exist(pBuff)) ? 
		mysql_real_query(dbHandler, buff_place_start(pBuff), buff_now_size(pBuff)) : FUN_RUN_FAIL;
}


/*-----txbug_create_news_table-----*/
void txbug_create_news_table(void)
{
	char	sqlCom[SQL_GCOM_LEN];

	/* create news table */
	sprintf(sqlCom, CREAT_NEWS_TAB, newsTblName);

	if(mysql_query(&txTranHand.tx_sql, sqlCom) != FUN_RUN_END) {
		txbug_dberr_dispose(&txTranHand.tx_sql, newsDbName, 
		"txbug_create_news_table - mysql_query - txTranHand.tx_sql");
		txbug_sig_error();
	}
}


/*-----txbug_create_view-----*/
void txbug_create_view(void)
{
	char	sqlCom[SQL_BCOM_LEN];
	char	uTab[SQL_TCOM_LEN], nTab[SQL_TCOM_LEN];
	int	nCir;

	sprintf(uTab, "%s.%s", urlsDbName, urlsTblName);
	sprintf(nTab, "%s.%s", newsDbName, newsTblName);

	printf("%s - %s\n", uTab, nTab);

	sprintf(sqlCom, CR_NEW_VIEW, &urlsTblName[1], uTab, nTab, uTab, uTab, uTab, uTab, uTab, uTab);

	for(nCir = 0; nCir < CREAT_VIEW_ERRT; nCir++) {
		if(mysql_query(&txTranHand.tx_sql, sqlCom) == FUN_RUN_END)
			break;

		if(nCir == CREAT_VIEW_ERRT) {
			txbug_dberr_dispose(&txTranHand.tx_sql, newsDbName, "txbug_create_view - mysql_query");
			txbug_sig_error();
		}

		sleep(TAKE_A_EYECLOSE);
	}
}


/*-----txbug_create_nv-----*/
void txbug_create_nv(void)
{
	txbug_create_news_table();
	txbug_create_view();
}
