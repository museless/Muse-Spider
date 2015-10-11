#include "spinc.h"
#include "spmpool.h"
#include "spnet.h"
#include <time.h>


/* locate data */
static	MATOS	writeDataLock;
static	MATOS	syncForceLock;
static	BUFF	logBufStru;
static	int	errLogFd;


/*------------------------------------------
	Part Zero: ErrLog control

	1. elog_init
	2. elog_write
	3. elog_write_force
	4. elog_destroy

--------------------------------------------*/

/*-----elog_init-----*/
int elog_init(char *confStr)
{
	char	telPath[PATH_LEN];

	if(mc_conf_read(confStr, CONF_STR, telPath, PATH_LEN) == FUN_RUN_FAIL) {
		mc_conf_print_err(confStr);
		return	FUN_RUN_FAIL;
	}

	if((errLogFd = open(telPath, O_CREAT | O_RDWR | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP)) == FUN_RUN_FAIL) {
		perror("elog_init - open - TetErrLog");
		return	FUN_RUN_FAIL;
	}

	if((logBufStru.b_start = malloc(LOGBUF_LEN)) == NULL) {
		perror("elog_init - malloc");
		close(errLogFd);
		return	FUN_RUN_FAIL;
	}

	logBufStru.b_cap = LOGBUF_LEN;
	logBufStru.b_size = 0;

	mato_init(&writeDataLock, 1);
	mato_init(&syncForceLock, 1);

	return	FUN_RUN_OK;
}


/*-----elog_write-----*/
int elog_write(char *errStr, char *objStr1, char *objStr2)
{
	struct	tm	*tmStru;
	time_t		tType;

	if(strlen(errStr) + strlen(objStr1) + strlen(objStr2) + LOGBASE_LEN > logBufStru.b_cap)
		return	FUN_RUN_FAIL;

	tType = time(NULL);
	tmStru = localtime(&tType);

	while(FUN_RUN_OK) {
		if(mato_dec_and_test(&writeDataLock)) {
			if(!buff_size_enough(&logBufStru, strlen(errStr) + strlen(objStr1) +
			strlen(objStr2) + logBufStru.b_size + LOGBASE_LEN))
				elog_write_force();

			buff_size_add(&logBufStru, sprintf(logBufStru.b_start + logBufStru.b_size, 
			"%d%02d%02d %02d:%02d:%02d  %s -> \"%s\" - \"%s\"\n",
			tmStru->tm_year + 1900, tmStru->tm_mon + 1, tmStru->tm_mday,
			tmStru->tm_hour, tmStru->tm_min, tmStru->tm_sec, errStr, objStr1, objStr2));

			mato_inc(&writeDataLock);
			return	FUN_RUN_OK;
		}

		mato_inc(&writeDataLock);
	}
}


/*-----elog_write_force-----*/
void elog_write_force(void)
{
	while(FUN_RUN_OK) {
		if(mato_dec_and_test(&syncForceLock)) {
			if(logBufStru.b_size) {
				if(writen(errLogFd, logBufStru.b_start, logBufStru.b_size) == FUN_RUN_FAIL) {
					perror("elog_write_force - writen");
					printf("%.*s\n", logBufStru.b_size, (char *)logBufStru.b_start);
				}

				logBufStru.b_size = 0;
			}

			mato_inc(&syncForceLock);
			return;
		}

		mato_inc(&syncForceLock);
	}
}


/*-----elog_destroy-----*/
void elog_destroy(void)
{
	elog_write_force();

	while(FUN_RUN_OK) {
		if(mato_dec_and_test(&writeDataLock)) {
			close(errLogFd);
			free(logBufStru.b_start);
			return;
		}

		mato_inc(&writeDataLock);
	}
}

