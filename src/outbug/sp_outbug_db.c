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
	Source file content Six part

	Part Zero:	Include
	Part One:	Define
	Part Two:	Local data
	Part Three:	Local function

	Part Four:	Database operate
	Part Five:	Database destroy

--------------------------------------------*/

/*------------------------------------------
	Part Zero: Include
--------------------------------------------*/

#include "spinc.h"
#include "spmpool.h"
#include "spnet.h"
#include "spdb.h"
#include "spoutb.h"
#include "spoglobal.h"


/*------------------------------------------
	Part Two: Local data
--------------------------------------------*/

static	MATOS	dbCtlLock;


/*------------------------------------------
	Part Four: Database operate

	1. otbug_database_init
	2. otbug_dberr_dispose

--------------------------------------------*/

/*-----otbug_database_init-----*/
void otbug_database_init(void)
{
	mysql_library_init(nrOpt, sqlOpt, NULL);

	if(!mysql_init(&keysDataBase)) {
		printf("Outbug---> otbug_database_init - mysql_init - failed\n");
		otbug_sig_error();
	}

	if(mc_conf_read("keys_database_name", CONF_STR, keysDbName, SQL_DBNAME_LEN) == FUN_RUN_FAIL) {
		mc_conf_print_err("urls_database_name");
		otbug_sig_error();
	}

	if(!mysql_real_connect(&keysDataBase, NULL, DBUSRNAME, DBUSRKEY, keysDbName, 0, NULL, 0)) {
		printf("Outbug---> otbug_database_init - mysql_real_connect - keydb - failed\n");
		otbug_sig_error();
	}

	if(mgc_add(obGarCol, &keysDataBase, otbug_unlink_database) == MGC_FAILED)
		elog_write("otbug_database_init - mgc_add", FUNCTION_STR, ERROR_STR);

	mato_init(&dbCtlLock, 1);
}


/*-----otbug_dberr_dispose-----*/
int otbug_dberr_dispose(void *sqlHand, char *dbName, char *pHead)
{
	MYSQL		*pSql = (MYSQL *)sqlHand;
	unsigned int	myErrno = mysql_errno(pSql);
	char		meBuf[32];

	sprintf(meBuf, "%d", mysql_errno(pSql));

	elog_write(pHead, meBuf, (char *)mysql_error(pSql));

	if(myErrno == CR_SERVER_LOST || myErrno == CR_SERVER_GONE_ERROR) {
		printf("Outbug---> mysql dropping - reconnect now\n");

		if(mysql_real_connect(pSql, NULL, DBUSRNAME, DBUSRKEY, dbName, 0, NULL, 0)) {
			printf("---> mysql_server - reconnect ok\n");
			return	FUN_RUN_OK;
		}

		printf("Outbug---> reconnect failed: %s\n", mysql_error(pSql));

	} else if(myErrno == ER_NO_SUCH_TABLE) {
		return	FUN_RUN_END;
	}

	return	FUN_RUN_FAIL;
}


/*-----otbug_rewind_state-----*/
void otbug_rewind_state(char *wStr, char *pID)
{
	char	updateSql[SQL_TCOM_LEN];
	int	sqlLen;

	while(!mato_dec_and_test(&dbCtlLock))
		mato_inc(&dbCtlLock);

	sqlLen = sprintf(updateSql, wStr, keysTblName, pID);

	if(mysql_real_query(&keysDataBase, updateSql, sqlLen) != FUN_RUN_END) {
		if(otbug_dberr_dispose(&keysDataBase, keysDbName,
		"otbug_rewind_state - mysql_query") < FUN_RUN_OK)
			otbug_sig_error();
	}

	mato_inc(&dbCtlLock);
}


/*------------------------------------------
	Part Four: Database destroy

	1. otbug_unlink_database
	2. otbug_unlink_database_serv

--------------------------------------------*/

/*-----otbug_unlink_database-----*/
void otbug_unlink_database(void *sqlHandler)
{
	mysql_close(sqlHandler);
}


/*-----otbug_unlink_database_serv-----*/
void otbug_unlink_database_serv(void)
{
	mysql_library_end();
}
