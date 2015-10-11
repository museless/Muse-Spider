#include "spinc.h"
#include "spmpool.h"
#include "spnet.h"
#include "spdb.h"
#include "sptextb.h"
#include "sptglobal.h"


/* local data */

/* extern */
extern	inline	int	txbug_tran_disc_real(void *writeFd, BUFF *pBuff);


/*------------------------------------------
	Part Zero: Disc module

	1. module_disc_init
	2. txbug_tran_disc
	3. txbug_tran_disc_force
	4. txbug_tran_disc_real
	5. txbug_print_help

--------------------------------------------*/

/*-----module_disc_init-----*/
void module_disc_init(void)
{
	char	datFile[PATH_LEN];

	if(mc_conf_read("default_texttmp_file", CONF_STR, datFile, PATH_LEN) == FUN_RUN_FAIL) {
		printf("textbug - configure file setting wrong - default_texttmp_file\n");
		txbug_sig_error();
	}

	if((txTranHand.tx_fd = open(datFile, O_RDWR | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP)) == FUN_RUN_FAIL) {
		txbug_perror("module_disc_init - open", errno);
		txbug_sig_error();
	}
}


/*-----txbug_tran_disc-----*/
void txbug_tran_disc(void *pText)
{
	TEXT	*pTxt = (TEXT *)pText;

	while(!mato_dec_and_test(&writeDataLock))
		mato_inc(&writeDataLock);

	if(!buff_size_enough(contStoreBuf, strlen(urlConfigSave[pTxt->wt_pattern].c_srcstr) +
	pTxt->wt_plsize + pTxt->wt_tlen + WEB_INDEX_LEN + WEB_TIME_LEN + strlen(pTxt->wt_url))) {
		if(txbug_tran_disc_real(&txTranHand.tx_fd, contStoreBuf) == FUN_RUN_FAIL)
			elog_write("txbug_tran_disc - writen", FUNCTION_STR, ERROR_STR);

		buff_stru_make_empty(contStoreBuf);
	}

	buff_size_add(contStoreBuf, sprintf(buff_place_end(contStoreBuf), "%.*s %s %s\n%s\n%.*s\n",
		pTxt->wt_tlen, pTxt->wt_title, urlConfigSave[pTxt->wt_pattern].c_srcstr,
		pTxt->wt_time, pTxt->wt_url, pTxt->wt_plsize, pTxt->wt_pool));

	mato_inc(&writeDataLock);
}


/*-----txbug_tran_disc_force-----*/
void txbug_tran_disc_force(void *writeFd, MATOS *atoLock, BUFF *pBuff)
{
	while(!mato_dec_and_test(atoLock))
		mato_inc(atoLock);

	if(!buff_stru_empty(pBuff)) {
		if(txbug_tran_disc_real(writeFd, pBuff) == FUN_RUN_FAIL)
			elog_write("txbug_tran_disc_force - writen", FUNCTION_STR, ERROR_STR);
			
		buff_stru_make_empty(pBuff);
	}

	mato_inc(atoLock);
}


/*-----txbug_tran_disc_real-----*/
inline int txbug_tran_disc_real(void *writeFd, BUFF *pBuff)
{
	return	(buff_check_exist(pBuff)) ? 
		writen(*(int *)writeFd, buff_place_start(pBuff), buff_now_size(pBuff)) : FUN_RUN_FAIL;	
}


/*-----txbug_print_help-----*/
void txbug_print_help(void)
{
	char	*hpCont;
	char	hlPath[PATH_LEN];

	if(mc_conf_read("textbug_help_document", CONF_STR, hlPath, PATH_LEN) == FUN_RUN_FAIL) {
		mc_conf_print_err("textbug_help_document");
		exit(FUN_RUN_FAIL);
	}

	if(read_all_file(&hpCont, hlPath, 0) == FUN_RUN_FAIL) {
		txbug_perror("txbug_print_help - read_all_file", errno);
		exit(FUN_RUN_FAIL);
	}

	printf("%s\n\n", hpCont);
	
	free(hpCont);
}
