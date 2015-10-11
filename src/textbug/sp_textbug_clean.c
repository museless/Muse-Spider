#include "spinc.h"
#include "spmpool.h"
#include "spnet.h"
#include "spdb.h"
#include "sptextb.h"
#include "sptglobal.h"


/*------------------------------------------
	Part Three: Cleaning operate

	1. txbug_pool_free
	2. txbug_db_unlink_url
	3. txbug_db_unlink_cont
	4. txbug_db_close

--------------------------------------------*/

/*-----txbug_pool_free-----*/
void txbug_pool_free(TEXT *textPoint)
{
	wmpool_free(contMemPool, textPoint->wt_pool);
	wmpool_free(tsMemPool, textPoint);
}


/*-----txbug_db_unlink_url-----*/
void txbug_db_unlink_url(void)
{
	mysql_close(&urlDataBase);
}


/*-----txbug_db_unlink_cont-----*/
void txbug_db_unlink_cont(void)
{
	mysql_close(&txTranHand.tx_sql);
}


/*-----txbug_db_close-----*/
void txbug_db_close(void)
{
	mysql_library_end();
}

