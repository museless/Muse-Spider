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
	Source file content Seven part

	Part Zero:	Include
	Part One:	Define
	Part Two:	Local data
	Part Three:	Local function

	Part Four:	Outbug filectl
	Part Five:	Tmpfile control
	Part Six:	Tool control

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

static	TFCTL	tmpFileCtler = {PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, CTLER_SLEEP};
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
static	int	otbug_tmpfile_wait(void);
static	void	otbug_tmpfile_wake(void);
static	void	otbug_tmpfile_thread_kill(void);


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
		if(tmpFileTid) {
			if(!otbug_tmpfile_wait()) {
				printf("Outbug---> tmp file pthread was killed for no reason\n");
				otbug_tmpfile_thread_kill();

			} else {
				tmpFileCtler.tf_flags = CTLER_WAKE;
				pthread_cond_signal(&tmpFileCtler.tf_cond);
	
				otbug_tmpfile_wake();
			}
		}

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
	otbug_tcler_lock(obShmCtler);

	if(fcThreadTid) {
		pthread_cancel(fcThreadTid);
		pthread_join(fcThreadTid, NULL);
	}

	fcThreadTid = (pth_t)0;

	otbug_tmpfile_thread_kill();
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
	2. otbug_tmpfile_wait
	3. otbug_tmpfile_wake
	4. otbug_tmpfile_thread_kill

--------------------------------------------*/

/*-----otbug_tmpfile_entrance-----*/
static void *otbug_tmpfile_entrance(void *pPara)
{
	PIDSAV	*pSaver;
	pid_t	readPid;
	int	tStatus;

	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

	while(FUN_RUN_OK) {
		if((tStatus = pthread_mutex_lock(&tmpFileCtler.tf_mutex))) {
			elog_write("otbug_tmpfile_entrance - pth_mutex_lock", THREAD_STR, strerror(tStatus));
			otbug_sig_error();
		}

		tmpFileCtler.tf_flags = CTLER_SLEEP;
		pthread_cond_signal(&tmpFileCtler.tf_cond);

		while(!tmpFileCtler.tf_flags) {
			if((tStatus = pthread_cond_wait(&tmpFileCtler.tf_cond, &tmpFileCtler.tf_mutex))) {
				elog_write("otbug_tmpfile_entrance - pth_cond_wait", THREAD_STR, strerror(tStatus));
				otbug_sig_error();
			}
		}

		if(tmpFileCtler.tf_flags == CTLER_QUIT) {
			pthread_mutex_unlock(&tmpFileCtler.tf_mutex);
			break;
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

		otbug_tools_sendsig(SIGUSR1);

		otbug_tmpfile_unlock(obShmCtler);
		otbug_tcler_dec(obShmCtler);

		if((tStatus = pthread_mutex_unlock(&tmpFileCtler.tf_mutex))) {
			elog_write("otbug_tmpfile_entrance - pth_mutex_unlock", THREAD_STR, strerror(tStatus));
			otbug_sig_error();
		}
	}

	return	NULL;
}


/*-----otbug_tmpfile_wait-----*/
static int otbug_tmpfile_wait(void)
{
	int	tStatus;

	if(pthread_mutex_trylock(&tmpFileCtler.tf_mutex)) {
		sleep(TAKE_A_NOTHING);

		if(pthread_mutex_trylock(&tmpFileCtler.tf_mutex)) {
			pthread_mutex_unlock(&tmpFileCtler.tf_mutex);
			return	FUN_RUN_END;
		}
	}

	while(tmpFileCtler.tf_flags != CTLER_SLEEP) {
		if((tStatus = pthread_cond_wait(&tmpFileCtler.tf_cond, &tmpFileCtler.tf_mutex))) {
			elog_write("otbug_tmpfile_entrance - pth_cond_wait", THREAD_STR, strerror(tStatus));
			return	FUN_RUN_END;
		}
	}

	return	FUN_RUN_OK;
}


/*-----otbug_tmpfile_wake-----*/
static void otbug_tmpfile_wake(void)
{
	pthread_mutex_unlock(&tmpFileCtler.tf_mutex);
}


/*-----otbug_tmpfile_thread_kill-----*/
static void otbug_tmpfile_thread_kill(void)
{
	otbug_tmpfile_lock(obShmCtler);

	/* kill all tools */
	otbug_tools_sendsig(SIGINT);

	/* kill the tmpfile control pthread */
	if(tmpFileTid) {
		if(otbug_tmpfile_wait()) {
			tmpFileCtler.tf_flags = CTLER_QUIT;
			pthread_cond_signal(&tmpFileCtler.tf_cond);
		
			otbug_tmpfile_wake();
		}

		if(pthread_tryjoin_np(tmpFileTid, NULL)) {
			pthread_cancel(tmpFileTid);
			pthread_join(tmpFileTid, NULL);
		}
	}

	tmpFileTid = (pth_t)0;

	otbug_tmpfile_unlock(obShmCtler);
}


/*------------------------------------------
	Part Seven: Tool control

	1. otbug_tools_sendsig

--------------------------------------------*/

/*-----otbug_tools_sendsig-----*/
void otbug_tools_sendsig(int nSig)
{
	PIDSAV	*pSaver;

	for(pSaver = toolPidSave; pSaver; pSaver = pSaver->ps_next)
		kill(pSaver->ps_pid, nSig);
}
