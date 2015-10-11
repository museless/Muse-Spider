#include "spinc.h"
#include "spdb.h"

/*------------------------------------------
	Part Zero: Define function

	1. 

--------------------------------------------*/


/*------------------------------------------
	Part One: Mysql operate

	1. mysql_string_exist_check

--------------------------------------------*/

/*-----mysql_string_exist_check-----*/
inline int mysql_string_exist_check(void *chkSql, char *chkCom)
{
	MYSQL_RES	*sqlRes;
	int		fRet;
		
	if(mysql_query((MYSQL *)chkSql, chkCom) != FUN_RUN_END)
		return	FUN_RUN_FAIL;

	if((sqlRes = mysql_store_result((MYSQL *)chkSql)) == NULL)
		return	FUN_RUN_FAIL;

	fRet = mysql_num_rows(sqlRes);

	mysql_free_result(sqlRes);

	return	(fRet) ? 1 : 0;
}
