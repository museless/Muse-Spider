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
	Source file content Eleven part

	Part Zero:	Include
	Part One:	Local data
	Part Two:	Local function
	Part Three:	Define

	Part Four:	Urlbug main
	Part Five:	Initialization
	Part Six:	Network
	Part Seven:	About pthread
	Part Eight:	Entrance
	Part Nine:	Urlbug mainly work
	Part Ten:	Message pluging

--------------------------------------------*/

/*------------------------------------------
	Part Zero: Include
--------------------------------------------*/

#include "spinc.h"
#include "spnet.h"
#include "spdb.h"
#include "spmsg.h"
#include "sphtml.h"
#include "spmpool.h"
#include "spframe.h"

#include "mipc.h"
#include "mpctl.h"

#include "spuglobal.h"
#include "spurlb.h"


/*------------------------------------------
	Part One: Local data
--------------------------------------------*/

static	char	timComMode[DATE_CMODE][SMALL_BUF];
static	int	nCatchNum;


/*------------------------------------------
	Part Two: Local function
--------------------------------------------*/

/* Part Four */
static	void	ubug_command_analyst(int nPara, char **pComm);

/* Part Five */
static	int	mainly_init(void);
static	int	ubug_init_mempool(void);
static	void	ubug_init_datebuf(char *pTime);
static	void	ubug_init_dbuf(void);
static	void	ubug_init_weblist(void);
static	void	ubug_init_urllist(char *pUrl, WEB *pWeb);

/* Part Six */
static	int	network_send_httpreq(WEBIN *wInfo, int nSock);
static	int	network_get_addr(WEBIN *wInfo);
static	int	network_down_web(WEBIN *wInfo);

/* Part Seven */
static	void	ubug_create_pthread(WEBIN *webNode);
static	void	ubug_pthread_entrance(void *nParameter);

/* Part Eight */
static	void	ubug_main_entrance(void);
static	void	ubug_text_abstract_cont(WEBIN *abPoint);
static	void	ubug_text_store_content(WEBIN *stPoint);

/* Part Nine */
static	void	ubug_job(WEBIN *wPoint);
static	char	*ubug_connect_head(WEBIN *wInfo, int hostLen, char *fName, int *fLen);
static	int	ubug_get_pattern(char *pHost);
static	void	ubug_check_separator(char *pUrl, int *uLen);
static	int	ubug_check_url_prefix(char *preSrc);
static	int	ubug_find_key_date(char *cSrc, int nLimit);
static	char	*ubug_reach_str_http(char *pSrc, char *pLimit);


/*------------------------------------------
	Part Three: Define
--------------------------------------------*/

#define	ubug_init_ubset(fInit, fEnt, fSt, fStf, fStr, fDway, fChk, rTime) { \
	urlRunSet.ubs_init = fInit; \
	urlRunSet.ubs_fent = fEnt; \
	urlRunSet.ubs_fst = fSt; \
	urlRunSet.ubs_fstf = fStf; \
	urlRunSet.ubs_fstr = fStr; \
	urlRunSet.ubs_dway = fDway; \
	urlRunSet.ubs_chk = fChk; \
	urlRunSet.ubs_rtime = rTime; \
}


/*------------------------------------------
	Part Four: Urlbug main

	1. main
	2. ubug_command_analyst

--------------------------------------------*/

/*-----main-----*/
int main(int argc, char *argv[])
{
	ubug_command_analyst(argc, argv);

	if(mainly_init() && ubug_init_mempool()) {
		ubug_init_signal();
		ubug_init_database();
		ubug_init_weblist();
		ubug_init_dbuf();
	
		ubug_main_entrance();
	}

	mgc_all_clean(urlGarCol);

	exit(FUN_RUN_END);
}


/*-----ubug_command_analyst-----*/
static void ubug_command_analyst(int nPara, char **pComm)
{
	int	nCir, cOff, tFlags = 0;

	initMsgFlags = 0;

	for(cOff = -1, nCir = 1; nCir < nPara; nCir++) {
		if(!strcmp(pComm[nCir], "--help") || !strcmp(pComm[nCir], "-h")) {
			ubug_print_help();
			exit(FUN_RUN_OK);

		} else if(!strcmp(pComm[nCir], "--write_disc") || !strcmp(pComm[nCir], "-w")) {
			ubug_init_ubset(ubug_disc_init, ubug_text_abstract_cont, ubug_tran_disc, ubug_tran_disc_force,
			ubug_tran_disc_real, network_down_web, NULL, RUN_ONCE);

		} else if(!strcmp(pComm[nCir], "--write_db") || !strcmp(pComm[nCir], "-d")) {
			ubug_init_ubset(NULL, ubug_text_abstract_cont, ubug_tran_db, ubug_tran_db_force,
			ubug_tran_db_real, network_down_web, ubug_url_review, RUN_PERN);

		} else if(!strcmp(pComm[nCir], "--down_website") || !strcmp(pComm[nCir], "-l")) {
			ubug_init_ubset(NULL, ubug_text_store_content, NULL, NULL, NULL,
			network_down_web, NULL, RUN_ONCE);

		} else if(!strcmp(pComm[nCir], "-l") || !strcmp(pComm[nCir], "--link")) {
			initMsgFlags = 1;

		} else if(!strcmp(pComm[nCir], "-t")) {
			ubug_init_datebuf(pComm[++nCir]); tFlags = 1;

		} else if(!strcmp(pComm[nCir], "-c")) {
			cOff = ++nCir;

		} else {
			printf("Urlbug---> wrong command: %s\n \
			\r--->please try \"-h\" or \"--help\"\n\n", pComm[nCir]);
			exit(FUN_RUN_END);
		}
	}

	if(mc_conf_load("Urlbug", ((cOff == -1) ? "/MuseSp/conf/urlbug.cnf" : pComm[cOff])) == FUN_RUN_FAIL) {
		printf("Urlbug---> load configure failed\n");
		perror("Urlbug---> ubug_command_analyst - mc_conf_load");
		mc_conf_unload();
		exit(FUN_RUN_FAIL);
	}

	if(urlRunSet.ubs_fent == NULL) {
		ubug_init_ubset(NULL, ubug_text_abstract_cont, ubug_tran_db, ubug_tran_db_force,
		ubug_tran_db_real, network_down_web, ubug_url_review, RUN_PERN);
	}

	if(!tFlags)
		ubug_init_datebuf(NULL);
}


/*------------------------------------------
	Part Five: Initialization

	1. mainly_init
	2. ubug_init_mempool
	3. ubug_init_datebuf
	4. ubug_init_dbuf
	5. ubug_init_weblist
	6. ubug_init_urllist

--------------------------------------------*/

/*-----mainly_init-----*/
static int mainly_init(void)
{
	if(!sp_normal_init("Urlbug", &urlGarCol, &urlMsgSet, ubug_msg_init, "urlbug_err_log", initMsgFlags))
		return	FUN_RUN_END;

	mato_init(&writeStoreLock, 1);

	/* urlbug pthread num read */
	if(mc_conf_read("urlbug_pthread_num", CONF_NUM, &nRunPthread, sizeof(int)) == FUN_RUN_FAIL) {
		mc_conf_print_err("urlbug_pthread_num");
		return	FUN_RUN_END;
	}

	if(nRunPthread > UBUG_PTHREAD_MAX || nRunPthread < UBUG_PTHREAD_MIN) {
		printf("Urlbug---> pthread num is underlimit: %d\n", nRunPthread);
		return	FUN_RUN_END;
	}

	/* muse thread pool init */
	if(!(ubugThreadPool = mpc_create(nRunPthread))) {
		printf("Urlbug---> mainly_init - mpc_create - failed\n");
		return	FUN_RUN_END;
	}

	return	FUN_RUN_OK;
}


/*-----ubug_init_mempool-----*/
static int ubug_init_mempool(void)
{
	if((contStorePool = wmpool_create(nRunPthread, WMP_PAGESIZE)) == NULL) {
		printf("Urlbug---> wmpool_create - contStorePool: %s", strerror(errno));
		return	FUN_RUN_END;
	}

	if(mgc_add(urlGarCol, contStorePool, wmpool_destroy) == MGC_FAILED)
		perror("Urlbug---> ubug_init_mempool - mgc_add - contStorePool");

	if((urlStorePool = wmpool_create(nRunPthread, NAMBUF_LEN)) == NULL) {
		printf("Urlbug---> wmpool_create - urlStorePool: %s", strerror(errno));
		return	FUN_RUN_END;
	}

	if(mgc_add(urlGarCol, urlStorePool, wmpool_destroy) == MGC_FAILED)
		perror("Urlbug---> ubug_init_mempool - mgc_add - urlStorePool");

	return	FUN_RUN_OK;
}


/*-----ubug_init_datebuf-----*/
static void ubug_init_datebuf(char *pTime)
{
	TMS	*nowTime;
	char	strYear[5];

	nowTime = time_str_extract(pTime);

	sprintf(strYear, "%d", nowTime->tm_year);

	sprintf(timComMode[0], "%s/%02d%02d", &strYear[2], nowTime->tm_mon, nowTime->tm_mday);
	sprintf(timComMode[1], "%s%02d%02d", strYear, nowTime->tm_mon, nowTime->tm_mday);
	sprintf(timComMode[2], "%s-%02d-%02d", &strYear[2], nowTime->tm_mon, nowTime->tm_mday);
	sprintf(timComMode[3], "%s-%02d/%02d", strYear, nowTime->tm_mon, nowTime->tm_mday);
	sprintf(timComMode[4], "%s_%02d_%02d", strYear, nowTime->tm_mon, nowTime->tm_mday);
	sprintf(timComMode[5], "%s%02d/%02d", &strYear[2], nowTime->tm_mon, nowTime->tm_mday);
	sprintf(timComMode[6], "%s/%02d-%02d", strYear, nowTime->tm_mon, nowTime->tm_mday);

	sprintf(urlTabName, "U%s%02d%02d", strYear, nowTime->tm_mon, nowTime->tm_mday);
}


/*-----ubug_init_dbuf-----*/
static void ubug_init_dbuf(void)
{
	if((urlBufStu = buff_stru_init(SQL_LCOM_LEN)) == NULL) {
		perror("Urlbug---> ubug_init - buff_stru_init");
		exit(FUN_RUN_FAIL);
	}

	if(mgc_add(urlGarCol, urlBufStu, buff_stru_free_all) == MGC_FAILED)
		perror("Urlbug---> ubug_init_dbuf - mgc_add - urlBufStu");
}


/*-----ubug_init_weblist-----*/
static void ubug_init_weblist(void)
{
	MSLRES	*allRes;
	MSLROW	allRow;
	WEBIN	**pList = &urlSaveList;

	if(mysql_query(&urlDataBase, GET_DIRECTORY) != FUN_RUN_END) {
		if(ubug_dberr_dispose(&urlDataBase, "ubug_init_weblist - mysql_query - GET_DIRECTORY") != FUN_RUN_OK)
			ubug_sig_error();
	}

	if(!(allRes = mysql_store_result(&urlDataBase))) {
		printf("Urlbug---> ubug_init_weblist - mysql_store_result - no web\n");

		if(ubug_dberr_dispose(&urlDataBase, "ubug_init_weblist - mysql_store_result") != FUN_RUN_OK)
			ubug_sig_error();
	}

	while((allRow = mysql_fetch_row(allRes))) {
		if((*pList = malloc(sizeof(WEBIN))) == NULL) {
			elog_write("ubug_init_weblist - malloc", FUNCTION_STR, ERROR_STR);
			ubug_sig_error();
		}

		ubug_init_urllist(allRow[0], &((*pList)->w_ubuf));

		if(allRow[1]) {
			(*pList)->w_latest[strlen(allRow[1])] = 0;
			strcpy((*pList)->w_latest, allRow[1]);
		}

		if(!network_get_addr(*pList)) {
			free(*pList);
			continue;
		}

		pList = &((*pList)->w_next);
		/* in order to keep mgc_all_clean safety */
		(*pList) = NULL;
	}

	mysql_free_result(allRes);

	(*pList) = NULL;

	if(mgc_add(urlGarCol, NULL_POINT, ubug_free_weblist) == MGC_FAILED)
		ubug_perror("ubug_init_weblist - mgc_add", errno);
}


/*-----ubug_init_urllist-----*/
static void ubug_init_urllist(char *pUrl, WEB *pWeb)
{
	int	nCir, urlLen;

	bzero(pWeb, sizeof(WEB));

	if(strncmp(pUrl, MATCH_HTTP, MHTTP_LEN)) {
		printf("ubug_init_urllist - lack of http: %s\n", pUrl);
		exit(FUN_RUN_END);
	}

	pWeb->web_port = 80;
	urlLen = strlen(pUrl);

	for(nCir = MHTTP_LEN; nCir < urlLen; nCir++) {
 		if(pUrl[nCir] == '/') {
			sprintf(pWeb->web_file, "%s", &pUrl[nCir]);
			sprintf(pWeb->web_host, "%.*s", nCir - MHTTP_LEN, &pUrl[MHTTP_LEN]);
			return;
		}
	}
}


/*------------------------------------------
	Part Six: Network

	1. network_down_web
	2. network_send_httpreq
	3. network_get_addr

--------------------------------------------*/

/*-----network_down_web-----*/
static int network_down_web(WEBIN *wInfo)
{
	int	tRead, strRet, sockFd, contOff = 0;

	if((sockFd = socket(AF_INET, SOCK_STREAM, 0)) == FUN_RUN_FAIL) {
		perror("Urlbug---> network_down_web - socket");
		return	FUN_RUN_END;
	}

	if(socket_set_timer(sockFd, TINY_TIME, 0, SO_SNDTIMEO) == FUN_RUN_FAIL) {
		perror("Urlbug---> network_down_web - socket_set_timer");
		close(sockFd);
		return	FUN_RUN_END;
	}

	if(connect(sockFd, (struct sockaddr *)&wInfo->w_sockaddr, sizeof(struct sockaddr)) == FUN_RUN_FAIL) {
		printf("Urlbug---> network_down_web - connect: %s%s\n%s\n",
		wInfo->w_ubuf.web_host, wInfo->w_ubuf.web_file, strerror(errno));
		close(sockFd);
		return	FUN_RUN_END;
	}

	contOff = network_send_httpreq(wInfo, sockFd);
	if(contOff == FUN_RUN_END || contOff == FUN_RUN_FAIL) {
		close(sockFd);
		return	FUN_RUN_FAIL;
	}

	for(tRead = 0; tRead < UBUG_NREAD; tRead++) {
		if((strRet = select_read(sockFd, wInfo->w_conbuf + contOff, RECE_DATA, 0, TINY_USEC)) == FUN_RUN_FAIL)
			break;

		if((strRet >= MEHTML_LEN && strnstr(wInfo->w_conbuf + contOff, MATCH_ENDHTML, strRet)) || 
		contOff > WMP_PSIZE_BOR) {
			wInfo->w_conbuf[contOff += strRet] = 0;
			break;
		}

		contOff += (strRet > 0) ? strRet : 0;
	}

	//printf("%s - %s - %d\n", wInfo->w_ubuf.web_host, wInfo->w_ubuf.web_file, contOff);

	close(sockFd);

	return	contOff;
}


/*-----network_send_httpreq-----*/
static int network_send_httpreq(WEBIN *wInfo, int nSock)
{
	char	sqlCom[SQL_SCOM_LEN], urlData[URL_LEN], rwData[RECE_DATA], *pChar, *pEnd;
	int	fRet, httpChk;

	fRet = sprintf(rwData, "GET %s HTTP/1.1\r\nHost: %s\r\n%s",
			wInfo->w_ubuf.web_file, wInfo->w_ubuf.web_host, rPac);

	sprintf(urlData, "%s%s%s", MATCH_HTTP, wInfo->w_ubuf.web_host, wInfo->w_ubuf.web_file);

	if(write(nSock, rwData, fRet) != fRet) {
		elog_write("network_send_httpreq - write", urlData, ERROR_STR);
		return	FUN_RUN_END;
	}

	if((fRet = select_read(nSock, rwData, RECE_DATA, TAKE_A_SEC, TAKE_A_NO)) < FUN_RUN_OK) {
		elog_write("network_send_httpreq - select_read", urlData, ERROR_STR);
		return	FUN_RUN_END;
	}

	rwData[fRet] = 0;

	if(!(pChar = strstr(rwData, "\r\n\r\n"))) {
		elog_write("network_send_httpreq - httpret", urlData, "http ret wrong");
		return	FUN_RUN_FAIL;
	}

	//printf("pChar:\n%.*s\n", pChar - rwData, rwData);

	strcpy(wInfo->w_conbuf, pChar);
	fRet -= (pChar - rwData);

	httpChk = atoi(rwData + 9);
	if(httpChk != RESP_CONNECT_OK) {
		sprintf(sqlCom, "%d", httpChk);
		elog_write("cannot download website", urlData, sqlCom);
		return	FUN_RUN_END;
	}

	if((pChar = strstr(rwData, MATCH_LAMD)))
		pChar += MLAMD_LEN;

	else if((pChar = strstr(rwData, MATCH_DATE)))
		pChar += MDATE_LEN;

	if(!pChar || (pEnd = strstr(pChar, MATCH_LINKBRK)) == NULL)
		return	FUN_RUN_FAIL;

	if(!strncmp(pChar, wInfo->w_latest, pEnd - pChar)) {
		//printf("---> %s%s  not be updated\n", wInfo->w_ubuf.web_host, wInfo->w_ubuf.web_file);
		return	FUN_RUN_END;
	}

	if(pChar) {
		sprintf(sqlCom, UPDATE_LATEST, (int)(pEnd - pChar), pChar, urlData);

		while(!mato_dec_and_test(&writeStoreLock))
			mato_inc(&writeStoreLock);

		if(mysql_query(&urlDataBase, sqlCom) != FUN_RUN_END) {
			if(ubug_dberr_dispose(&urlDataBase, "send_httpreq - update latest") != FUN_RUN_OK)
				ubug_sig_error();
		}

		mato_inc(&writeStoreLock);
	}

	return	fRet;
}


/*-----network_get_addr-----*/
static int network_get_addr(WEBIN *wInfo)
{
	struct	hostent	*pHost;

	printf("Urlbug---> %s%s\n", wInfo->w_ubuf.web_host, wInfo->w_ubuf.web_file);

	while(!(pHost = gethostbyname(wInfo->w_ubuf.web_host))) {
		if(errno) {
			perror("Urlbug---> network_get_addr - gethostbyname");
			return	FUN_RUN_END;
		}
	}

	wInfo->w_sockaddr.sin_family = AF_INET;
	wInfo->w_sockaddr.sin_port = htons(80);
	wInfo->w_sockaddr.sin_addr = *((struct in_addr *)(pHost->h_addr));

	return	FUN_RUN_OK;
}


/*------------------------------------------
	Part Seven: About Pthread

	1. ubug_create_pthread
	2. ubug_pthread_entrance

--------------------------------------------*/

/*-----ubug_create_pthread-----*/
static void ubug_create_pthread(WEBIN *webNode)
{
	if(!(webNode->w_conbuf = wmpool_malloc(contStorePool))) {
		printf("Urlbug---> wmpool_malloc - conbuf failed - map: %d\n", *(contStorePool->wmp_map));
		ubug_sig_error();
	}

	if(!(webNode->w_url = wmpool_malloc(urlStorePool))) {
		printf("Urlbug---> wmpool_malloc - url failed - map: %d\n", *(urlStorePool->wmp_map));
		ubug_sig_error();
	}

	if((webNode->w_size = urlRunSet.ubs_dway(webNode)) > FUN_RUN_END) {
		if(mpc_thread_wake(ubugThreadPool, ubug_pthread_entrance, (void *)webNode))
			return;

		elog_write("ubug_create_pthread - mpc_thread_wake", FUNCTION_STR, ERROR_STR);
		ubug_sig_error();
	}

	wmpool_free(urlStorePool, webNode->w_url);
	wmpool_free(contStorePool, webNode->w_conbuf);
}


/*-----ubug_pthread_entrance-----*/
void ubug_pthread_entrance(void *nParameter)
{
	WEBIN	*webNode = (WEBIN *)nParameter;

	pthread_detach(pthread_self());

	urlRunSet.ubs_fent(webNode);

	wmpool_free(contStorePool, webNode->w_conbuf);
	wmpool_free(urlStorePool, webNode->w_url);
}


/*------------------------------------------
	Part Eight: Entrance

	1. ubug_main_entrance
	2. ubug_text_abstract_cont
	3. ubug_text_store_content

--------------------------------------------*/

/*-----ubug_main_entrance-----*/
static void ubug_main_entrance(void)
{
	WEBIN	*webPoint;
	int	nCnt;

	do {
		for(nCnt = nCatchNum = 0, webPoint = urlSaveList; webPoint != NULL; webPoint = webPoint->w_next) {
			if(nCnt++ == nRunPthread) {
				nCnt = 1;
				mpc_thread_wait(ubugThreadPool);
			}

			ubug_create_pthread(webPoint);
		}

		urlRunSet.ubs_fstf();

		if(initMsgFlags)
			sp_msgs_frame_run(urlMsgSet, NULL);

		sleep(TAKE_A_REST);

	} while(urlRunSet.ubs_rtime);

	ubug_sig_error();
}


/*-----ubug_text_abstract_cont-----*/
static void ubug_text_abstract_cont(WEBIN *abPoint)
{
	if((abPoint->w_pattern = ubug_get_pattern(abPoint->w_ubuf.web_host)) == FUN_RUN_FAIL)
		abPoint->w_pattern = -1;

	ubug_job(abPoint);
}


/*-----ubug_text_store_content-----*/
static void ubug_text_store_content(WEBIN *stPoint)
{

}


/*------------------------------------------
	Part Nine: Urlbug mainly work

	1. ubug_job
	2. ubug_get_pattern
	3. ubug_check_separator
	4. ubug_connect_head
	5. ubug_find_key_date
	6. ubug_check_url_prefix
	7. ubug_reach_str_http

--------------------------------------------*/

/*-----ubug_job-----*/
static void ubug_job(WEBIN *wPoint)
{
	char	*pHttp, *pBegin, *pSign;
	int	getLen, hostLen = strlen(wPoint->w_ubuf.web_host);

	for(pHttp = pBegin = wPoint->w_conbuf; pBegin && pBegin < wPoint->w_conbuf + wPoint->w_size; pBegin = pSign) {
		if(!(pHttp = strstr(pBegin, MATCH_HREF)))
			return;

		pHttp += MHREF_LEN;

		if(!(pSign = strchr(pHttp, '"')))
			return;

		if(!(pHttp = ubug_reach_str_http(pHttp, pSign)))
			continue;

		if((getLen = pSign - pHttp) >= WEBFILE_LEN)
			continue;

		if(!strncmp(pSign - MHTML_LEN, MATCH_HTML, MHTML_LEN) ||
		   !strncmp(pSign - MSHTML_LEN, MATCH_SHTML, MSHTML_LEN) ||
		   !strncmp(pSign - MHTM_LEN, MATCH_HTM, MHTM_LEN)) {
			if(!ubug_find_key_date(pHttp, getLen))
				continue;

			ubug_check_separator(pHttp, &getLen);

			if(strncmp(pHttp, MATCH_HTTP, MHTTP_LEN)) {
				if((pHttp = ubug_connect_head(wPoint, hostLen, pHttp, &getLen)) == NULL)
					continue;
			}

			if(ubug_check_url_prefix(pHttp) || 
			(urlRunSet.ubs_chk && (urlRunSet.ubs_chk(pHttp, getLen) == FUN_RUN_OK)))
				continue;

			urlRunSet.ubs_fst(pHttp, getLen, wPoint->w_pattern);
			nCatchNum++;
		}
	}
}


/*-----ubug_get_pattern-----*/
static int ubug_get_pattern(char *pHost)
{
	int	nCir;

	for(nCir = 0; nCir < webPattNum; nCir++) {
		if(strstr(pHost, perWebPatt[nCir]))
			return	nCir;
	}

	return	FUN_RUN_FAIL;
}


/*-----ubug_check_separator-----*/
static void ubug_check_separator(char *pUrl, int *uLen)
{
	char	*pBeg, *pEnter;
	int	nDec;

	if((pBeg = strnchr(pUrl, '\r', *uLen)) || (pBeg = strnchr(pUrl, '\n', *uLen))) {
		nDec = ((*pBeg == '\r') ? MLINK_LEN : 1);
		*uLen -= nDec;

		/* only one "\r\n" existed */
		if((pEnter = strnchr(pBeg + nDec, *pBeg, *uLen - (pBeg - pUrl))) == NULL) {
			strncpy(pBeg, pBeg + nDec, *uLen - (pBeg - pUrl));

		} else {
			strncpy(pBeg, pEnter + nDec, *uLen - (pEnter - pUrl));
			*uLen -= (pEnter - pBeg);
		}
	}
}


/*-----ubug_connect_head-----*/
static char *ubug_connect_head(WEBIN *wInfo, int hostLen, char *fName, int *fLen)
{
	char	*puBuf;

	if(!wInfo || !fName)
		return	NULL;

	memset(wInfo->w_url, 0, NAMBUF_LEN);
	strcpy(wInfo->w_url, MATCH_HTTP);
	strncpy(wInfo->w_url + MHTTP_LEN, wInfo->w_ubuf.web_host, hostLen);
	puBuf = wInfo->w_url + MHTTP_LEN + hostLen;

	/* if the url looks like href="./xxxxx.html" */
	while(*fName == '.')
		fName++, (*fLen)--;

	/* url looks like href="xxxxx.html" */
	if(*fName != '/') {
		*puBuf++ = '/';
		(*fLen)++;
	}

	strncpy(puBuf, fName, *fLen);
	*fLen += (hostLen + MHTTP_LEN);

	return	wInfo->w_url;
}


/*-----ubug_find_key_date-----*/
static int ubug_find_key_date(char *cSrc, int nLimit)
{
	int	nCir;

	for(nCir = 0; nCir < DATE_CMODE; nCir++) {
		if(strnstr(cSrc, timComMode[nCir], nLimit))
			break;
	}

	return	(nCir < DATE_CMODE) ? FUN_RUN_OK : FUN_RUN_END;
}


/*-----ubug_check_url_prefix-----*/
static int ubug_check_url_prefix(char *preSrc)
{
	int	nCir;

	preSrc += MHTTP_LEN;

	for(nCir = 0; nCir < nFbWord; nCir++) {
		if(!strncmp(forbitStrList[nCir].fb_str, preSrc, forbitStrList[nCir].fb_len))
			return	FUN_RUN_OK;
	}

	return	FUN_RUN_END;
}


/*-----ubug_reach_str_http-----*/
static char *ubug_reach_str_http(char *pSrc, char *pLimit)
{
	for(; pSrc < pLimit; pSrc++) {
		if(isalnum(*pSrc) || *pSrc == '/' || *pSrc == '.')
			return	pSrc;
	}

	return	NULL;
}


/*------------------------------------------
	Part Ten: Message pluging

	1. ubug_time_change
	2. ubug_send_message

--------------------------------------------*/

/*-----ubug_time_change-----*/
void ubug_time_change(void)
{
	mpc_thread_wait(ubugThreadPool);

	urlRunSet.ubs_fstf();
	ubug_init_datebuf(NULL);
	ubug_create_dbtable();
}


/*-----ubug_send_message-----*/
int ubug_send_message(int nSock)
{
	SOCKMG	sendMsg;

	if(nCatchNum) {
		sp_msgs_fill(sendMsg, PART_URLBUG, PART_TEXTBUG, KEEP_WORKING);

		if(!sp_msgs_send(nSock, &sendMsg, sizeof(SOCKMG))) {
			elog_write("ubug_send_message - sp_msgs_send", FUNCTION_STR, ERROR_STR);
			return	FUN_RUN_END;
		}
	}

	return	FUN_RUN_OK;
}
