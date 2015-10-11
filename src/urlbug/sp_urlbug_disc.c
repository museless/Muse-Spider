#include "spinc.h"
#include "sphtml.h"
#include "spnet.h"
#include "spdb.h"
#include "spmpool.h"
#include "spmsg.h"

#include "spurlb.h"
#include "spuglobal.h"


/* local data */
static	int	datFileDes;


/*------------------------------------------
	Part Zero: Print Help tip

	1. ubug_print_help

--------------------------------------------*/

/*-----ubug_print_help-----*/
void ubug_print_help(void)
{
	struct	stat	stBuf;

	char	*hpCont;
	int	hpFd;

	if((hpFd = open("/MuseSp/src/urlbug/urlbug.hlp", O_RDWR)) == FUN_RUN_FAIL) {
		perror("ubug_print_help - open");
		exit(FUN_RUN_FAIL);
	}

	if(fstat(hpFd, &stBuf) == FUN_RUN_FAIL) {
		perror("ubug_print_help - fstat");
		exit(FUN_RUN_FAIL);	
	}

	if((hpCont = malloc(stBuf.st_size + 1)) == NULL) {
		perror("ubug_print_help - malloc");
		exit(FUN_RUN_FAIL);
	}

	hpCont[stBuf.st_size] = 0;
	if(readn(hpFd, hpCont, stBuf.st_size) == FUN_RUN_FAIL) {
		perror("ubug_print_help - readn");
		exit(FUN_RUN_FAIL);
	}

	printf("%s\n", hpCont);
	
	free(hpCont);
	close(hpFd);
}


/*------------------------------------------
	Part One: Disc module

	1. ubug_disc_init
	2. ubug_tran_disc
	3. ubug_tran_disc_force
	4. ubug_tran_disc_real
	5. ubug_tran_disc_clean

--------------------------------------------*/

/*-----ubug_disc_init-----*/
void ubug_disc_init(void)
{
	char	datFile[PATH_LEN];

	if(mc_conf_read("default_tmpfile_locate", CONF_STR, datFile, PATH_LEN) == FUN_RUN_FAIL) {
		printf("urlbug - configure file setting wrong - default_tmpfile_locate\n");
		mc_conf_unload();
		exit(FUN_RUN_FAIL);
	}

	if((datFileDes = open(datFile, O_RDWR | O_TRUNC | O_CREAT)) == FUN_RUN_FAIL) {
		ubug_perror("ubug_disc_init - open", errno);
		mc_conf_unload();
		exit(FUN_RUN_FAIL);
	}
}


/*-----ubug_tran_disc-----*/
void ubug_tran_disc(void *pInfo, char *pUrl, int uLen)
{
        WEBIN   *wInfo = (WEBIN *)pInfo;
        
	if(!buff_size_enough(urlBufStu, URL_LEN))
                ubug_tran_disc_force(wInfo->w_buff);

	buff_size_add(wInfo->w_buff, sprintf(buff_place_end(wInfo->w_buff), 
			"%.*s, %d\n", uLen, pUrl, wInfo->w_pattern));
}


/*-----ubug_tran_disc_whole-----*/
void ubug_tran_disc_whole(void)
{
        WEBIN   *pInfo;
        
        for (pInfo = urlSaveList; pInfo; pInfo = pInfo->w_next)
                ubug_tran_disc_force(pInfo->w_buff);       
}


/*-----ubug_tran_disc_force-----*/
void ubug_tran_disc_force(BUFF *pBuff)
{
	while(!mato_dec_and_test(&writeStoreLock))
		mato_inc(&writeStoreLock);

	if(!buff_stru_empty(pBuff)) {
		if(ubug_tran_disc_real(pBuff) == FUN_RUN_FAIL) {
			elog_write("ubug_tran_disc_force - ubug_tran_disc_real", FUNCTION_STR, ERROR_STR);
			ubug_sig_error();
		}
	
		buff_stru_make_empty(pBuff);
	}

	mato_inc(&writeStoreLock);
}


/*-----ubug_tran_disc_real-----*/
int ubug_tran_disc_real(BUFF *pBuff)
{
	return	(buff_check_exist(pBuff)) ? writen(datFileDes, buff_place_start(pBuff), 
		buff_now_size(pBuff)) : FUN_RUN_FAIL;
}


/*-----ubug_tran_disc_clean-----*/
void ubug_tran_disc_clean(void)
{
	
}

