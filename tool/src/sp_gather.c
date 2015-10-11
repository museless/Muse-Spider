/*******************************************
 * this tool use for showing how many news
 * that every news publish src pulished
 * and make a sorting
 *******************************************/

/*------------------------------------------
	Source file content Seven part

	Part Zero:	Include
	Part One:	Define
	Part Two:	Local data
	Part Three:	Local function

	Part Four:	Src container
	Part Five:	Src sort
	Part Six:	Pack function

--------------------------------------------*/

/*------------------------------------------
	Part Zero: Include
--------------------------------------------*/

#include "spinc.h"
#include "spdb.h"

#include "malgo.h"


/*------------------------------------------
	Part One: Define
--------------------------------------------*/

/* typedef */
typedef	struct	srccontainer	SCT;

/* struct */
struct	srccontainer {
	char	*sct_str;
	int	sct_cnt;
};


/*------------------------------------------
	Part Two: Local data
--------------------------------------------*/

static	SCT	*srcContentContain;

static	MYSQL	sqlDataBase;
static	SSORTS	*gatShellSort;


/*------------------------------------------
	Part Three: Local function
--------------------------------------------*/

/* Part Four */
static	void	sct_perror(char *errStr);
static	void	sct_dberr_dispose(MYSQL *sqlHand, char *dbName, char *errHead);

/* Part Five */
static	void	sct_sort_init(void);
static	SCT	*sct_sort_mov(SCT *sctHand, SCT *sctData, int nMov);
static	SCT	*sct_sort_assign(SCT *pSct);
static	SCT	*sct_sort_progress(SCT *pHandler, SCT *pData);
static	void	sct_sort_cmprep(SCT *pSrc, SCT *pCmp);

/* Part Six */
static	void	*sct_malloc(void *pEmpty, int nSize);


/*------------------------------------------
	Part Four: Src container

	1. main
	2. sct_perror
	3. sct_dberr_dispose

--------------------------------------------*/

/*-----main-----*/
int main(int argc, char *argv[])
{
	MSLRES	*srcRes, *cntRes;
	SCT	*pSrc;
	MSLROW	datRow;
	char	sqlBuf[512];
	uLong	nLine;
	int	nSize, nCir;

	if(argc != 2) {
		printf("SCT---> usage: <date>");
		exit(FUN_RUN_FAIL);
	}

	mysql_library_init(nrOpt, sqlOpt, NULL);

	if(!mysql_init(&sqlDataBase))
		sct_perror("main - mysql_init");

	if(!mysql_real_connect(&sqlDataBase, NULL, "root", "WdSr0922", "News", 0, NULL, 0))
		sct_perror("main - mysql_real_connect");

	sct_sort_init();

	/* read UCONF */
	nSize = sprintf(sqlBuf, "select SrcStr from Url.UCONF");

	if(mysql_real_query(&sqlDataBase, sqlBuf, nSize))
		sct_dberr_dispose(&sqlDataBase, "News", "main - mysql_real_query");

	if(!(srcRes = mysql_store_result(&sqlDataBase)))
		sct_dberr_dispose(&sqlDataBase, "News", "main - mysql_store_result");

	if(!(nLine = mysql_num_rows(srcRes))) {
		printf("SCT---> there are no lines in UCONF\n");
		exit(FUN_RUN_FAIL);
	}

	if(!(srcContentContain = malloc(nLine * sizeof(SCT))))
		sct_perror("main - malloc");

	for(pSrc = srcContentContain, nCir = 0; nCir < nLine; nCir++, pSrc++) {
		datRow = mysql_fetch_row(srcRes);
		pSrc->sct_str = (char *)datRow[0];
		pSrc->sct_cnt = 0;
	}

	for(pSrc = srcContentContain, nCir = 0; nCir < nLine; nCir++, pSrc++) {
		nSize = sprintf(sqlBuf, "select count(*) from News.N%s where Source=\"%s\"", argv[1], pSrc->sct_str);

		mysql_real_query(&sqlDataBase, sqlBuf, nSize);
		cntRes = mysql_store_result(&sqlDataBase);
		datRow = mysql_fetch_row(cntRes);
		pSrc->sct_cnt = atoi((char *)datRow[0]);

		mysql_free_result(cntRes);
	}

	sp_shell_sort(gatShellSort, srcContentContain, nLine);

	for(pSrc = srcContentContain, nCir = 0; nCir < nLine; nCir++, pSrc++)
		printf("%s\n", pSrc->sct_str);

	for(pSrc = srcContentContain, nCir = 0; nCir < nLine; nCir++, pSrc++)
		printf("%d\n", pSrc->sct_cnt);

	mysql_free_result(srcRes);
	free(srcContentContain);

	mysql_close(&sqlDataBase);
	mysql_library_end();

	exit(FUN_RUN_OK);
}


/*-----sct_perror-----*/
static void sct_perror(char *errStr)
{
	printf("SCT---> %s: %s\n", errStr, strerror(errno));
	exit(FUN_RUN_FAIL);
}


/*-----sct_dberr_dispose-----*/
static void sct_dberr_dispose(MYSQL *sqlHand, char *dbName, char *errHead)
{
	printf("SCT---> %s - %s: %s\n", dbName, errHead, (char *)mysql_error(sqlHand));
	exit(FUN_RUN_FAIL);
}


/*------------------------------------------
	Part Five: Src sort

	1. sct_sort_init
	2. sct_sort_mov
	3. sct_sort_assign
	4. sct_sort_progress
	5. sct_sort_cmprep

--------------------------------------------*/

/*-----sct_sort_init-----*/
static void sct_sort_init(void)
{
	if(!(gatShellSort = sp_shell_sort_init((ssmov)sct_sort_mov, (ssass)sct_sort_assign,
	(ssprog)sct_sort_progress, (sscmre)sct_sort_cmprep, (mafun)sct_malloc, free, NULL))) {
		sct_perror("sct_sort_init - sp_shell_sort_init");
		exit(FUN_RUN_FAIL);
	}
}


/*-----sct_sort_mov-----*/
static SCT *sct_sort_mov(SCT *sctHand, SCT *sctData, int nMov)
{
	return	(sctData + nMov);
}


/*-----sct_sort_assign-----*/
static SCT *sct_sort_assign(SCT *pSct)
{
	return	srcContentContain;
}


/*-----sct_sort_progress-----*/
static SCT *sct_sort_progress(SCT *pHandler, SCT *pData)
{
	return	(++pData);
}


/*-----sct_sort_cmprep-----*/
static void sct_sort_cmprep(SCT *pSrc, SCT *pCmp)
{
	SCT	sctSave;

	if(pCmp->sct_cnt > pSrc->sct_cnt) {
		sctSave = *pCmp;
		*pCmp = *pSrc;
		*pSrc = sctSave;
	}
}


/*------------------------------------------
	Part Six: Pack function

	1. sct_malloc

--------------------------------------------*/

/*-----sct_malloc-----*/
static void *sct_malloc(void *pEmpty, int nSize)
{
	return	malloc(nSize);
}
