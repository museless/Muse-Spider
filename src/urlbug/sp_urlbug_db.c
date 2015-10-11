/*------------------------------------------
	Source file content Five part

	Part Zero:	Include
	Part One:	Local data
	Part Two:	Local function
	Part Three:	Define

	Part Four:	Mysql operate

--------------------------------------------*/


/*------------------------------------------
	Part Zero: Include
--------------------------------------------*/

#include "spinc.h"
#include "spdb.h"
#include "spnet.h"
#include "spmsg.h"
#include "spmpool.h"

#include "spuglobal.h"
#include "spurlb.h"


/*------------------------------------------
        Part Four: Mysql operate

        1. ubug_init_database
        2. ubug_create_dbtable
        3. ubug_tran_db
        4. ubug_tran_db_whole
        5. ubug_tran_db_force
        6. ubug_tran_db_real
        7. ubug_dberr

--------------------------------------------*/

/*-----ubug_init_database-----*/
void ubug_init_database(void)
{
	char	sqlName[SQL_DBNAME_LEN];

	if(mc_conf_read("urls_database_name", CONF_STR, sqlName, SQL_DBNAME_LEN) == FUN_RUN_FAIL) {
		elog_write("urlbug", "configure file setting wrong",  "urls_database_name");
		ubug_sig_error();
	}

	mysql_library_init(nrOpt, sqlOpt, NULL);

	if(!mysql_init(&urlDataBase)) {
		elog_write("ubug_init_database - mysql_init", "urlDataBase", "Failed");
		ubug_sig_error();
	}

	if(!mysql_real_connect(&urlDataBase, NULL, DBUSRNAME, DBUSRKEY, sqlName, 0, NULL, 0)) {
		elog_write("ubug_init_database - mysql_real_connect", "urlDataBase", "Failed");
		ubug_sig_error();
	}

	if(mgc_add(urlGarCol, NULL_POINT, ubug_db_clean) == MGC_FAILED)
		ubug_perror("ubug_init_database - mgc_add", errno);

	ubug_create_dbtable();
}


/*-----ubug_create_dbtable-----*/
void ubug_create_dbtable(void)
{
	char	creatSql[SQL_SCOM_LEN];

	sprintf(creatSql, CREAT_URL_TAB, urlTabName, urlMaxLen);

	if(mysql_query(&urlDataBase, creatSql) != FUN_RUN_END) {
		if(ubug_dberr(&urlDataBase, "ubug_init_database - mysql_query - creatTab") != FUN_RUN_OK)
			ubug_sig_error();
	}
}


/*-----ubug_tran_db-----*/
void ubug_tran_db(void *pInfo, char *pUrl, int uLen)
{
        WEBIN   *webInfo = (WEBIN *)pInfo;
        
	if(!buff_size_enough(webInfo->w_buff, SQL_PERCOM_MLEN))
		ubug_tran_db_force(webInfo->w_buff);

	buff_size_add(webInfo->w_buff, ((buff_stru_empty(webInfo->w_buff)) ? 
	sprintf(buff_place_start(webInfo->w_buff), TRAN_URL_BEG, urlTabName, uLen, pUrl, webInfo->w_pattern) : 
	sprintf(buff_place_end(webInfo->w_buff), TRAN_URL, uLen, pUrl, webInfo->w_pattern)));
}


/*-----ubug_tran_db_whole-----*/
void ubug_tran_db_whole(void)
{
        WEBIN   *pInfo;
        
        for (pInfo = urlSaveList; pInfo; pInfo = pInfo->w_next)
                ubug_tran_db_force(pInfo->w_buff);
}


/*-----ubug_tran_db_force-----*/
void ubug_tran_db_force(BUFF *pBuff)
{
	while(!mato_dec_and_test(&writeStoreLock))
		mato_inc(&writeStoreLock);

	if(!buff_stru_empty(pBuff)) {
		if(ubug_tran_db_real(pBuff) != FUN_RUN_END) {
			if(ubug_dberr(&urlDataBase, "ubug_tran_db_force - mysql_query") != FUN_RUN_OK)
				ubug_sig_error();
		}

		buff_stru_make_empty(pBuff);
	}

	mato_inc(&writeStoreLock);
}


/*-----ubug_tran_db_real-----*/
int ubug_tran_db_real(BUFF *pBuff)
{
	return	(!buff_stru_empty(pBuff)) ? 
		mysql_real_query(&urlDataBase, buff_place_start(pBuff), buff_now_size(pBuff)) : FUN_RUN_FAIL;
}


/*-----ubug_dberr-----*/
int ubug_dberr(MYSQL *sHandler, char *withStr)
{
	uInt	myErrno = mysql_errno(sHandler);

	elog_write(withStr, FUNCTION_STR, MYERR_STR(sHandler));

	if(myErrno == CR_SERVER_LOST || myErrno == CR_SERVER_GONE_ERROR) {
		if(mysql_real_connect(&urlDataBase, NULL, DBUSRNAME, DBUSRKEY, urlTabName, 0, NULL, 0))
			return	FUN_RUN_OK;
	}

	return	FUN_RUN_FAIL;
}
