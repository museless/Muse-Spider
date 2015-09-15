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

	Part Four:	Outbug main
	Part Five:	Initialization
	Part Six:	Outbug extract
	Part Seven:	Outbug message part

--------------------------------------------*/

/*------------------------------------------
	Part Zero: Include
--------------------------------------------*/

#include "spinc.h"
#include "spnet.h"
#include "spdb.h"
#include "spmsg.h"
#include "spframe.h"

#include "mmdpool.h"
#include "mipc.h"
#include "malgo.h"
#include "mpctl.h"

#include "spoutb.h"
#include "spoglobal.h"


/*------------------------------------------
	Part Three: Define
--------------------------------------------*/


/*------------------------------------------
	Part Two: Local data
--------------------------------------------*/

static	MATOS	obSourceLock;


/*------------------------------------------
	Part Three: Local function
--------------------------------------------*/

/* Part Four */
static	void	otbug_command_analysis(int nPara, char **pComm);

/* Part Five */
static	int	mainly_init(void);
static	int	otbug_ipc_init(void);
static	int	otbug_hash_bucket_init(void);
static	int	otbug_mempool_init(void);
static	void	otbug_timbuf_init(char *timStr);
static	int	otbug_synctime_init(void);
static	int	otbug_locker_init(void);
static	void	otbug_final_init(void);

/* Part Six */
static	void	otbug_work_entrance(void);
static	void	otbug_work_body(void *keyRow);
static	void	otbug_work_extract_string(char *keyStr);


/*------------------------------------------
	Part Four: Outbug main

	1. main
	2. otbug_command_analysis

--------------------------------------------*/

/*-----main-----*/
int main(int argc, char *argv[])
{
	otbug_command_analysis(argc, argv);

	if(mainly_init()) {
		if(otbug_mempool_init() && otbug_hash_bucket_init() && otbug_locker_init()) {
			otbug_database_init();
			otbug_final_init();

			otbug_work_entrance();
		}
	}

	mgc_all_clean(obGarCol);

	return	FUN_RUN_END;
}


/*-----otbug_command_analysis-----*/
static void otbug_command_analysis(int nPara, char **pComm)
{
	int	nCir, cOff, tFlags, hFlags;

	initMsgFlags = tFlags = hFlags = 0;

	for(cOff = -1, nCir = 1; nCir < nPara; nCir++) {
		if(!strcmp(pComm[nCir], "--help") || !strcmp(pComm[nCir], "-h")) {
			hFlags = 1; break;

		} else if(!strcmp(pComm[nCir], "-l") || !strcmp(pComm[nCir], "--link")) {
			initMsgFlags = 1;

		} else if(!strcmp(pComm[nCir], "-t")) {
			otbug_timbuf_init(pComm[++nCir]); tFlags = 1;

		} else if(!strcmp(pComm[nCir], "-c")) {
			cOff = ++nCir;

		} else {
			hFlags = 1; break;
		}
	}

	if(mc_conf_load("Outbug", ((cOff == -1) ? "/MuseSp/conf/outbug.cnf" : pComm[cOff])) == FUN_RUN_FAIL) {
		printf("Outbug---> load configure failed\n");
		otbug_perror("ubug_command_analyst - mc_conf_load", errno);
		mc_conf_unload();
		exit(FUN_RUN_FAIL);
	}

	if(hFlags) {
		//otbug_print_help();
		mc_conf_unload();
		exit(FUN_RUN_END);
	}

	if(!tFlags)
		otbug_timbuf_init(NULL);

	return;
}


/*------------------------------------------
	Part Five: Initialization

	1. mainly_init
	2. otbug_ipc_init
	3. otbug_hash_bucket_init
	4. otbug_mempool_init
	5. otbug_timbuf_init
	6. otbug_synctime_init
	7. otbug_locker_init
	8. otbug_final_init

--------------------------------------------*/

/*-----mainly_init-----*/
static int mainly_init(void)
{
	if(!sp_normal_init("Outbug", &obGarCol, (SOSET **)&obMsgSet, otbug_msg_init, "outbug_err_locate", initMsgFlags))
		return	FUN_RUN_END;

	/* lock init */
	mato_init(&obSourceLock, 0);

	/* read pthread limit num */
	if(mc_conf_read("outbug_pthread_num", CONF_NUM, &nOtbugPthread, sizeof(int)) == FUN_RUN_FAIL) {
		mc_conf_print_err("outbug_pthread_num");
		return	FUN_RUN_END;
	}

	/* mgc one init */
	if(mgc_one_init(&otResCol, (gcfun)mysql_free_result, TRY_A_LIMIT) == MGC_FAILED) {
		otbug_perror("mainly_init - mgc_one_init", errno);
		return	FUN_RUN_END;
	}

	if(!otbug_ipc_init() || !otbug_synctime_init())
		return	FUN_RUN_END;

	/* init file control */
	if(!otbug_filectl_init())
		return	FUN_RUN_END;

	/* init pthread pool */
	if(!(obThreadPool = mpc_create(nOtbugPthread))) {
		printf("Outbug---> mainly_init - mpc_create - failed");
		return	FUN_RUN_END;
	}

	if(mgc_add(obGarCol, NULL_POINT, otbug_filectl_destroy) == MGC_FAILED)
		perror("Outbug---> mainly_init - mgc_add - filectl");

	return	otbug_init_signal();
}


/*-----otbug_ipc_init-----*/
static int otbug_ipc_init(void)
{
	char	keyFile[PATH_LEN];
	int	smSize;

	/* shm control init */
	if(mc_conf_read("outbug_semkey_file", CONF_STR, keyFile, PATH_LEN) == FUN_RUN_FAIL) {
		mc_conf_print_err("outbug_semkey_file");
		return	FUN_RUN_END;
	}

	if(mc_conf_read("outbug_sm_size", CONF_NUM, &smSize, sizeof(int)) == FUN_RUN_FAIL) {
		mc_conf_print_err("outbug_sm_size");
		return	FUN_RUN_END;
	}

	if(!(obShmContainer = mshm_create(keyFile, smSize, PROJ_SHM_CTL))) {
		otbug_perror("otbug_ipc_init - mshm_create", errno);
		return	FUN_RUN_END;
	}

	printf("Outbug---> shm index: %d\n", ((MSHM *)obShmContainer)->shm_id);

	if(mgc_add(obGarCol, obShmContainer, (gcfun)mshm_destroy) == MGC_FAILED)
		otbug_perror("otbug_ipc_init - mgc_add - mshm", errno);

	return	FUN_RUN_OK;
}


/*-----otbug_hash_bucket_init-----*/
static int otbug_hash_bucket_init(void)
{
	int	nBucket;

	if(mc_conf_read("outbug_hash_bucket_num", CONF_NUM, &nBucket, sizeof(int)) == FUN_RUN_FAIL) {
		mc_conf_print_err("outbug_hash_bucket_num");
		return	FUN_RUN_END;
	}

	if(!(obHashBucket = hash_bucket_init(nBucket, (mafun)mmdp_malloc, NULL,
	obDynaMp, (hbcreat)otbug_shm_alloc, otbug_shm_cmp, otbug_shm_hash))) {
		otbug_perror("otbug_hash_bucket_init - hash_bucket_init", errno);
		return	FUN_RUN_END;
	}

	if(mgc_add(obGarCol, obHashBucket, (gcfun)hash_bucket_destroy) == MGC_FAILED)
		otbug_perror("otbug_hash_bucket_init - mgc_add - hashbucket", errno);

	return	FUN_RUN_OK;
}


/*-----otbug_mempool_init-----*/
static int otbug_mempool_init(void)
{
	/* obDynaMp's data will save till end */
	if(!(obDynaMp = mmdp_create(OTBUG_MP_HASH))) {
		otbug_perror("otbug_mempool_init - mmdp_create - mp", errno);
		return	FUN_RUN_END;
	}

	if(mgc_add(obGarCol, obDynaMp, (gcfun)mmdp_free_all) == MGC_FAILED)
		otbug_perror("otbug_mempool_init - mgc_add - obDynaMp", errno);

	/*obThreadMp's data will be reset */
	if(!(obThreadMp = mmdp_create(OTBUG_MP_THREAD))) {
		otbug_perror("otbug_mempool_init - mmdp_create - thread", errno);
		return	FUN_RUN_END;
	}

	if(mgc_add(obGarCol, obThreadMp, (gcfun)mmdp_free_all) == MGC_FAILED)
		otbug_perror("otbug_mempool_init - mgc_add - thread", errno);

	return	FUN_RUN_OK;
}


/*-----otbug_timbuf_init-----*/
static void otbug_timbuf_init(char *timStr)
{
	struct	tm	*nowTime;

	nowTime = time_str_extract(timStr);

	sprintf(keysTblName, "K%d%02d%02d", nowTime->tm_year, nowTime->tm_mon, nowTime->tm_mday);
}


/*-----otbug_synctime_init-----*/
static int otbug_synctime_init(void)
{
	/* read sync time */
	if(mc_conf_read("outbug_sync_sec", CONF_NUM, &obSyncSec, sizeof(int)) == FUN_RUN_FAIL) {
		mc_conf_print_err("outbug_sync_sec");
		return	FUN_RUN_END;
	}

	if(mc_conf_read("outbug_sync_microsec", CONF_NUM, &obSyncMicrosec, sizeof(int)) == FUN_RUN_FAIL) {
		mc_conf_print_err("outbug_sync_microsec");
		return	FUN_RUN_END;
	}

	if(obSyncSec + (obSyncMicrosec / MICSEC_PER_SEC) < TAKE_A_SEC) {
		printf("Outbug---> sync time smaller than one sec\n");
		return	FUN_RUN_END;
	}

	if(obSyncSec > TAKE_A_SEC) {
		obSyncSec -= TAKE_A_SEC;
		obSyncMicrosec += MILLISEC_500;

	} else {
		obSyncMicrosec -= MILLISEC_500;
	}

	return	FUN_RUN_OK;
}


/*-----otbug_locker_init-----*/
static int otbug_locker_init(void)
{
	if(!(obShmCtler = otbug_tcler_init((mafun)mshm_malloc, obShmContainer))) {
		otbug_perror("otbug_locker_init - otbug_tcler_init - shm", errno);
		return	FUN_RUN_END;
	}

	if(!(obTimLock = otbug_tcler_init((mafun)mmdp_malloc, obDynaMp))) {
		otbug_perror("otbug_locker_init - otbug_tcler_init - time", errno);
		return	FUN_RUN_END;
	}

	return	FUN_RUN_OK;
}


/*-----otbug_final_init-----*/
static void otbug_final_init(void)
{
	if(mgc_add(obGarCol, NULL_POINT, (gcfun)otbug_unlink_database_serv) == MGC_FAILED)
		perror("Outbug---> otbug_final_init - mgc_add - dbserv");
}


/*------------------------------------------
	Part Six: Outbug extract

	1. otbug_work_entrance
	2. otbug_work_body
	3. otbug_work_extract_string

--------------------------------------------*/

/*-----otbug_work_entrance-----*/
static void otbug_work_entrance(void)
{
	MSLRES	*pResult;
	MSLROW	uRow;
	char	dnSql[SQL_LICOM_LEN];
	int	strLen, nRet;

	strLen = sprintf(dnSql, GET_KEYWD_LIM, keysTblName, nOtbugPthread);

	if((nRet = otbug_filectl_thread_creat())) {
		otbug_perror("outbug_work_entrance - otbug_filectl_thread_creat", nRet);
		return;
	}

	if((nRet = otbug_script_start("outbug_tool_script_loca"))) {
		otbug_perror("outbug_work_entrance - otbug_script_start", nRet);
		return;
	}

	while(FUN_RUN_OK) {
		if(mysql_real_query(&keysDataBase, dnSql, strLen)) {
			otbug_dberr_dispose(&keysDataBase, keysTblName,
			"otbug_work_entrance - mysql_real_query");
			otbug_sig_error();
		}

		pResult = mysql_store_result(&keysDataBase);

		/* checking message */
		if(initMsgFlags) {
			nRet = sp_msgs_frame_run(obMsgSet, pResult);

			if(nRet == FUN_RUN_FAIL || nRet == FUN_RUN_END) {
				sleep(TAKE_A_NOTHING);
				continue;
			}
		}

		if(!pResult) {
			sleep(TAKE_A_EYECLOSE);
			continue;
		}

		mgc_one_add(&otResCol, pResult);
		/* wait all thread ready for using */
		mpc_thread_wait(obThreadPool);

		while((uRow = mysql_fetch_row(pResult))) {
			mato_inc(&obSourceLock);

			if(!mpc_thread_wake(obThreadPool, otbug_work_body, (void *)uRow))
				otbug_sig_error();
		}

		while(!mato_sub_and_test(&obSourceLock, 0))
			;	/* nothing */

		mgc_one_clean(&otResCol);
	}
}


/*-----otbug_work_body-----*/
static void otbug_work_body(void *keyRow)
{
	MSLROW	datRow = (MSLROW)keyRow;

	otbug_work_extract_string(datRow[1]);

	otbug_rewind_state(SET_KEYWD_FLAGS, datRow[0]);
	mato_dec(&obSourceLock);
}


/*-----otbug_work_extract_string-----*/
static void otbug_work_extract_string(char *keyStr)
{
	char	*pChar = keyStr;
	TMCON	*pTmcont;
	int	datSav[1];

	otbug_tcler_lock(obShmCtler);
	otbug_tcler_lock(obTimLock);

	for(; (pChar = strchr(keyStr, ';')); keyStr = pChar + 1) {
		if((pTmcont = hash_bucket_find(obHashBucket, keyStr, pChar - keyStr))) {
			pTmcont->tc_count++;
			continue;
		}

		datSav[0] = otbug_tcler_cnter(obTimLock);

		if(!hash_bucket_insert(obHashBucket, keyStr, pChar - keyStr, datSav))
			otbug_sig_error();
	}

	otbug_tcler_unlock(obTimLock);
	otbug_tcler_unlock(obShmCtler);
}


/*------------------------------------------
	Part Seven: Outbug message part

	1. otbug_time_change
	2. otbug_keep_working

--------------------------------------------*/

/*-----otbug_time_change-----*/
void otbug_time_change(void)
{
	otbug_script_start("outbug_relo_script_loca");
}


/*-----otbug_keep_working-----*/
void otbug_keep_working(void *pResult)
{
	if(pResult && !mysql_num_rows((MSLRES *)pResult))
		sleep(TAKE_A_EYECLOSE);
}

