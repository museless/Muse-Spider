#include "spinc.h"
#include "spdb.h"
/*------------------------------------------
	Source file content Five part

	Part Zero:	Include
	Part One:	Define
	Part Two:	Local data
	Part Three:	Local function

	Part Four:	Http state handling
	Part Five:	Data hand

--------------------------------------------*/

/*------------------------------------------
	Part Zero: Include
--------------------------------------------*/

#include "spinc.h"
#include "spdb.h"
#include "spnet.h"
#include "sphtml.h"

#include "spmpool.h"
#include "sptglobal.h"
#include "sptextb.h"


/*------------------------------------------
	Part Four: Http state handling

	1. txbug_dispose_http301

--------------------------------------------*/

/*-----txbug_dispose_http301-----*/
int txbug_dispose_http301(char *retBuf, TEXT *pText)
{
	char	*pEnter;

	if(!(retBuf = strstr(retBuf, MATCH_LOCA)))
		return	FUN_RUN_END;

	if(!(pEnter = strstr(retBuf += MLOCA_LEN, MATCH_LINKBRK)))
		return	FUN_RUN_END;

	if(!buff_size_enough(urlSaveBuf, SQL_PERCOM_MLEN)) {
		if(txbug_tran_db_news(&urlDataBase, urlSaveBuf)) {
			if(txbug_dberr_dispose(&urlDataBase, urlsDbName, 
			"txbug_dispose_http301 - txbug_tran_db_news") < FUN_RUN_OK)
				txbug_sig_error();
		}

		buff_stru_make_empty(urlSaveBuf);
	}

	buff_size_add(urlSaveBuf, ((buff_stru_empty(urlSaveBuf)) ? 
	sprintf(buff_place_end(urlSaveBuf), TRAN_URL_BEG, urlsTblName, (int)(pEnter - retBuf), retBuf, pText->wt_pattern) : 
	sprintf(buff_place_end(urlSaveBuf), TRAN_URL, (int)(pEnter - retBuf), retBuf, pText->wt_pattern)));

	return	FUN_RUN_OK;
}
