/*------------------------------------------
	Source file content Twelve part

	Part Zero:	Include
	Part One:	Local data
	Part Two:	Local function
	Part Three:	Define

	Part Four:	Textbug main
	Part Five:	Initialization
	Part Six:	Network
	Part Seven:	Entrance
	Part Eight:	Running setting
	Part Nine:	Textbug mainly work
	Part Ten:	Message part
	Part Eleven:	Charset

--------------------------------------------*/

/*------------------------------------------
	Part Zero: Include
--------------------------------------------*/

#include "spinc.h"
#include "spnet.h"
#include "spdb.h"
#include "spmsg.h"
#include "spmpool.h"
#include "sphtml.h"
#include "spsock.h"
#include "spframe.h"

#include "mmdpool.h"
#include "mpctl.h"
#include "mipc.h"

#include "sptextb.h"
#include "sptglobal.h"


/*------------------------------------------
	Part One: Local data
--------------------------------------------*/

static	char	selectSqlComm[SQL_LICOM_LEN];


/*------------------------------------------
	Part Two: Local function
--------------------------------------------*/

/* Part Four */
static	void	txbug_command_analyst(int nPara, char **pComm);

/* Part Five */
static	int	mainly_init(void);
static	int	txbug_init(void);
static	int	txbug_mempool_init(void);
static	void	txbug_timbuf_init(char *pTime);
static	void	txbug_conf_init(void);
static	void	txbug_databuf_init(void);
static	void	txbug_searchlist_init(SSTRL **pList, char *searchStr);

/* Part Six */
static	int	txbug_web_download(WEB *wPoint, TEXT *pTex);
static	int	txbug_web_initialize(WEB *webStru, TEXT *wtStru);
static	int	txbug_url_abstract(char *pUrl, WEB *webStru);
static	int	txbug_httpreq_send(WEB *wStru, int nSock, TEXT *tStru);
static	int	txbug_handling_httpreq(int httpState, char *retBuf, TEXT *pText);

/* Part Seven */
static	void	txbug_main(void);
static	void	txbug_pthread_generator(TEXT *textPoint);
static	MSLRES 	*txbug_sql_download(void);

/* Part Eight */
static	TEXT	*txbug_handler_alloc(void);
static	void	txbug_txbug_charset_set(TEXT *charText);

/* Part Nine */
static	void	txbug_job(void *aParameter);
static	int	txbug_get_title(TEXT *pText, iconv_t dCv);
static	int	txbug_find_pubtime(TEXT *pText);
static	int	txbug_get_context(TEXT *pText, iconv_t dCv);
static	char	*txbug_get_title_end(char *pBeg, int tLimit);
static	size_t	txbug_tran_chars(TEXT *pText, iconv_t cCv, char *inBuf, size_t iSize, char *outBuf, size_t oSize);
static	int	txbug_norway_get_context(TEXT *pText, char *searchBeg, int nLimit, char *toBuf, int toLimit);
static	char	*txbug_match_string(char *pFind, SSTRL *begList, int nLimit);
static	int	txbug_charset_cmpmode_set(TEXT *pText, iconv_t *pCv);
static	void	txbug_create_pubtime(TEXT *newText);

static	inline	int	txbug_check_contxbug_end(char *contEnd);

/* Part Ten */

/* Part Eleven */
static	int	txbug_accept_word(TEXT *wText, unsigned char *pChar);

static	inline	int	txbug_is_utf8(unsigned char *wordBeg);
static	inline	int	txbug_is_gb2312(unsigned char *wordBeg);
static	inline	int	txbug_is_gbk(unsigned char *wordBeg);


/*------------------------------------------
	Part Three: Define
--------------------------------------------*/

#define	txbug_run_set(initFun, wrFun, wrFunforce, wrFunreal, ctimeFun, txHand) { \
	textbugRunSet.ts_init = initFun; \
	textbugRunSet.ts_wt = wrFun; \
	textbugRunSet.ts_wtf = wrFunforce; \
	textbugRunSet.ts_wtr = wrFunreal; \
	textbugRunSet.ts_fctim = ctimeFun; \
	textbugRunSet.ts_hand = txHand; \
}

#define	txbug_pass_space(pStr) { \
	for(; isspace(*pStr); pStr++) \
		;	/* nothing */   \
}

#define txbug_data_sync(nSync) { \
	if(++nSync > cSyncTime) { \
		nSync = 0; \
		textbugRunSet.ts_wtf(&txTranHand.tx_sql, &writeDataLock, contStoreBuf); \
	} \
}


/*------------------------------------------
	Part Four: Textbug main

	1. main
	2. txbug_command_analyst

--------------------------------------------*/

/*-----main-----*/
int main(int argc, char *argv[])
{
	txbug_command_analyst(argc, argv);

	if(mainly_init()) {
		if(txbug_mempool_init() == FUN_RUN_OK) {
			txbug_databuf_init();
			txbug_database_init();
			txbug_conf_init();

			textbugRunSet.ts_init();
		
			mgc_add(textGarCol, NULL_POINT, (gcfun)txbug_db_close);
		
			txbug_main();
		}
	}

	mgc_all_clean(textGarCol);

	exit(FUN_RUN_END);
}


/*-----txbug_command_analyst-----*/
static void txbug_command_analyst(int nPara, char **pComm)
{
	int	nCir, tFlags, hFlags, cOff;

	tFlags = hFlags = 0;

	for(cOff = -1, nCir = 1; nCir < nPara; nCir++) {
		if(!strcmp(pComm[nCir], "--help") || !strcmp(pComm[nCir], "-h")) {
			hFlags = 1; break;

		} else if(!strcmp(pComm[nCir], "--write_disc") || !strcmp(pComm[nCir], "-w")) {
			txbug_run_set(module_disc_init, txbug_tran_disc, (fwtf)txbug_tran_disc_force, 
				(fwtr)txbug_tran_disc_real, NULL, &txTranHand.tx_fd);

		} else if(!strcmp(pComm[nCir], "--write_db") || !strcmp(pComm[nCir], "-d")) {
			txbug_run_set(module_database_init, txbug_tran_db, (fwtf)txbug_tran_db_force, 
				(fwtr)txbug_tran_db_news, txbug_create_nv, &txTranHand.tx_sql);

		} else if(!strcmp(pComm[nCir], "-f")) {
			textProcFd = atoi(pComm[++nCir]);

		} else if(!strcmp(pComm[nCir], "-t")) {
			txbug_timbuf_init(pComm[++nCir]); tFlags = 1;

		} else if(!strcmp(pComm[nCir], "-c")) {
			cOff = ++nCir;

		} else {
			hFlags = 1; break;
		}
	}

	if(mc_conf_load("Textbug", ((cOff == -1) ? "/MuseSp/conf/textbug.cnf" : pComm[cOff])) == FUN_RUN_FAIL) {
		printf("Textbug---> load configure failed\n");
		txbug_perror("txbug_command_analyst - mc_conf_load", errno);
		mc_conf_unload();
		exit(FUN_RUN_FAIL);
	}

	if(hFlags) {
		txbug_print_help();
		mc_conf_unload();
		exit(FUN_RUN_OK);
	}

	if(textbugRunSet.ts_init == NULL)
		txbug_run_set(module_database_init, txbug_tran_db, (fwtf)txbug_tran_db_force, 
			(fwtr)txbug_tran_db_news, txbug_create_nv, &txTranHand.tx_sql);

	if(!tFlags)
		txbug_timbuf_init(NULL);
}


/*------------------------------------------
	Part Five: Initialization

	1. mainly_init
	2. txbug_init
	3. txbug_mempool_init
	4. txbug_timbuf_init
	5. txbug_conf_init
	6. txbug_searchlist_init
	7. txbug_databuf_init

--------------------------------------------*/

/*-----mainly_init-----*/
static int mainly_init(void)
{
	if(!sp_normal_init("Textbug", &textGarCol, (MSGSET **)&txbugMsgSet, txbug_msg_init, "textbug_err_log", textProcFd))
		return	FUN_RUN_END;

	/* mgc one init */
	if(mgc_one_init(&txbugResCol, (gcfun)mysql_free_result, TRY_A_LIMIT) == MGC_FAILED) {
		txbug_perror("mainly_init - mgc_one_init", errno);
		return	FUN_RUN_END;
	}

	if(!txbug_init())
		return	FUN_RUN_END;

	/* error tolerate time reading */
	if(mc_conf_read("tb_errtime_tolerate", CONF_NUM, &nToleError, sizeof(int)) == FUN_RUN_FAIL) {
		mc_conf_print_err("tb_errtime_tolerate");
		return	FUN_RUN_END;
	}

	/* sync time reading */
	if(mc_conf_read("sync_times", CONF_NUM, &cSyncTime, sizeof(int)) == FUN_RUN_FAIL) {
		mc_conf_print_err("sync_times");
		return	FUN_RUN_END;
	}

	/* txbug thread pool init */
	if(!(tbThreadPool = mpc_create(nTxbugPthread))) {
		txbug_perror("mainly_init - mpc_thread_create", errno);
		return	FUN_RUN_END;
	}

	txbug_signal_init();

	return	FUN_RUN_OK;
}


/*-----txbug_init-----*/
static int txbug_init(void)
{
	mato_init(&writeDataLock, 1);

	/* pthread limit num reading */
	if(mc_conf_read("textbug_pthread_num", CONF_NUM, &nTxbugPthread, sizeof(int)) == FUN_RUN_FAIL) {
		mc_conf_print_err("textbug_pthread_num");
		return	FUN_RUN_END;
	}

	/* sql download limit read */
	if(mc_conf_read("textbug_read_limit", CONF_NUM, &nLoadLimit, sizeof(int)) == FUN_RUN_FAIL) {
		mc_conf_print_err("textbug_read_limit");
		return	FUN_RUN_END;
	}

	if(nTxbugPthread > nLoadLimit) {
		printf("Textbug---> \"textbug_pthread_num\" should smaller than \"textbug_read_limit\"\n");
		return	FUN_RUN_END;
	}

	return	FUN_RUN_OK;
}


/*-----txbug_mempool_init-----*/
static int txbug_mempool_init(void)
{
	/* init web size pool */
	if((contMemPool = wmpool_create(nTxbugPthread, WMP_PAGESIZE)) == NULL) {
		printf("Textbug---> wmpool_create - webpage - failed\n");
		return	FUN_RUN_FAIL;
	}

	if(mgc_add(textGarCol, contMemPool, wmpool_destroy) == MGC_FAILED)
		txbug_perror("txbug_mempool_init - mgc_add - contMemPool", errno);

	/* init text info pool */
	if((tsMemPool = wmpool_create(nTxbugPthread, sizeof(TEXT))) == NULL) {
		printf("Textbug---> wmpool_create - text - failed\n");
		return	FUN_RUN_FAIL;
	}

	if(mgc_add(textGarCol, tsMemPool, wmpool_destroy) == MGC_FAILED)
		txbug_perror("txbug_mempool_init - mgc_add - tsMemPool", errno);

	if((txbugRunMp = mmdp_create(PROC_MP_MAX)) == NULL) {
		txbug_perror("txbug_mempool_init - mmdp_create", errno);
		return	FUN_RUN_FAIL;
	}

	if(mgc_add(textGarCol, txbugRunMp, (gcfun)mmdp_free_all) == MGC_FAILED)
		txbug_perror("txbug_mempool_init - mgc_add - txbugRunMp", errno);

	return	FUN_RUN_OK;
}


/*-----txbug_timbuf_init-----*/
static void txbug_timbuf_init(char *pTime)
{
	struct	tm	*nowTime;

	nowTime = time_str_extract(pTime);

	sprintf(urlsTblName, "U%d%02d%02d", nowTime->tm_year, nowTime->tm_mon, nowTime->tm_mday);
 	sprintf(newsTblName, "N%d%02d%02d", nowTime->tm_year, nowTime->tm_mon, nowTime->tm_mday);
}


/*-----txbug_conf_init-----*/
static void txbug_conf_init(void)
{
	MYSQL_RES	*confRes;
	MYSQL_ROW	conRow;
	SCONF		*pCpy;

	if(mysql_query(&urlDataBase, GET_CONF) != FUN_RUN_END) {
		txbug_dberr_dispose(&urlDataBase, urlsDbName, "txbug_conf_init - mysql_query");
		txbug_sig_error();
	}

	if((confRes = mysql_store_result(&urlDataBase)) == NULL) {
		txbug_dberr_dispose(&urlDataBase, urlsDbName, "txbug_conf_init - mysql_store_result");
		txbug_sig_error();
	}

	if((pCpy = urlConfigSave = calloc(mysql_num_rows(confRes), sizeof(SCONF))) == NULL) {
		mysql_free_result(confRes);
		elog_write("txbug_conf_init - calloc", FUNCTION_STR, ERROR_STR);
		txbug_sig_error();
	}

	if(mgc_add(textGarCol, urlConfigSave, free) == MGC_FAILED)
		elog_write("txbug_conf_init - mgc_add - urlConfigSave", FUNCTION_STR, ERROR_STR);

	for(; (conRow = mysql_fetch_row(confRes)); pCpy++) {
		strcpy(pCpy->c_domain, conRow[0]);
		strcpy(pCpy->c_conrloc, conRow[3]);
		strcpy(pCpy->c_srcstr, conRow[5]);
		strcpy(pCpy->c_charset, conRow[6]);

		txbug_searchlist_init(&pCpy->c_conbeg, conRow[1]);
		txbug_searchlist_init(&pCpy->c_conend, conRow[2]);
		txbug_searchlist_init(&pCpy->c_timbeg, conRow[4]);
	}

	mysql_free_result(confRes);
}


/*-----txbug_searchlist_init-----*/
static void txbug_searchlist_init(SSTRL **pList, char *searchStr)
{
	SSTRL	**pChain = pList;
	char	*sEnd, *sMov;
	int	struSize = sizeof(SSTRL);

	if((sMov = strchr(searchStr, '@')) == NULL) {
		elog_write("UCONF", "Ruined", searchStr);
		txbug_sig_error();
	}

	if((*pList = *pChain = mmdp_malloc(txbugRunMp, struSize)) == NULL) {
		elog_write("txbug_searchlist_init - mmdp_malloc", FUNCTION_STR, ERROR_STR);
		txbug_sig_error();
	}

	pChain = &((*pChain)->s_next);
	sprintf((*pList)->s_str, "%.*s", (int)(sMov - searchStr), searchStr);

	if(++sMov != (sEnd = strlen(searchStr) + searchStr)) {
		for(searchStr = sMov; sMov < sEnd; sMov++, searchStr = sMov, pChain = &((*pChain)->s_next)) {
			if(!(sMov = strchr(searchStr, '@'))) {
				elog_write("UCONF", "Ruined", searchStr);
				txbug_sig_error();
			}

			if((*pChain = mmdp_malloc(txbugRunMp, struSize)) == NULL) {
				elog_write("txbug_searchlist_init - calloc", FUNCTION_STR, ERROR_STR);
				txbug_sig_error();
			}

			sprintf((*pChain)->s_str, "%.*s", (int)(sMov - searchStr), searchStr);
			(*pChain)->s_next = NULL;
		}
	}

	(*pChain) = NULL;
}


/*-----txbug_databuf_init-----*/
static void txbug_databuf_init(void)
{
	int	readSize;

	if(mc_conf_read("textbug_contbuff_size", CONF_NUM, &readSize, sizeof(int)) == FUN_RUN_FAIL) {
		mc_conf_print_err("textbug_contbuff_size");
		exit(FUN_RUN_FAIL);
	}

	/* news content buff init */
	if((contStoreBuf = buff_stru_init(readSize)) == NULL)
		elog_write("txbug_databuf_init - buff_stru_init - content", FUNCTION_STR, ERROR_STR);

	if(mgc_add(textGarCol, contStoreBuf, buff_stru_free_all) == MGC_FAILED)
		elog_write("txbug_databuf_init - mgc_add - content", FUNCTION_STR, ERROR_STR);

	/* url replace buff init */
	if((urlSaveBuf = buff_stru_init(HUGE_BUF)) == NULL)
		elog_write("txbug_databuf_init - buff_stru_init - urlsave", FUNCTION_STR, ERROR_STR);

	if(mgc_add(textGarCol, urlSaveBuf, buff_stru_free_all) == MGC_FAILED)
		elog_write("txbug_databuf_init - mgc_add - urlsave", FUNCTION_STR, ERROR_STR);
}


/*------------------------------------------
	Part Six: Network

	1. txbug_url_abstract
	2. txbug_web_download
	3. txbug_web_initialize
	4. txbug_httpreq_send
	5. txbug_handling_httpreq

--------------------------------------------*/

/*-----txbug_url_abstract-----*/
static int txbug_url_abstract(char *pUrl, WEB *wPoint)
{
	int	nCir, urlLen;

	if(strncmp(pUrl, MATCH_HTTP, MHTTP_LEN)) {
		printf("Textbug---> txbug_url_abstract - lack of head for url: %s\n", pUrl);
		return	FUN_RUN_FAIL;
	}

	memset(wPoint, 0, sizeof(WEB));

	wPoint->web_port = 80;
	urlLen = strlen(pUrl);

	for(nCir = MHTTP_LEN; nCir < urlLen; nCir++) {
 		if(pUrl[nCir] == '/') {
			strcpy(wPoint->web_file, &pUrl[nCir]);
			strncpy(wPoint->web_host, &pUrl[MHTTP_LEN], nCir - MHTTP_LEN);
			return	FUN_RUN_OK;
		}
	}

	return	FUN_RUN_FAIL;
}


/*-----txbug_web_download-----*/
static int txbug_web_download(WEB *webStru, TEXT *pTex)
{
	char	*pRet;
	int	strRet, tRead, nSocket;

	if((nSocket = txbug_web_initialize(webStru, pTex)) < FUN_RUN_OK)
		return	nSocket;

	for(strRet = 1, tRead = 0; strRet && tRead != TEXT_NREAD; tRead++) {
		if((strRet = select_read(nSocket, pTex->wt_pool + pTex->wt_plsize, 
		RECE_DATA, 0, TINY_USEC)) == FUN_RUN_FAIL) {
			close(nSocket);
			return	FUN_RUN_FAIL;
		}

		if(strRet) {
			if((pRet = txbug_match_string(pTex->wt_pool + pTex->wt_plsize, 
			   urlConfigSave[pTex->wt_pattern].c_conend, strRet))) {
				pTex->wt_plsize = pRet - pTex->wt_pool;
				break;
			}
		}

		if((pTex->wt_plsize += strRet) + RECE_DATA  >= WMP_PAGESIZE)
			break;
	}

	pTex->wt_pool[pTex->wt_plsize] = 0;

	close(nSocket);
	return	FUN_RUN_OK;
}


/*-----txbug_web_initialize-----*/
static int txbug_web_initialize(WEB *webStru, TEXT *wtStru)
{
	struct	sockaddr_in	sockStru;
	struct	hostent		*pHost;
	int			nSocket;

	if((pHost = gethostbyname(webStru->web_host)) == NULL) {
		elog_write("txbug_web_initialize - gethostbyname", wtStru->wt_url, HERROR_STR);
		return	(h_errno == TRY_AGAIN || h_errno < HOST_NOT_FOUND) ? FUN_RUN_END : FUN_RUN_FAIL;
	}

	if((nSocket = socket(AF_INET, SOCK_STREAM, 0)) == FUN_RUN_FAIL) {
		elog_write("txbug_web_initialize - socket", wtStru->wt_url, ERROR_STR);
		return	FUN_RUN_END;
	}

	sockStru.sin_family = AF_INET;
	sockStru.sin_port = htons(webStru->web_port);
	sockStru.sin_addr = *((struct in_addr *)(pHost->h_addr));

	if(socket_set_timer(nSocket, TINY_TIME, 0, SO_SNDTIMEO) == FUN_RUN_FAIL) {
		txbug_perror("txbug_web_initialize - socket_set_timer", errno);
		return	FUN_RUN_END;
	}

	if(connect(nSocket, (struct sockaddr *)&sockStru, sizeof(struct sockaddr)) == FUN_RUN_FAIL) {
		elog_write("txbug_web_initialize - connect", FUNCTION_STR, webStru->web_host);
		close(nSocket);
		return	FUN_RUN_END;
	}

	if((wtStru->wt_plsize = txbug_httpreq_send(webStru, nSocket, wtStru)) < FUN_RUN_OK) {
		close(nSocket);
		return	wtStru->wt_plsize;
	}

	wtStru->wt_plsize--;

	return	nSocket;
}


/*-----txbug_httpreq_send-----*/
static int txbug_httpreq_send(WEB *wStru, int nSock, TEXT *tStru)
{
	char	*pChar, *pFind;
	char	strBuf[RECE_DATA];
	char	resBuf[4];
	int	nSave, httpRes, handRet;

	nSave = sprintf(strBuf, HTTP_GFILE_STR, wStru->web_file, wStru->web_host, rPac);
	
	if(write(nSock, strBuf, nSave) != nSave) {
		elog_write("txbug_httpreq_send - write", tStru->wt_url, ERROR_STR);
		return	FUN_RUN_END;
	}

	/* must said "nSave < FUN_RUN_OK" */
	if((nSave = select_read(nSock, strBuf, RECE_DATA, TAKE_A_SEC, 0)) < FUN_RUN_OK) {
		elog_write("txbug_httpreq_send - select_read", tStru->wt_url, ERROR_STR);
		return	FUN_RUN_END;
	}

	strBuf[nSave] = 0;
	if(!(nSave = ((pChar = strstr(strBuf, "\r\n\r\n")) == NULL) ? 0 : (nSave - (pChar - strBuf))))
		return	FUN_RUN_FAIL;

	//printf("Http: %s\n%.*s\n\n", tStru->wt_url, pChar - strBuf, strBuf);

	httpRes = atoi(strBuf + 9);
	if((handRet = txbug_handling_httpreq(httpRes, strBuf, tStru)) != FUN_RUN_OK) {
		sprintf(resBuf, "%d", httpRes);
		elog_write(resBuf, wStru->web_host, wStru->web_file);
		return	handRet;
	}

	if(pChar) {
		strcpy(tStru->wt_pool, pChar);

		if((pFind = strnstr(strBuf, MATCH_CHARSET, pChar - strBuf))) {
			pFind += MCHARSET_LEN;
			if((pChar = strstr(pFind, MATCH_LINKBRK)) != NULL)
				strncpy(tStru->wt_charset, pFind, pChar - pFind);
		}
	}

	return	nSave + 1;
}


/*-----txbug_handling_httpreq-----*/
static int txbug_handling_httpreq(int httpState, char *retBuf, TEXT *pText)
{
	if(httpState == RESP_CONNECT_OK)
		return	FUN_RUN_OK;

	if(httpState == RESP_PERM_MOVE) {
		txbug_dispose_http301(retBuf, pText);
		return	FUN_RUN_FAIL;
	}

	if(httpState == RESP_TEMP_MOVE || httpState == RESP_SERV_UNAVAIABLE || 
		httpState == RESP_CONNECT_ACCEPT)
		return	FUN_RUN_END;

	return	FUN_RUN_FAIL;
}


/*------------------------------------------
	Part Seven: Entrance

	1. txbug_main
	2. txbug_pthread_generator
	3. txbug_sql_download

--------------------------------------------*/

/*-----txbug_main-----*/
static void txbug_main(void)
{
	MSLRES	*uResult;
	MSLROW	uRow;
	TEXT	*pText;
	WEB	webStru;
	short	nLimit, nRet, tSync = cSyncTime;

	sprintf(selectSqlComm, GET_URL_LIMIT, urlsTblName, nToleError, newsDbName, newsTblName, nLoadLimit);

	while(FUN_RUN_OK) {
		if(!(uResult = txbug_sql_download()))
			continue;

		mgc_one_add(&txbugResCol, uResult);

		/* tSync used to force the buffer's data tran to I/O */
		txbug_data_sync(tSync);

		if(textProcFd) {
			if(sp_msg_frame_run(txbugMsgSet, uResult) == FUN_RUN_FAIL) {
				sleep(TAKE_A_NOTHING);
				continue;
			}
		}

		for(nLimit = 0; (uRow = mysql_fetch_row(uResult)); ) {
			if(nLimit++ == nTxbugPthread) {
				nLimit = 1;
				mpc_thread_wait(tbThreadPool);
			}

			if(txbug_url_abstract(uRow[1], &webStru) == FUN_RUN_OK) {
				pText = txbug_handler_alloc();

				sprintf(pText->wt_id, "%s", uRow[0]);
				sprintf(pText->wt_url, "%s", uRow[1]);

				pText->wt_pattern = atoi(uRow[2]);
				memset(pText->wt_charset, 0, CHARSET_LEN);

				if((nRet = txbug_web_download(&webStru, pText)) == FUN_RUN_OK) {
					txbug_txbug_charset_set(pText);
					txbug_pthread_generator(pText);
					continue;
				}

				txbug_pool_free(pText);

				(nRet == FUN_RUN_FAIL) ?
				txbug_db_rewind_state(WGURL_STATE, uRow[0]) :
				txbug_db_rewind_state(INC_ERRT, uRow[0]);
			}
		}

		//sp_stop();

		mgc_one_clean(&txbugResCol);
		sleep(TAKE_A_EYECLOSE);
	}
}


/*-----txbug_pthread_generator-----*/
static void txbug_pthread_generator(TEXT *textPoint)
{
	txbug_db_rewind_state(USING_STATE, textPoint->wt_id);

	if(!mpc_thread_wake(tbThreadPool, txbug_job, (void *)textPoint)) {
		elog_write("txbug_pthread_generator - mpc_thread_wake", FUNCTION_STR, "Thread List is full");
		txbug_sig_error();
	}
}


/*-----txbug_sql_download-----*/
static MSLRES *txbug_sql_download(void)
{
	MYSQL_RES	*pRes;

	if(mysql_query(&urlDataBase, selectSqlComm) != FUN_RUN_END) {
		txbug_dberr_dispose(&urlDataBase, urlsDbName, "txbug_sql_download - mysql_query - selectCom");
		return	NULL;
	}

	if((pRes = mysql_store_result(&urlDataBase)) == NULL) {
		if(mysql_errno(&urlDataBase))
			txbug_dberr_dispose(&urlDataBase, urlsDbName, "txbug_sql_download - mysql_store_result");

		return	NULL;
	}

	return	pRes;
}


/*------------------------------------------
	Part Eight: Running setting

	1. txbug_handler_alloc
	2. txbug_txbug_charset_set

--------------------------------------------*/

/*-----txbug_handler_alloc-----*/
static TEXT *txbug_handler_alloc(void)
{
	struct	tm	*tmStru;
	time_t		tType;
	TEXT		*pText;

	if((pText = wmpool_malloc(tsMemPool)) != NULL) {
		if((pText->wt_pool = wmpool_calloc(contMemPool)) != NULL)
			return	pText;

		wmpool_destroy(tsMemPool);
	}

	tType = time(NULL);
	tmStru = localtime(&tType);

	printf("%02d: %02d: %02d - txbug_handler_alloc - wmpool_malloc - failed - textmap: %u - loadmap: %u\n", 
		tmStru->tm_hour, tmStru->tm_min, tmStru->tm_sec, (u_int32_t)*(tsMemPool->wmp_map),
		(u_int32_t)*(contMemPool->wmp_map));
		
	txbug_sig_error();
	return	NULL;
}


/*-----txbug_txbug_charset_set-----*/
static void txbug_txbug_charset_set(TEXT *charText)
{
	if(!charText->wt_charset[0] || !strcmp(charText->wt_charset, "iso-8859-1"))
		sprintf(charText->wt_charset, "%s", urlConfigSave[charText->wt_pattern].c_charset);
}


/*------------------------------------------
	Part Nine: Textbug mainly work

	1. txbug_job
	2. txbug_get_title
	3. txbug_find_pubtime
	4. txbug_get_context
	5. txbug_match_string
	6. txbug_tran_chars
	7. txbug_norway_get_context
	8. txbug_get_title_end
	9. txbug_check_contxbug_end
	10. txbug_charset_cmpmode_set
	11. txbug_create_pubtime

--------------------------------------------*/

/*-----txbug_job-----*/
static void txbug_job(void *aParameter)
{
	TEXT	*textPoint = (TEXT *)aParameter;
	iconv_t	webCv = (iconv_t)-1;

	pthread_detach(pthread_self());

	if(txbug_charset_cmpmode_set(textPoint, &webCv) == FUN_RUN_FAIL) {
		txbug_db_rewind_state(FUNWG_STATE, textPoint->wt_id);
		goto	fun_pthread_end;
	}

	if(txbug_find_pubtime(textPoint) == FUN_RUN_FAIL)
		txbug_create_pubtime(textPoint);

	if(txbug_get_title(textPoint, webCv) == FUN_RUN_FAIL) {
		txbug_db_rewind_state(NOTIT_STATE, textPoint->wt_id);
		goto	fun_pthread_end;
	}

	if(txbug_get_context(textPoint, webCv) != FUN_RUN_OK) {
		txbug_db_rewind_state(NOCONT_STATE, textPoint->wt_id);
		goto	fun_pthread_end;
	}

	textbugRunSet.ts_wt(textPoint);
	txbug_db_rewind_state(RTURL_STATE, textPoint->wt_id);

	fun_pthread_end:

	if(webCv != (iconv_t)FUN_RUN_FAIL)
		iconv_close(webCv);

	txbug_pool_free(textPoint);
}


/*-----txbug_get_title-----*/
static int txbug_get_title(TEXT *pText, iconv_t dCv)
{
	char	*pBeg, *pEnd;
	int	tLen;
	
	/* get title between <title> and </title> */
	if(!(pBeg = strstr(pText->wt_pool, MATCH_TITLE)))
		return	FUN_RUN_FAIL;

	pBeg += MTITLE_LEN;
	txbug_pass_space(pBeg);

	if(!(pEnd = strstr(pBeg, MATCH_ETITLE)))
		return	FUN_RUN_FAIL;

	/* get the end of title */
	pEnd = txbug_get_title_end(pBeg, pEnd - pBeg);

	if(dCv != (iconv_t)FUN_RUN_FAIL) {
		/* transition charset and get title len & source len */
		if((tLen = txbug_tran_chars(pText, dCv, pBeg, pEnd - pBeg, pText->wt_title, WEB_TITLE_LEN)) < FUN_RUN_OK)
			return	FUN_RUN_FAIL;
	
		pText->wt_tlen = WEB_TITLE_LEN - tLen;

	} else {
		pText->wt_tlen = pEnd - pBeg;
		strncpy(pText->wt_title, pBeg, pText->wt_tlen);
	}

	return	FUN_RUN_OK;
}


/*-----txbug_find_pubtime-----*/
static int txbug_find_pubtime(TEXT *pText)
{
	char	*pBeg, *pLimit = pText->wt_pool + pText->wt_plsize;

	if(!(pBeg = txbug_match_string(pText->wt_pool, urlConfigSave[pText->wt_pattern].c_timbeg, pText->wt_plsize))) {
		elog_write("Cannot locate pubtime", pText->wt_url, EMPTY_OBJ_STR);
		return	FUN_RUN_FAIL;
	}

	for(; pBeg < pLimit; pBeg++) {
		if(isdigit(*pBeg) && isdigit(*(pBeg + 1)) && *(pBeg + 2) == ':' && isdigit(*(pBeg + 3))) {
			strncpy(pText->wt_time, pBeg, WEB_TIME_LEN - 1);
			pText->wt_time[WEB_TIME_LEN - 1] = 0;
			return	FUN_RUN_OK;
		}
	}

	elog_write("Cannot finding pubtime", pText->wt_url, EMPTY_OBJ_STR);

	return	FUN_RUN_FAIL;
}


/*-----txbug_get_context-----*/
static int txbug_get_context(TEXT *pText, iconv_t dCv)
{
	char	*cBeg, *cEnd, *pRloc;
	char	newsBuf[NEWS_BUFF_SIZE];
	int	bufSize;

	if(!(cBeg = txbug_match_string(pText->wt_pool, urlConfigSave[pText->wt_pattern].c_conbeg, pText->wt_plsize))) {
		elog_write("Cannot locate context begin", pText->wt_url, EMPTY_OBJ_STR);
		return	FUN_RUN_FAIL;
	}

	cEnd = pText->wt_pool + pText->wt_plsize;

	if(strcmp(urlConfigSave[pText->wt_pattern].c_conrloc, "NULL")) {
		if((pRloc = strstr(cBeg, urlConfigSave[pText->wt_pattern].c_conrloc)) && pRloc <= cEnd)
			cEnd = pRloc;
	}

	bufSize = txbug_norway_get_context(pText, cBeg, cEnd - cBeg, newsBuf, NEWS_BUFF_SIZE);

	if(bufSize < FUN_RUN_OK) {
		elog_write("Cannot get any context", pText->wt_url, EMPTY_OBJ_STR);
		return	bufSize;
	}

	if(dCv != (iconv_t)FUN_RUN_FAIL) {
		if((bufSize = txbug_tran_chars(pText, dCv, newsBuf, bufSize, pText->wt_pool, WMP_PAGESIZE)) < FUN_RUN_OK)
			return	FUN_RUN_FAIL;
	
		pText->wt_plsize = WMP_PAGESIZE - bufSize;

	} else {
		strncpy(pText->wt_pool, newsBuf, (pText->wt_plsize = bufSize));
	}

	//printf("Pool: %.*s\n", pText->wt_plsize, pText->wt_pool);

	return	FUN_RUN_OK;
}


/*-----txbug_norway_get_context-----*/
static int txbug_norway_get_context(TEXT *pText, char *searchBeg, int nLimit, char *toBuf, int toLimit)
{
	char	*pMov, *pEnd, *pFinal;
	int	nCnt = 0, nRet, nTotal = 0;

	pFinal = (pMov = searchBeg) + nLimit;

	for(; pMov && *pMov && pMov < pFinal && nTotal < toLimit; pMov += nRet) {
		if(((nRet = txbug_accept_word(pText, (unsigned char *)pMov)) & RET_VALUE_MASK) == RET_WORD_NO_ACC) {
			nRet >>= 2;

			if(nCnt && nTotal + nCnt <= toLimit) {
				strncpy(toBuf + nTotal, pMov - nCnt, nCnt);
				nTotal += nCnt;
				nCnt = 0;
			}

			if(*pMov == '<' || *pMov == '{') {
				pMov = (pEnd = strchr(pMov, *pMov + 2)) ? pEnd : pMov;

				if(txbug_check_contxbug_end(pMov))
					break;
			}

		} else {
			nRet >>= 2;
			nCnt += nRet;
		}
	}

	return	nTotal;
}


/*-----txbug_match_string-----*/
static char *txbug_match_string(char *pFind, SSTRL *begList, int nLimit)
{
	SSTRL	*strList;
	char	*pContent;

	for(strList = begList; strList; strList = strList->s_next) {
		if((pContent = strnstr(pFind, strList->s_str, nLimit)))
			return	pContent + strlen(strList->s_str);
	}

	return	NULL;
}


/*-----txbug_tran_chars-----*/
static size_t txbug_tran_chars(TEXT *pText, iconv_t cCv, char *inBuf, size_t iSize, char *outBuf, size_t oSize)
{
	size_t	outByte = oSize;

	while(iconv(cCv, &inBuf, &iSize, &outBuf, &outByte) == (size_t)FUN_RUN_FAIL) {
		if(errno != EILSEQ) {
			elog_write("txbug_tran_chars - iconv", ERROR_STR, pText->wt_url);
			break;
		}
		
		*inBuf = ' ';
	}

	return	outByte;
}


/*-----txbug_get_title_end-----*/
static char *txbug_get_title_end(char *pBeg, int tLimit)
{
	int	nCir;

	for(nCir = 0; nCir < tLimit; nCir++) {
		if(pBeg[nCir] == '|' || pBeg[nCir] == '_' || pBeg[nCir] == '-' || 
		pBeg[nCir] == '\r' || pBeg[nCir] == '\n') {
			/* situation like 1-2 */
			if(pBeg[nCir] == '-' && isdigit(pBeg[nCir + 1]))
				continue;

			return	&pBeg[nCir];
		}

		if(pBeg[nCir] == ' ' && pBeg[nCir + 1] == ' ' && pBeg[nCir + 2] == ' ')
			return	&pBeg[nCir];
	}

	return	pBeg + tLimit;
}


/*-----txbug_charset_cmpmode_set-----*/
static int txbug_charset_cmpmode_set(TEXT *pText, iconv_t *pCv)
{
	if(!strcmp(pText->wt_charset, "utf-8") || !strcmp(pText->wt_charset, "UTF-8")) {
		pText->wt_csetfun = txbug_is_utf8;
		pText->wt_cinc = UTF8_WORD_LEN;
		return	FUN_RUN_OK;
	}

	if(!strcmp(pText->wt_charset, "gb2312") || !strcmp(pText->wt_charset, "GB2312")) {
		pText->wt_csetfun = txbug_is_gb2312;
		pText->wt_cinc = GB2312_WORD_LEN;

	} else if(!strcmp(pText->wt_charset, "gbk") || !strcmp(pText->wt_charset, "GBK")) {
		pText->wt_csetfun = txbug_is_gbk;
		pText->wt_cinc = GBK_WORD_LEN;

	} else {
		printf("---> unaccepted charset: %s - %s\n", pText->wt_charset, pText->wt_url);
		txbug_sig_error();
	}

	if((*pCv = iconv_open("UTF-8", pText->wt_charset)) == (iconv_t)FUN_RUN_FAIL) {
		elog_write("txbug_charset_cmpmode_set - iconv_open", FUNCTION_STR, ERROR_STR);
		return	FUN_RUN_FAIL;
	}

	return	FUN_RUN_OK;
}


/*-----txbug_create_pubtime-----*/
static void txbug_create_pubtime(TEXT *newText)
{
	struct	tm	*tmStru;
	time_t		nTime;

	nTime = time(NULL);
	tmStru = localtime(&nTime);

	sprintf(newText->wt_time, "%d:%d", tmStru->tm_hour, tmStru->tm_min);
}


/*-----txbug_check_contxbug_end-----*/
static inline int txbug_check_contxbug_end(char *contEnd)
{
	/* 相关 */
	if(*((unsigned int *)contEnd) == 0xD8B9E0CF || !strncmp(contEnd, "相关", 6) || !strncmp(contEnd, "【相关", 9)) {
		/* 报道 or 图集 */
		if(*((unsigned int *)(contEnd + 4)) == 0xC0B5A8B1 || *((unsigned int *)(contEnd + 4)) == 0xAFBCBCCD ||
		   !strncmp(contEnd + 6, "阅读", 6) || !strncmp(contEnd + 6, "报道", 6) || !strncmp(contEnd + 6, "推荐", 6))
			return	FUN_RUN_OK;
	}

	return	FUN_RUN_END;
}


/*------------------------------------------
	Part Ten: Message part

	1. txbug_keep_working
	2. txbug_time_change
	3. txbug_send_message

--------------------------------------------*/

/*-----txbug_keep_working-----*/
void txbug_keep_working(void *pResult)
{
	if(!mysql_num_rows((MSLRES *)pResult))
		sleep(TAKE_A_EYECLOSE);
}


/*-----txbug_time_change-----*/
void txbug_time_change(void)
{
	mpc_thread_wait(tbThreadPool);

	textbugRunSet.ts_wtf(&txTranHand.tx_sql, &writeDataLock, contStoreBuf);

	txbug_wait_arouse(sp_msg_frame_fd(txbugMsgSet), TAKE_A_REST);
	txbug_timbuf_init(NULL);
	textbugRunSet.ts_fctim();
	sprintf(selectSqlComm, GET_URL_LIMIT, urlsTblName, nToleError, newsDbName, newsTblName, nLoadLimit);

	return;
}


/*-----txbug_send_message-----*/
int txbug_send_message(int msgFd)
{
	PMSG	sendMsg;

	memset(&sendMsg, 0, MSG_LEN);
	sp_msg_fill_stru(&sendMsg, PART_EXTBUG, KEEP_WORKING);

	if(sp_msg_write(msgFd, &sendMsg) == FUN_RUN_FAIL) {
		elog_write("txbug_send_message - sp_msg_write", FUNCTION_STR, ERROR_STR);
		return	FUN_RUN_FAIL;
	}

	return	FUN_RUN_OK;
}


/*------------------------------------------
	Part Eleven: Charset

	1. txbug_accept_word
	2. txbug_is_utf8
	3. txbug_is_gb2312
	4. txbug_is_gbk

--------------------------------------------*/

/*-----txbug_accept_word-----*/
static int txbug_accept_word(TEXT *wText, unsigned char *pChar)
{
	int	nRet = 0x4;

	if(wText->wt_csetfun(pChar))
		return	(1 | (wText->wt_cinc << 2));

	return	nRet;
}


/*-----txbug_is_utf8-----*/
static inline int txbug_is_utf8(unsigned char *wordBeg)
{
	return	((*wordBeg & UTF8_WHEAD) == UTF8_WHEAD) ? FUN_RUN_OK : FUN_RUN_END;
}


/*-----txbug_is_gb2312-----*/
static inline int txbug_is_gb2312(unsigned char *wordBeg)
{
	return	(*wordBeg >= GB2312_BEG && *wordBeg <= GB2312_END) ? FUN_RUN_OK : FUN_RUN_END;
}


/*-----txbug_is_gbk-----*/
static inline int txbug_is_gbk(unsigned char *wordBeg)
{
	return	(*wordBeg >= GBK_BEG && *wordBeg <= GBK_END) ? FUN_RUN_OK : FUN_RUN_END;
}
