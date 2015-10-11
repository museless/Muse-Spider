/*------------------------------------------
	Source file content Six part

	Part Zero:	Include
	Part One:	Define
	Part Two:	Local data
	Part Three:	Local function

	Part Four:	Outbug filectl
	Part Five:	Tmpfile control

--------------------------------------------*/

/*------------------------------------------
	Part Zero: Include
--------------------------------------------*/

#include "spinc.h"
#include "spdb.h"

#include "mipc.h"
#include "mmdpool.h"

#include "spoglobal.h"
#include "spoutb.h"


/*------------------------------------------
	Part Two: Local data
--------------------------------------------*/

static	int	logFileFd, keyIndexFd, cntIndexFd, datFileFd, tmpFileFd;
static	int	keyIndOff, cntIndOff, datFileOff;
static	time_t	logTimeStart;

static	TFCTL	tmpFileCtlStru = {PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, 0};
static	PIDSAV	*toolPidSave = NULL;


/*------------------------------------------
	Part Three: Local function
--------------------------------------------*/

/* Part Four */
static	TMCON	*otbug_filectl_index_write(TMCON *pTm, int oldCnt, int newCnt);
static	int	otbug_filectl_data_write(int nCnt);
static	void	otbug_filectl_off_write(int datOff);
static	int	otbug_filectl_write(int nFd, void *pData, int datLen);

static	int	otbug_filectl_open(int *fileDesc, char *filePath, char *fileName, int pathOff, int fileOff);

/* Part Five */
static	void	*otbug_tmpfile_entrance(void *pPara);


/*------------------------------------------
	Part Four: Outbug filectl

	1. otbug_filectl_init
	2. otbug_filectl_entrance
	3. otbug_filectl_index_write
	4. otbug_filectl_data_write
	5. otbug_filectl_off_write
	6. otbug_filectl_write
	7. otbug_filectl_destroy
	8. otbug_filectl_thread_creat
	9. otbug_filectl_open

--------------------------------------------*/

/*-----otbug_filectl_init-----*/

/***********************************
 *  the init function must put
 *  before the otbug_synctime_init
 ***********************************/

int otbug_filectl_init(void)
{
	STAT	statBuf;
	LGDATA	logData;
	char	filePath[PATH_LEN];
	int	nPath, logOff;

	/* Key data file path read */
	if(mc_conf_read("outbug_datsave_locate", CONF_STR, filePath, PATH_LEN) == FUN_RUN_FAIL) {
		mc_conf_print_err("outbug_datsave_locate");
		return	FUN_RUN_END;
	}

	nPath = strlen(filePath);
	nPath += sprintf(filePath + nPath, "%s/", &keysTblName[1]);

	if(mkdir(filePath, FILE_AUTHOR) == FUN_RUN_FAIL) {
		if(errno != EEXIST) {
			otbug_perror("otbug_filectl_init - mkdir", errno);
			return	FUN_RUN_END;
		}
	}

	keyIndOff = cntIndOff = datFileOff = 0;

	/* open log file */
	memset(&logData, 0, LOG_DATA_SIZE);
	sprintf(filePath + nPath, "logFile");

	if((logFileFd = open(filePath, O_CREAT | O_RDWR, FILE_AUTHOR)) == FUN_RUN_FAIL) {
		otbug_perror("otbug_filectl_init - open - log", errno);
		return	FUN_RUN_END;
	}

	if(fstat(logFileFd, &statBuf)) {
		otbug_perror("otbug_filectl_init - fstat", errno);
		return	FUN_RUN_END;
	}

	if(statBuf.st_size > LOG_DATA_SIZE + LOG_HEAD_SEC) {
		logOff = (statBuf.st_size - LOG_DATA_SIZE);

		if(pread(logFileFd, &logData, LOG_DATA_SIZE, logOff) == FUN_RUN_FAIL) {
			otbug_perror("otbug_filectl_init - pread - logdata", errno);
			return	FUN_RUN_END;
		}

		if(logData.ld_check != LOG_CHECK_DATA) {
			printf("Otbug---> log file was ruined\n");
			return	FUN_RUN_END;
		}

		if(pread(logFileFd, &obSyncMicrosec, LOG_HEAD_SEC, 0) == FUN_RUN_FAIL) {
			otbug_perror("otbug_filectl_init - pread - usec", errno);
			return	FUN_RUN_END;
		}

	} else {
		obSyncMicrosec += (obSyncSec * MICSEC_PER_SEC);

		if(write(logFileFd, &obSyncMicrosec, LOG_HEAD_SEC) == FUN_RUN_FAIL) {
			otbug_perror("otbug_filectl_init - write", errno);
			return	FUN_RUN_END;
		}
	}

	obSyncSec = 0;

	keyIndOff = logData.ld_keyoff;
	cntIndOff = logData.ld_cntoff;
	datFileOff = logData.ld_datoff;

	/********************************************
	 * open keyIndex, cntIndex, dataFile, tmpFile
	 * tmpFile for otbug to read the tool's pid
	 *********************************************/
	if(!otbug_filectl_open(&keyIndexFd, filePath, "keyIndex", nPath, keyIndOff) ||
	   !otbug_filectl_open(&cntIndexFd, filePath, "cntIndex", nPath, cntIndOff) ||
	   !otbug_filectl_open(&datFileFd, filePath, "dataFile", nPath, datFileOff))
		return	FUN_RUN_END;

	sprintf(filePath + nPath, "tmpFile");

	if((tmpFileFd = open(filePath, O_RDWR | O_CREAT | O_TRUNC)) == FUN_RUN_FAIL) {
		otbug_perror("otbug_filectl_init - open - tmpFile", errno);
		return	FUN_RUN_END;
	}

	return	FUN_RUN_OK;
}


/*-----otbug_filectl_entrance-----*/
void *otbug_filectl_entrance(void *toPara)
{
	TMCON	*idxShm;
	int	datOff;
	int	lastCnt, newCnt;
	TMVAL	seleTm;

	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

	idxShm = otbug_shm_data_begin(obShmContainer);
	logTimeStart = time(NULL);

	lastCnt = newCnt = 0;
	datOff = datFileOff;

	while(FUN_RUN_OK) {
		seleTm.tv_sec = obSyncSec;
		seleTm.tv_usec = obSyncMicrosec;

		if(select(0, NULL, NULL, NULL, &seleTm) == FUN_RUN_FAIL) {
			if(errno == EINTR)
				continue;

			otbug_sig_error();
		}

		otbug_tcler_lock(obShmCtler);
		newCnt = otbug_tcler_cnter(obShmCtler);

		/* wake up the tmpFile ctl thread */
		pthread_mutex_lock(&tmpFileCtlStru.tf_mutex);

		tmpFileCtlStru.tf_flags = 1;
		pthread_cond_signal(&tmpFileCtlStru.tf_cond);

		pthread_mutex_unlock(&tmpFileCtlStru.tf_mutex);

		/* start to write three file */
		if(lastCnt < newCnt) {
			idxShm = otbug_filectl_index_write(idxShm, lastCnt, newCnt);
			lastCnt = newCnt;
		}

		otbug_filectl_off_write(datOff);
		datOff += otbug_filectl_data_write(newCnt);
		datFileOff += datOff;

		otbug_tcler_inc_cnt(obTimLock);
		otbug_tcler_unlock(obShmCtler);
	}
}


/*-----otbug_filectl_index_write-----*/
static TMCON *otbug_filectl_index_write(TMCON *pTm, int oldCnt, int newCnt)
{
	char	*idxFile, *idxMov;
	int	idxLimit;

	idxLimit = (newCnt - oldCnt) * TC_PERWR_LEN;

	if((idxFile = idxMov = mmdp_malloc(obThreadMp, idxLimit)) == NULL) {
		elog_write("otbug_filectl_index_write - mmdp_malloc - index", FUNCTION_STR, ERROR_STR);
		otbug_sig_error();
	}

	for(; oldCnt < newCnt; oldCnt++) {
		if(idxMov - idxFile > idxLimit) {
			otbug_filectl_write(keyIndexFd, idxFile, idxMov - idxFile);
			idxMov = idxFile;
		}

		idxMov += sprintf(idxMov, "%s %*d\n", ((uChar *)pTm) + TC_FIG_LEN,
		TC_CNT_STRLEN, otbug_tcler_cnter(obTimLock));

		pTm = otbug_shm_mov_next(pTm);
	}

	keyIndOff += (idxMov - idxFile);

	otbug_filectl_write(keyIndexFd, idxFile, idxMov - idxFile);
	mmdp_reset_default(obThreadMp);
	
	return	pTm;
}


/*-----otbug_filectl_data_write-----*/
static int otbug_filectl_data_write(int nCnt)
{
	int	*datFile, *datMov;
	TMCON	*datShm;
	int	datLimit, wrLen;

	datLimit = (nCnt + 1) * sizeof(int);
	datShm = otbug_shm_data_begin(obShmContainer);

	if((datFile = datMov = mmdp_malloc(obThreadMp, datLimit)) == NULL) {
		elog_write("otbug_filectl_data_write - mmdp_malloc - data", FUNCTION_STR, ERROR_STR);
		otbug_sig_error();
	}

	*datMov = nCnt;

	for(wrLen = 0, datMov++; datMov < datFile + nCnt; datMov++) {
		if(datMov - datFile > datLimit) {
			wrLen += otbug_filectl_write(datFileFd, datFile, (char *)datMov - (char *)datFile);
			datMov = datFile;
		}

		*datMov = datShm->tc_count;
		datShm = otbug_shm_mov_next(datShm);
	}

	wrLen += otbug_filectl_write(datFileFd, datFile, (char *)datMov - (char *)datFile);
	mmdp_reset_default(obThreadMp);

	return	wrLen;
}


/*-----otbug_filectl_off_write-----*/
static void otbug_filectl_off_write(int datOff)
{
	char	offBuff[TC_CNT_STRLEN + TC_NULL_LEN + TC_ENTER_LEN];

	cntIndOff += otbug_filectl_write(cntIndexFd, offBuff, sprintf(offBuff, "%*d\n", TC_CNT_STRLEN, datOff));
}


/*-----otbug_filectl_write-----*/
static int otbug_filectl_write(int nFd, void *pData, int datLen)
{
	int	writeLen;

	if((writeLen = writen(nFd, pData, datLen)) == FUN_RUN_FAIL) {
		elog_write("otbug_filectl_write - writen", FUNCTION_STR, ERROR_STR);
		otbug_sig_error();
	}

	return	writeLen;
}


/*-----otbug_filectl_destroy-----*/
void otbug_filectl_destroy(void *fileDes)
{
	LGDATA	logData;

	logData.ld_tend = time(NULL);
	logData.ld_tstart = logTimeStart;

	logData.ld_keyoff = keyIndOff;
	logData.ld_cntoff = cntIndOff;
	logData.ld_datoff = datFileOff;
	logData.ld_check = LOG_CHECK_DATA;

	if(write(logFileFd, &logData, LOG_DATA_SIZE) == FUN_RUN_FAIL)
		otbug_perror("otbug_file_destroy - write", errno);

	close(tmpFileFd);
	close(logFileFd);
	close(keyIndexFd);
	close(cntIndexFd);
	close(datFileFd);
}


/*-----otbug_filectl_thread_creat-----*/
int otbug_filectl_thread_creat(void)
{
	int	tStatus;

	if((tStatus = pthread_create(&fcThreadTid, NULL, otbug_filectl_entrance, NULL)) ||
	   (tStatus = pthread_create(&tmpFileTid, NULL, otbug_tmpfile_entrance, NULL)))
		return	tStatus;

	return	FUN_RUN_END;
}


/*-----otbug_filectl_thread_destroy-----*/
void otbug_filectl_thread_destroy(void)
{
	PIDSAV	*pSaver;

	otbug_tcler_lock(obShmCtler);

	if(fcThreadTid) {
		pthread_cancel(fcThreadTid);
		pthread_join(fcThreadTid, NULL);
	}

	otbug_tmpfile_lock(obShmCtler);

	for(pSaver = toolPidSave; pSaver; pSaver = pSaver->ps_next)
		kill(pSaver->ps_pid, SIGINT);

	if(tmpFileTid) {
		pthread_cancel(tmpFileTid);
		pthread_join(tmpFileTid, NULL);
	}

	pthread_mutex_unlock(&tmpFileCtlStru.tf_mutex);
}


/*-----otbug_filectl_open-----*/
static int otbug_filectl_open(int *fileDesc, char *filePath, char *fileName, int pathOff, int fileOff)
{
	sprintf(filePath + pathOff, fileName);

	if((*fileDesc = open(filePath, O_CREAT | O_RDWR, FILE_AUTHOR)) == FUN_RUN_FAIL) {
		printf("Outbug---> otbug_filectl_open: %s - %s\n", ERROR_STR, filePath);
		return	FUN_RUN_END;
	}

	if(fileOff) {
		if(lseek(*fileDesc, fileOff, SEEK_SET) == FUN_RUN_FAIL) {
			otbug_perror("otbug_filectl_open - lseek", errno);
			return	FUN_RUN_END;
		}
	}

	return	FUN_RUN_OK;
}


/*------------------------------------------
	Part Five: Tmpfile control

	1. otbug_tmpfile_entrance

--------------------------------------------*/

/*-----otbug_tmpfile_entrance-----*/
static void *otbug_tmpfile_entrance(void *pPara)
{
	PIDSAV	*pSaver;
	pid_t	readPid;
	int	tStatus;

	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

	while(FUN_RUN_OK) {
		if((tStatus = pthread_mutex_lock(&tmpFileCtlStru.tf_mutex))) {
			elog_write("otbug_tmpfile_entrance - pth_mutex_lock", THREAD_STR, strerror(tStatus));
			otbug_sig_error();
		}

		while(!tmpFileCtlStru.tf_flags) {
			if((tStatus = pthread_cond_wait(&tmpFileCtlStru.tf_cond, &tmpFileCtlStru.tf_mutex))) {
				elog_write("otbug_tmpfile_entrance - pth_cond_wait", THREAD_STR, strerror(tStatus));
				otbug_sig_error();
			}
		}

		otbug_tcler_inc(obShmCtler);
		otbug_tmpfile_lock(obShmCtler);

		while(read(tmpFileFd, &readPid, sizeof(pid_t)) == sizeof(pid_t)) {
			if(!(pSaver = mmdp_malloc(obDynaMp, sizeof(PIDSAV)))) {
				elog_write("otbug_tmpfile_entrance - mmdp_malloc", THREAD_STR, ERROR_STR);
				otbug_sig_error();
			}

			pSaver->ps_pid = readPid;
			pSaver->ps_next = toolPidSave;
			toolPidSave = pSaver;
		}

		for(pSaver = toolPidSave; pSaver; pSaver = pSaver->ps_next)
			kill(pSaver->ps_pid, SIGUSR1);

		tmpFileCtlStru.tf_flags = 0;

		otbug_tmpfile_unlock(obShmCtler);
		otbug_tcler_dec(obShmCtler);

		if((tStatus = pthread_mutex_unlock(&tmpFileCtlStru.tf_mutex))) {
			elog_write("otbug_tmpfile_entrance - pth_mutex_unlock", THREAD_STR, strerror(tStatus));
			otbug_sig_error();
		}
	}

	return	NULL;
}
