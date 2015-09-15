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
	3. ubug_url_review
	4. ubug_tran_db
	5. ubug_tran_db_force
	6. ubug_tran_db_real
	7. ubug_dberr_dispose

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
		perror("Urlbug---> ubug_init_database - mgc_add");

	ubug_create_dbtable();
}


/*-----ubug_create_dbtable-----*/
void ubug_create_dbtable(void)
{
	char	creatSql[SQL_SCOM_LEN];

	sprintf(creatSql, CREAT_URL_TAB, urlTabName);

	if(mysql_query(&urlDataBase, creatSql) != FUN_RUN_END) {
		if(ubug_dberr_dispose(&urlDataBase, "ubug_init_database - mysql_query - creatTab") != FUN_RUN_OK)
			ubug_sig_error();
	}
}


/*-----ubug_url_review-----*/
int ubug_url_review(char *pUrl, int uLen)
{
	char	seleCom[SQL_SCOM_LEN];
	int	funRet;

	while(!mato_dec_and_test(&writeStoreLock))
		mato_inc(&writeStoreLock);

	sprintf(seleCom, REVIEW_URL, urlTabName, uLen, pUrl);

	if((funRet = mysql_string_exist_check(&urlDataBase, seleCom)) == FUN_RUN_FAIL) {
		if(ubug_dberr_dispose(&urlDataBase, "ubug_url_review - mysql_check") != FUN_RUN_OK)
			ubug_sig_error();
	}
		
	if(funRet == 0) {
		strncpy(seleCom, pUrl, uLen);
		seleCom[uLen] = 0;
		
		if(buff_stru_strstr(urlBufStu, seleCom))
			funRet = 1;
	}

	mato_inc(&writeStoreLock);

	return	(funRet) ? FUN_RUN_OK : FUN_RUN_END;
}


/*-----ubug_tran_db-----*/
void ubug_tran_db(char *pUrl, int uLen, int pattWay)
{
	while(!mato_dec_and_test(&writeStoreLock))
		mato_inc(&writeStoreLock);

	if(!buff_size_enough(urlBufStu, SQL_PERCOM_MLEN)) {
		if(ubug_tran_db_real() != FUN_RUN_END) {
			if(ubug_dberr_dispose(&urlDataBase, "ubug_tran_db - mysql_query") != FUN_RUN_OK)
				ubug_sig_error();
		}

		buff_stru_make_empty(urlBufStu);
	}

	buff_size_add(urlBufStu, ((buff_stru_empty(urlBufStu)) ? 
	sprintf(buff_place_start(urlBufStu), TRAN_URL_BEG, urlTabName, uLen, pUrl, pattWay) : 
	sprintf(buff_place_end(urlBufStu), TRAN_URL, uLen, pUrl, pattWay)));

	mato_inc(&writeStoreLock);
}


/*-----ubug_tran_db_force-----*/
void ubug_tran_db_force(void)
{
	while(!mato_dec_and_test(&writeStoreLock))
		mato_inc(&writeStoreLock);

	if(!buff_stru_empty(urlBufStu)) {
		if(ubug_tran_db_real() != FUN_RUN_END) {
			if(ubug_dberr_dispose(&urlDataBase, "ubug_tran_db_force - mysql_query") != FUN_RUN_OK)
				ubug_sig_error();
		}

		buff_stru_make_empty(urlBufStu);
	}

	mato_inc(&writeStoreLock);
}


/*-----ubug_tran_db_real-----*/
int ubug_tran_db_real(void)
{
	return	(!buff_stru_empty(urlBufStu)) ? 
		mysql_query(&urlDataBase, buff_place_start(urlBufStu)) : FUN_RUN_FAIL;
}


/*-----ubug_dberr_dispose-----*/
int ubug_dberr_dispose(MYSQL *sHandler, char *withStr)
{
	uInt	myErrno = mysql_errno(sHandler);

	elog_write(withStr, FUNCTION_STR, MYERR_STR(sHandler));

	if(myErrno == CR_SERVER_LOST || myErrno == CR_SERVER_GONE_ERROR) {
		if(mysql_real_connect(&urlDataBase, NULL, DBUSRNAME, DBUSRKEY, urlTabName, 0, NULL, 0))
			return	FUN_RUN_OK;
	}

	return	FUN_RUN_FAIL;
}
