#include "spinc.h"
#include "spdb.h"
#include "sphtml.h"
#include "sptinc.h"

/* local data */
static	char	*fileStoreBuf;
static	MYSQL	*toolSqlHandler;
static	char	gloTabName[MIDDLE_BUF];
static	char	contSaveDic[NAMBUF_LEN];
static	char	modFileName[FNAME_LEN];

static	char	urlPatternStore[][PATTERN_LEN] = {
		"mofcom.gov.cn", "p5w.net", "ccstock.cn", "eeo.com.cn", "nbd.com.cn", "time-weekly.com", "guancha.cn",
		"cneo.com.cn", "caixin.com", "cankaoxiaoxi.com", "youth.cn", "stcn.com", "cnr.cn", "people.com.cn",
		"cnfol.com", "qq.com", "xinhuanet.com", "sina.com.cn", "ifeng.com", "china.com.cn", "cntv.cn", 
		"163.com", "hexun.com", "eastmoney.com", "chinanews.com", "yangtse.com", "bjnews.com.cn", 
		"ynet.com", "chinadaily.com.cn", "cfi.cn", "gmw.cn", "southcn.com"};

static	int	nUrlPattern = sizeof(urlPatternStore) / sizeof(urlPatternStore[0]);

/* local function */
static	int	tool_filebuf_init(char *fName);
static	int	tool_file_open(char *fName);
static	int	tool_database_init(int nDoor, char *tabName);
static	void	tool_dbtable_create(int nChose);
static	void	tool_news_db_init(char *namTab);

static	int	tool_get_npattern(char *pUrl, int nLen);

static	void	tool_upload_entrance(int nChose, int nDoor);
static	void	tool_content_deal(int fSize);
static	void	tool_fconf_deal(int fSize);
static	void	tool_uall_deal(int fSize);

static	void	tool_tran_content(void);

static	inline	int	tool_tran_urls(char *sqlComm, void *myRow);
static	inline	int	tool_tran_conf(char *sqlComm, void *myRow);
static	inline	int	tool_tran_uall(char *sqlComm, void *myRow);

static	void	tool_tran_data(int nDoor);

static	void	tool_make_pattern(void);
static	void	tool_sep_athname(void);
static	void	tool_sep_uaddr(void);

static	void	tool_funerr_deal(char *pStr);
static	void	tool_dberr_deal(void);
static	void	tool_db_shutdown(void);


/*------------------------------------------
	Source file content Nine part

	Part Zero:	Define function
	Part One:	Tool main
	Part Two:	Shared Part
	Part Three:	Upload Part
	Part Four:	Download Part
	Part Five:	Content download
	Part Six:	Make pattern
	Part NT-Nine:	Error manage
	Part Hundred:	Dispose

--------------------------------------------*/

/*------------------------------------------
	Part Zero: Define function

	1. tool_find_endsign

--------------------------------------------*/

#define	tool_find_endsign(pEnter) \
	for(; *pEnter != 0 && *pEnter != '$'; pEnter++) \
		;	/* nothing */


/*------------------------------------------
	Part One: Tool main

	1. main

--------------------------------------------*/

/*-----main-----*/
int main(int argc, char *argv[])
{
	int	fSize, nDoor, nCir;

	if(mc_conf_load("tool", "../conf/tool.cnf") == FUN_RUN_FAIL) {
		printf("SPTOOL---> conf '../conf/tool.cnf' cannot load\n");
		exit(FUN_RUN_FAIL);
	}

	for(nCir = 1; nCir < argc; nCir++, nDoor = 0) {
		if(!strcmp(argv[nCir], "-u") || !strcmp(argv[nCir], "--up")) {
			if(!strcmp(argv[nCir + 1], "urls")) {
				nDoor = URLS_DLOCKS;

			} else if(!strcmp(argv[nCir + 1], "conf")) {
				nDoor = CONF_DLOCKS;

			} else if(!strcmp(argv[nCir + 1], "uall")) {
				nDoor = UALL_DLOCKS;
			}

			if(tool_database_init(nDoor, argv[nCir + 2]) == FUN_RUN_OK) {
				if((fSize = tool_filebuf_init(modFileName)) != FUN_RUN_FAIL)
					tool_upload_entrance(nDoor, fSize);
			}

			tool_db_shutdown();
			nCir += MD_UPDOWN_AOFF;
	
		} else if(!strcmp(argv[nCir], "-d") || !strcmp(argv[nCir], "--down")) {
			if(!strcmp(argv[nCir + 1], "urls")) {
				nDoor = URLS_DLOCKS;

			} else if(!strcmp(argv[nCir + 1], "conf")) {
				nDoor = CONF_DLOCKS;

			} else if(!strcmp(argv[nCir + 1], "uall")) {
				nDoor = UALL_DLOCKS;
			}

			if(tool_database_init(nDoor, argv[nCir + 2]) == FUN_RUN_OK)
				tool_tran_data(nDoor);

			tool_db_shutdown();
			nCir += MD_UPDOWN_AOFF;
	
		} else if(!strcmp(argv[nCir], "--dnc")) {
			tool_news_db_init(argv[nCir + 1]);
			tool_tran_content();
			tool_db_shutdown();
			nCir += MD_DNC_AOFF;
	
		} else if(!strcmp(argv[nCir], "--make_pattern")) {
			tool_make_pattern();
			nCir += MD_MDPATT_AOFF;

		} else if(!strcmp(argv[nCir], "--help") || !strcmp(argv[nCir], "-h")) {
			printf("usage: \"-u\" \"--up\" or \"-d\" \"--down\"\n \
				\r\turls [date (example: 20140909)]\n \
				\r\tconf [conf_name]\n \
				\r\tuall [url_save_path]\n");
			printf("usage: \"--dnc\" [<date> (example: 20140909)]\n");
			printf("usage: \"--make_pattern\"\n\n");
			break;
		}
	}

	mc_conf_unload();

	exit(FUN_RUN_OK);
}


/*------------------------------------------
	Part Two: Shared Part

	1. tool_filebuf_init
	2. tool_database_init
	3. tool_dbtable_create
	4. tool_file_open

--------------------------------------------*/

/*-----tool_filebuf_init-----*/
static int tool_filebuf_init(char *fName)
{
	STAT	sBuf;
	int	fRet, nFd;

	if((nFd = open(fName, O_RDWR)) == FUN_RUN_FAIL) {
		perror("tool_filebuf_init - open");
		return	nFd;
	}

	if(fstat(nFd, &sBuf) == FUN_RUN_FAIL) {
		perror("tool_filebuf_init - fstat");
		return	FUN_RUN_FAIL;
	}

	if((fileStoreBuf = malloc(sBuf.st_size)) == NULL) {
		perror("tool_filebuf_init - malloc");
		return	FUN_RUN_FAIL;
	}

	if((fRet = readn(nFd, fileStoreBuf, sBuf.st_size)) == FUN_RUN_FAIL) {
		perror("tool_filebuf_init - readn");
		return	FUN_RUN_FAIL;
	}

	fileStoreBuf[sBuf.st_size - 1] = 0;

	close(nFd);

	return	fRet;
}


/*-----tool_database_init-----*/
static int tool_database_init(int nDoor, char *tabName)
{
	char	dbName[SMALL_BUF];
	int	cFlags = 0;

	mysql_library_init(nrOpt, sqlOpt, NULL);
	toolSqlHandler = mysql_init(NULL);

	if(mc_conf_read("urls_db_name", CONF_STR, dbName, SMALL_BUF) == FUN_RUN_FAIL) {
		mc_conf_print_err("urls_db_name");
		return	FUN_RUN_FAIL;
	}

	if(mysql_real_connect(toolSqlHandler, NULL, DBUSER, DBKEY, dbName, 0, NULL, cFlags) == NULL)
		tool_dberr_deal();

	if(nDoor == URLS_DLOCKS) {
		if(mc_conf_read("urls_save_path", CONF_STR, gloTabName, MIDDLE_BUF) == FUN_RUN_FAIL) {
			mc_conf_print_err("urls_save_path");
			return	FUN_RUN_FAIL;
		}

	} else if(nDoor == CONF_DLOCKS || nDoor == UALL_DLOCKS) {
		if(mc_conf_read("confs_save_path", CONF_STR, gloTabName, MIDDLE_BUF) == FUN_RUN_FAIL) {
			mc_conf_print_err("confs_save_path");
			return	FUN_RUN_FAIL;
		}

	} else {
		return	FUN_RUN_FAIL;
	}

	sprintf(modFileName, "%s/%s", gloTabName, tabName);
	sprintf(gloTabName, "U%s", tabName);

	return	FUN_RUN_OK;
}


/*-----tool_dbtable_create-----*/
static void tool_dbtable_create(int nChose)
{
	char	sqlCom[SQL_LEN];

	sprintf(sqlCom, DROP_TABLE, gloTabName);
	mysql_query(toolSqlHandler, sqlCom);

	switch(nChose) {
	  case	URLS_DLOCKS:
		sprintf(sqlCom, CREAT_URLSTAB, gloTabName);
		break;

	  case  CONF_DLOCKS:
		sprintf(sqlCom, CREAT_CONFTAB);
	  	break;

	  case	UALL_DLOCKS:
	  	sprintf(sqlCom, CREAT_UALLTAB);
	  	break;
	}

	if(mysql_query(toolSqlHandler, sqlCom) != FUN_RUN_END)
		tool_dberr_deal();
}


/*-----tool_file_open-----*/
static int tool_file_open(char *fName)
{
	int	retFd;

	if((retFd = open(fName, O_RDWR | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP)) == FUN_RUN_FAIL)
		perror("tool_file_open - open");

	return	retFd;
}


/*------------------------------------------
	Part Three: Upload Part

	1. tool_content_deal
	2. tool_fconf_deal
	3. tool_uall_deal

--------------------------------------------*/

/*-----tool_upload_entrance-----*/
static void tool_upload_entrance(int nChose, int fSize)
{
	tool_up	uploadFun;
	char	dropSql[SQL_LEN];

	switch(nChose) {
	  case	URLS_DLOCKS:
		uploadFun = tool_content_deal;
		sprintf(dropSql, DROP_TABLE, gloTabName);
		break;

	  case	CONF_DLOCKS:
		uploadFun = tool_fconf_deal;
		sprintf(dropSql, DROP_TABLE, "UCONF");
	  	break;

	  case	UALL_DLOCKS:
		uploadFun = tool_uall_deal;
		sprintf(dropSql, DROP_TABLE, "UALL");
	  	break;
	}

	mysql_query(toolSqlHandler, dropSql);

	tool_dbtable_create(nChose);
	uploadFun(fSize);

	free(fileStoreBuf);
}


/*-----tool_content_deal-----*/
static void tool_content_deal(int fSize)
{
	char	*pBuf = fileStoreBuf, *pUrl, *pDot, *pEnd;
	char	sqlBuf[SQL_LEN];
	int	nPattern;
	
	for(; pBuf < fileStoreBuf + fSize; pBuf = pEnd) {
		if(!(pUrl = strstr(pBuf, MATCH_HTTP)) || !(pDot = strchr(pUrl, '.')) || !(pEnd = strchr(pDot, '\n')))
			break;

		if((nPattern = tool_get_npattern(pUrl, pEnd - pUrl)) == FUN_RUN_FAIL)
			continue;

		sprintf(sqlBuf, INSERT_URLSTAB, gloTabName, (int)(pEnd - pUrl), pUrl, nPattern);

		if(mysql_query(toolSqlHandler, sqlBuf) != FUN_RUN_END)
			tool_dberr_deal();
	}
}


/*-----tool_fconf_deal-----*/
static void tool_fconf_deal(int fSize)
{
	char	*pEnter, *pBuf, *pStru;
	int	sizeBuf[CONF_NMEM] = {0, 32, 160, 288, 320, 448, 472};
	char	insertBuf[SQL_LEN];
	SPCONF	conStru;
	int	nCir;

	for(pEnter = pBuf = fileStoreBuf; pBuf - fileStoreBuf < fSize; ) {
		bzero(&conStru, sizeof(SPCONF));

		/* At pEnter += 2, 2 means '$' and ' ' */
		for(pStru = (char *)&conStru, nCir = 0; nCir < CONF_NMEM; nCir++, pEnter += 2, pBuf = pEnter) {
			tool_find_endsign(pEnter);
			mysql_real_escape_string(toolSqlHandler, pStru + sizeBuf[nCir], pBuf, pEnter - pBuf);
		}

		sprintf(insertBuf, INSERT_CONFTAB, conStru.c_domain, conStru.c_conbeg, conStru.c_conend, 
		conStru.c_conrloc, conStru.c_timbeg, conStru.c_srcstr, conStru.c_charset);

		if(mysql_query(toolSqlHandler, insertBuf) != FUN_RUN_END)
			tool_dberr_deal();
	}
}


/*-----tool_uall_deal-----*/
static void tool_uall_deal(int fSize)
{
	char	*pMov, *pEnter;
	char	insertBuf[SQL_LEN];

	for(pMov = pEnter = fileStoreBuf; pMov - fileStoreBuf < fSize; pMov = pEnter + 1) {
		if((pEnter = strchr(pMov, '\n')) == NULL)
			break;

		sprintf(insertBuf, INSERT_UALLTAB, (int)(pEnter - pMov), pMov);

		if(mysql_query(toolSqlHandler, insertBuf) != FUN_RUN_END)
			tool_dberr_deal();
	}
}


/*------------------------------------------
	Part Four: Download Part

	1. tool_tran_data
	2. tool_tran_urls
	3. tool_tran_conf
	4. tool_tran_uall

--------------------------------------------*/

/*-----tool_tran_data-----*/
static void tool_tran_data(int nChose)
{
	MYSQL_RES	*sqlRes;
	MYSQL_ROW	sqlRow;
	tool_tran	tranFun;
	char		sqlCom[SQL_LEN];
	int		wFd;

	if((wFd = tool_file_open(modFileName)) == FUN_RUN_FAIL)
		return;

	switch(nChose) {
	  case	URLS_DLOCKS:
	  	tranFun = tool_tran_urls;
	  	sprintf(sqlCom, SELECT_URLS, gloTabName);
	  	break;

	  case  CONF_DLOCKS:
	  	tranFun = tool_tran_conf;
	  	sprintf(sqlCom, SELECT_UCONF);
	  	break;

	  case	UALL_DLOCKS:
	  	tranFun = tool_tran_uall;
	  	sprintf(sqlCom, SELECT_UALL);
	  	break;
	}

	if(mysql_query(toolSqlHandler, sqlCom) != FUN_RUN_END)
		tool_dberr_deal();

	if(!(sqlRes = mysql_store_result(toolSqlHandler)))
		tool_dberr_deal();

	while((sqlRow = mysql_fetch_row(sqlRes)))
		write(wFd, sqlCom, tranFun(sqlCom, sqlRow));

	mysql_free_result(sqlRes);
	close(wFd);
}


/*-----tool_tran_urls-----*/
static inline int tool_tran_urls(char *sqlComm, void *myRow)
{
	return	sprintf(sqlComm, "%s\n", ((MYSQL_ROW)myRow)[1]);
}


/*-----tool_tran_conf-----*/
static inline int tool_tran_conf(char *sqlComm, void *myRow)
{
	return	sprintf(sqlComm, "%s$ %s$ %s$ %s$ %s$ %s$ %s$\n", ((MYSQL_ROW)myRow)[0], ((MYSQL_ROW)myRow)[1], 
		((MYSQL_ROW)myRow)[2], ((MYSQL_ROW)myRow)[3], ((MYSQL_ROW)myRow)[4], 
		((MYSQL_ROW)myRow)[5], ((MYSQL_ROW)myRow)[6]);
}


/*-----tool_tran_uall-----*/
static inline int tool_tran_uall(char *sqlComm, void *myRow)
{
	return	sprintf(sqlComm, "%s\n", ((MYSQL_ROW)myRow)[0]);
}


/*------------------------------------------
	Part Five: Content download

	1. tool_tran_content
	2. tool_news_db_init

--------------------------------------------*/

/*-----tool_tran_content-----*/
static void tool_tran_content(void)
{
	MYSQL_RES	*lRes, *cRes;
	MYSQL_ROW	lRow, cRow;
	char		sqlCom[SQL_LEN];
	BUFF		*contBuff;
	int		nFd;

	if((contBuff = buff_stru_init(BUFF_LEN)) == NULL)
		tool_funerr_deal("tool_tran_content - buff_stru_init");

	sprintf(sqlCom, SELECT_SRC);

	if(mysql_query(toolSqlHandler, sqlCom) != FUN_RUN_END)
		tool_dberr_deal();

	if(!(lRes = mysql_store_result(toolSqlHandler))) {
		printf("---> in tool_tran_content - lRes\n");
		tool_dberr_deal();
	}

	while((lRow = mysql_fetch_row(lRes))) {
		sprintf(sqlCom, "%s/%s", contSaveDic, lRow[0]);

		if((nFd = open(sqlCom, O_CREAT | O_RDWR | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP)) == FUN_RUN_FAIL)
			tool_funerr_deal("tool_tran_content - open");

		sprintf(sqlCom, SELECT_DATA, gloTabName, lRow[0]);

		if(mysql_query(toolSqlHandler, sqlCom) != FUN_RUN_END)
			tool_dberr_deal();

		if(!(cRes = mysql_store_result(toolSqlHandler))) {
			printf("---> in tool_tran_content - cRes\n");
			tool_dberr_deal();
		}

		printf("%s cnt: %lld\n", lRow[0], mysql_num_rows(cRes));

		while((cRow = mysql_fetch_row(cRes))) {
			if(!buff_size_enough(contBuff, (strlen(cRow[0]) + strlen(cRow[1]) + 
			strlen(cRow[2]) + strlen(cRow[3]) + strlen(cRow[4]) + 64))) {
				write(nFd, buff_place_start(contBuff), buff_now_size(contBuff));
				buff_stru_make_empty(contBuff);
			}

			buff_size_add(contBuff, sprintf(buff_place_end(contBuff),
				"%s  %s  %s\n%s\n%s\n\n", cRow[0], cRow[1], cRow[2], cRow[3], cRow[4]));
		}

		write(nFd, buff_place_start(contBuff), buff_now_size(contBuff));
		buff_stru_make_empty(contBuff);

		mysql_free_result(cRes);
		close(nFd);
	}

	mysql_free_result(lRes);
	buff_stru_free_all(contBuff);
}


/*-----tool_news_db_init-----*/
static void tool_news_db_init(char *namTab)
{
	mysql_library_init(nrOpt, sqlOpt, NULL);
	toolSqlHandler = mysql_init(NULL);

	if(mysql_real_connect(toolSqlHandler, NULL, DBUSER, DBKEY, NEWDB, 0, NULL, 0) == NULL)
		tool_dberr_deal();

	if(mc_conf_read("conts_save_path", CONF_STR, contSaveDic, NAMBUF_LEN) == FUN_RUN_FAIL) {
		mc_conf_print_err("conts_save_path");
		exit(FUN_RUN_FAIL);
	}

	sprintf(contSaveDic + strlen(contSaveDic), "/%s", namTab);

	if(mkdir(contSaveDic, S_IRUSR | S_IWUSR | S_IRGRP) == FUN_RUN_FAIL) {
		if(errno != EEXIST) {
			perror("tool_news_db_init - mkdir");
			exit(FUN_RUN_FAIL);
		}
	}

	sprintf(gloTabName, "N%s", namTab);
}


/*-----tool_get_npattern------*/
static int tool_get_npattern(char *pUrl, int nLen)
{
	int	nCir;
	char	saveCh;

	saveCh = pUrl[nLen];
	pUrl[nLen] = 0;

	for(nCir = 0; nCir < nUrlPattern; nCir++) {
		if(strstr(pUrl, urlPatternStore[nCir]))
			break;
	}

	pUrl[nLen] = saveCh;

	return	((nCir == nUrlPattern) ? FUN_RUN_FAIL : nCir);
}


/*------------------------------------------
	Part Six: Make pattern

	1. tool_make_pattern
	2. tool_sep_athname
	3. tool_sep_uaddr

--------------------------------------------*/

/*-----tool_make_pattern-----*/
static void tool_make_pattern(void)
{
	tool_sep_athname();
	tool_sep_uaddr();
}


/*-----tool_sep_athname-----*/
static void tool_sep_athname(void)
{
	char	*pStore, *pEnter;
	char	strBuf[PATH_LEN];
	int	uanSize, sppFd, nCir;

	if(mc_conf_read("uathname_save_path", CONF_STR, strBuf, PATH_LEN) == FUN_RUN_FAIL) {
		mc_conf_print_err("uathname_save_path");
		return;
	}

	if((uanSize = tool_filebuf_init(strBuf)) == FUN_RUN_FAIL)
		return;

	if(mc_conf_read("sppatt_header_path", CONF_STR, strBuf, PATH_LEN) == FUN_RUN_FAIL) {
		mc_conf_print_err("sppatt_header_path");
		return;
	}

	if((sppFd = tool_file_open(strBuf)) == FUN_RUN_FAIL)
		return;

	write(sppFd, strBuf, sprintf(strBuf, "#ifndef\t_SPPATT_H\n#define\t_SPPATT_H\n\n"));

	for(nCir = 1, pStore = fileStoreBuf; pStore < fileStoreBuf + uanSize; pStore = pEnter + 1) {
		if((pEnter = strchr(pStore, '\n')) == NULL)
			break;

		write(sppFd, strBuf, sprintf(strBuf, "#define\t%.*s\t%d\n", (int)(pEnter - pStore), pStore, nCir++));
	}

	write(sppFd, strBuf, sprintf(strBuf, "\n#endif\n"));

	close(sppFd);
	free(fileStoreBuf);
}


/*-----tool_sep_uaddr-----*/
static void tool_sep_uaddr(void)
{
	char	*pStore, *pEnter;
	char	strStore[PATH_LEN];
	int	uapSize, uspFd, ulpFd;

	if(mc_conf_read("urladdr_save_path", CONF_STR, strStore, PATH_LEN) == FUN_RUN_FAIL) {
		mc_conf_print_err("urladdr_save_path");
		return;
	}

	if((uapSize = tool_filebuf_init(strStore)) == FUN_RUN_FAIL)
		return;

	if(mc_conf_read("uaddr_separate_path", CONF_STR, strStore, PATH_LEN) == FUN_RUN_FAIL) {
		mc_conf_print_err("uaddr_separate_path");
		return;
	}

	if((uspFd = tool_file_open(strStore)) == FUN_RUN_FAIL)
		return;

	if(mc_conf_read("uaddr_link_path", CONF_STR, strStore, PATH_LEN) == FUN_RUN_FAIL) {
		mc_conf_print_err("uaddr_link_path");
		return;
	}

	if((ulpFd = tool_file_open(strStore)) == FUN_RUN_FAIL)
		return;

	for(pStore = fileStoreBuf; pStore && pStore < fileStoreBuf + uapSize; pStore = pEnter + 1) {
		if((pStore = strchr(pStore, '.'))) {
			pStore++;

			if((pEnter = strchr(pStore, '/'))) {
				write(uspFd, strStore, sprintf(strStore, "%.*s$\n", (int)(pEnter - pStore), pStore));
				write(ulpFd, strStore, sprintf(strStore, "\"%.*s\", ", (int)(pEnter - pStore), pStore));

				if((pEnter = strchr(pEnter, '\n')) == NULL)
					break;
			}
		}
	}

	free(fileStoreBuf);
	close(uspFd);
	close(ulpFd);
}


/*------------------------------------------
	Part NT-Nine: Error manage

	1. tool_funerr_deal
	2. tool_dberr_deal

--------------------------------------------*/

/*-----tool_funerr_deal-----*/
static void tool_funerr_deal(char *pStr)
{
	if(fileStoreBuf)
		free(fileStoreBuf);

	perror(pStr);
	exit(FUN_RUN_END);	
}


/*-----tool_dberr_deal-----*/
static void tool_dberr_deal(void)
{
	printf("DB err: %s\n", mysql_error(toolSqlHandler));
	tool_db_shutdown();
	tool_funerr_deal(NULL);
}


/*------------------------------------------
	Part Hundred: Dispose

	1. tool_db_shutdown

--------------------------------------------*/

/*-----tool_db_shutdown-----*/
static void tool_db_shutdown(void)
{
	mysql_close(toolSqlHandler);
	mysql_library_end();
}
