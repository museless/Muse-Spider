/*------------------------------------------
	Otbug keyword sort

	Source file content Ten part

	Part Zero:	Include
	Part One:	Define
	Part Two:	Local data
	Part Three:	Local function

	Part Four:	Main
	Part Five:	Signal operate
	Part Six:	Plugins init
	Part Seven:	Pthread ctl
	Part Eight:	OKSSDT ctl
	Part Nine:	Error dealing

--------------------------------------------*/

/*------------------------------------------
	Part Zero: Include
--------------------------------------------*/

#include "spotool.h"

#include "malgo.h"
#include "spextb.h"


/*------------------------------------------
	Part One: Define
--------------------------------------------*/

/* Define */
#define	OPS_UNDONE		0x0
#define	OPS_DONE		0x1
#define	OPS_KILLED		0x2

#define	NTERMS_LEN_MAX		0xA

#define	SDT_PER_OKSHANDLER	0x200

/* typedef */
typedef	struct	oks_pctl	OPCTL;

typedef	struct	oks_sdt_handler	OSH;
typedef	struct	oks_sh_conter	OSHC;

/* struct */
struct	oks_pctl {
	pmut_t	op_mutex;
	pcd_t	op_cond;

	int	op_flags;
};

struct	oks_sdt_handler {
	OSH	*oh_next;
	SDT	*oh_sdt[SDT_PER_OKSHANDLER];

	int	oh_left;
};

struct	oks_sh_conter {
	OSH	*oc_start, *oc_last;
	SDT	**oc_psdt;

	int	oc_noh;
	int	oc_tsdt;
};


/*------------------------------------------
	Part Two: Local data
--------------------------------------------*/

static	MGCH	*oksGarCol;
static	MSHM	*oksShmReader;
static	DMPH	*oksMemPool;
static	BUFF	*oksTermBuf;

static	SSORTS	*oksShellSort;
static	OSHC	*oksSdtContainer;

static	char	curRunPath[PATH_LEN];

static	int	rankFileDesc, nTopTerms;

/* pthread */
static	pth_t	oksThreadId;
static	OPCTL	oksThreadCtl = {PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, OPS_UNDONE};


/*------------------------------------------
	Part Three: Local function
--------------------------------------------*/

/* Part Five */
static	void	outbug_ks_signal_handler(int nSign);

/* Part Six */
static	int	mainly_init(void);
static	int	otbug_ks_mempool_init(void);
static	int	otbug_ks_buff_init(void);

/* Part Seven */
static	void	*otbug_ks_thread_entrance(void *pPara);

/* Part Eight */
static	SDT	**otbug_ks_sdt_mov(OSH *pOsh, SDT **pSdt, int nMov);
static	SDT	**otbug_ks_sdt_assign(OSH *pOsh);
static	SDT	**otbug_ks_sdt_progress(OSH **pOshp, SDT **pSdt);
static	void	otbug_ks_sdt_cmprep(SDT **pSdt, SDT **pCmp);

/* Part Nine */
static	void	otbug_ks_error_deal(char *errStr, int nError);


/*------------------------------------------
	Part Four: Main

	1. main

--------------------------------------------*/

/*-----main------*/
int main(void)
{
	int	tStatus;

	if(mainly_init() && otbug_ks_mempool_init() && otbug_ks_buff_init() && 
	otbug_tool_signal_init(outbug_ks_signal_handler, otbug_ks_error_deal)) {
		if(!(tStatus = pthread_create(&oksThreadId, NULL, otbug_ks_thread_entrance, NULL)))
			sp_stop();

		otbug_ks_error_deal("main - pthread_create", tStatus);
	}

	mgc_all_clean(oksGarCol);

	return	FUN_RUN_END;
}


/*------------------------------------------
	Part Five: Signal operate

	1. outbug_ks_signal_handler

--------------------------------------------*/

/*-----outbug_ks_signal_handler-----*/
static void outbug_ks_signal_handler(int nSign)
{
	if(nSign == SIGINT) {
		printf("OKS---> catch a SIGINT\n");

		if(pthread_mutex_trylock(&oksThreadCtl.op_mutex)) {
			/* if locked by pthread */
			sleep(TAKE_A_SEC);
			pthread_mutex_unlock(&oksThreadCtl.op_mutex);
			pthread_cancel(oksThreadId);
			pthread_join(oksThreadId, NULL);

		} else {
			oksThreadCtl.op_flags = OPS_KILLED;
			pthread_cond_signal(&oksThreadCtl.op_cond);
			pthread_mutex_unlock(&oksThreadCtl.op_mutex);
			sleep(TAKE_A_SEC);
		}

	} else if(nSign == SIGUSR1) {
		pthread_mutex_lock(&oksThreadCtl.op_mutex);

		oksThreadCtl.op_flags = OPS_DONE;
		pthread_cond_signal(&oksThreadCtl.op_cond);

		pthread_mutex_unlock(&oksThreadCtl.op_mutex);

		return;
	
	} else if(nSign == SIGUSR2) {
		printf("OKS---> pthread was killed by himself\n");

	} else if(nSign == SIGSEGV) {
		printf("OKS---> catch a SIGSEGV\n");
		kill(getpid(), SIGINT);

	} else {
		printf("OKS---> unknown signal: %d\n", nSign);
	}

	mgc_all_clean(oksGarCol);
	exit(FUN_RUN_FAIL);
}


/*------------------------------------------
	Part Six: Plugins init

	1. mainly_init
	2. otbug_ks_mempool_init
	3. otbug_ks_buff_init

--------------------------------------------*/

/*-----mainly_init-----*/
static int mainly_init(void)
{
	/* tool frame */
	if(!otbug_tool_frame_init("OKS", &oksGarCol, &oksShmReader, otbug_ks_error_deal))
		return	FUN_RUN_END;

	/* read top num */
	if(mc_conf_read("oks_rank_top_num", CONF_NUM, &nTopTerms, sizeof(int)) == FUN_RUN_FAIL) {
		mc_conf_print_err("oks_rank_top_num");
		return	FUN_RUN_END;
	}

	/* mgc one init */
	if(mgc_one_init(&otResCol, (gcfun)mysql_free_result, TRY_A_LIMIT) == MGC_FAILED) {
		perror("Outbug---> mainly_init - mgc_one_init");
		return	FUN_RUN_END;
	}

	return	FUN_RUN_OK;
}


/*-----otbug_ks_mempool_init-----*/
static int otbug_ks_mempool_init(void)
{
	/* memory pool init */
	if(!(oksMemPool = mmdp_create(OTBUG_MP_HASH))) {
		otbug_ks_error_deal("otbug_ks_mempool_init - mmdp_create", errno);
		return	FUN_RUN_END;
	}

	/* malloc OSHC */
	if(!(oksSdtContainer = mmdp_malloc(oksMemPool, sizeof(OSHC)))) {
		otbug_ks_error_deal("otbug_ks_mempool_init - mmdp_malloc - oshc", errno);
		return	FUN_RUN_END;
	}

	oksSdtContainer->oc_noh = 1;
	oksSdtContainer->oc_tsdt = 0;

	if(!(oksSdtContainer->oc_last = oksSdtContainer->oc_start = mmdp_malloc(oksMemPool, sizeof(OSH)))) {
		otbug_ks_error_deal("otbug_ks_mempool_init - mmdp_malloc - osh", errno);
		return	FUN_RUN_END;
	}

	oksSdtContainer->oc_psdt = &oksSdtContainer->oc_start->oh_sdt[0];
	oksSdtContainer->oc_start->oh_next = NULL;
	oksSdtContainer->oc_start->oh_left = SDT_PER_OKSHANDLER;

	/* shell sort mode init */
	if(!(oksShellSort = sp_shell_sort_init((ssmov)otbug_ks_sdt_mov, (ssass)otbug_ks_sdt_assign,
	(ssprog)otbug_ks_sdt_progress, (sscmre)otbug_ks_sdt_cmprep, (mafun)mmdp_malloc, NULL, oksMemPool))) {
		otbug_ks_error_deal("otbug_ks_mempool_init - shell_sort_init", errno);
		return	FUN_RUN_END;
	}

	if(mgc_add(oksGarCol, oksShellSort, (gcfun)sp_shell_sort_destroy) == MGC_FAILED)
		otbug_ks_error_deal("otbug_ks_mempool_init - mgc_add - ssort", errno);

	if(mgc_add(oksGarCol, oksMemPool, (gcfun)mmdp_free_all) == MGC_FAILED)
		otbug_ks_error_deal("otbug_ks_mempool_init - mgc_add - oksMemPool", errno);

	return	FUN_RUN_OK;
}


/*-----otbug_ks_buff_init-----*/
static int otbug_ks_buff_init(void)
{
	/**************************************
	 * 1. BYTE_CMP_MAX at src/extbug/spextb.h
	 * 2. 3 means two space and one enter
	 * 3. the last (NTERMS_LEN_MAX + 1) means
	 *   one num cnt and one enter at file begin
	 **************************************/

	if(!(oksTermBuf = buff_stru_init(((BYTE_CMP_MAX + NTERMS_LEN_MAX + 3 + sizeof(int)) * nTopTerms) + 
	(NTERMS_LEN_MAX + 1)))) {
		otbug_ks_error_deal("otbug_ks_buff_init - buff_stru_init", errno);
		return	FUN_RUN_END;
	}

	if(mgc_add(oksGarCol, oksTermBuf, (gcfun)buff_stru_free_all) == MGC_FAILED)
		otbug_ks_error_deal("otbug_ks_buff_init - mgc_add - oksTermBuf", errno);

	return	FUN_RUN_OK;
}


/*------------------------------------------
	Part Seven: Pthread ctl

	1. otbug_ks_thread_entrance

--------------------------------------------*/

/*-----otbug_ks_thread_entrance-----*/
static void *otbug_ks_thread_entrance(void *pPara)
{
	TLCTL	*tmCtl;
	OSH	*pHandler;
	SDT	*pSdt, *sdtMov;
	int	newCnt, oldCnt, nCir, nLoop;
	int	thStatus;

	newCnt = oldCnt = 0;
	tmCtl = otbug_shm_tcler_get(oksShmReader);
	sdtMov = (SDT *)otbug_shm_data_begin(oksShmReader);

	if(!otbug_tool_signup(curRunPath, tmCtl, otbug_ks_error_deal))
		kill(getpid(), SIGUSR2);

	/* open ranking file and sign to garbage collector */
	sprintf(curRunPath + strlen(curRunPath), "rankFile");

	if((rankFileDesc = open(curRunPath, O_CREAT | O_RDWR | O_TRUNC)) == FUN_RET_NEG) {
		otbug_ks_error_deal("otbug_ks_thread_entrance - open", errno);
		kill(getpid(), SIGUSR2);
	}

	if(mgc_add(oksGarCol, ((void *)&rankFileDesc), (gcfun)close) == MGC_FAILED)
		otbug_ks_error_deal("otbug_ks_thread_entrance - mgc_add - rankFileDesc", errno);

	/* start running */
	if((thStatus = pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL))) {
		otbug_ks_error_deal("otbug_ks_thread_entrance - pthread_setcanceltype", thStatus);
		kill(getpid(), SIGUSR2);
	}

	while(FUN_RUN_OK) {
		pthread_mutex_lock(&oksThreadCtl.op_mutex);

		while(oksThreadCtl.op_flags != OPS_DONE) {
			if(oksThreadCtl.op_flags == OPS_KILLED) {
				pthread_mutex_unlock(&oksThreadCtl.op_mutex);
				pthread_detach(pthread_self());

				pthread_exit(NULL);
			}

			pthread_cond_wait(&oksThreadCtl.op_cond, &oksThreadCtl.op_mutex);
		}

		otbug_tcler_inc(tmCtl);

		if((newCnt = otbug_tcler_cnter(tmCtl)) > oldCnt) {
			pHandler = oksSdtContainer->oc_last;

			for(; oldCnt < newCnt; oldCnt++, sdtMov = otbug_shm_data_fetch(sdtMov)) {
				if(!pHandler->oh_left) {
					if(!(pHandler = pHandler->oh_next = mmdp_malloc(oksMemPool, sizeof(OSH)))) {
						otbug_ks_error_deal("otbug_ks_thread_entrance - mmdp_malloc", errno);
	
						pthread_mutex_unlock(&oksThreadCtl.op_mutex);
						otbug_tcler_dec(tmCtl);
	
						kill(getpid(), SIGUSR2);
					}

					pHandler->oh_next = NULL;
					pHandler->oh_left = SDT_PER_OKSHANDLER;

					oksSdtContainer->oc_last = oksSdtContainer->oc_last->oh_next = pHandler;
					oksSdtContainer->oc_psdt = &pHandler->oh_sdt[0];
					oksSdtContainer->oc_noh++;
				}

				pHandler->oh_left--;
				*oksSdtContainer->oc_psdt = sdtMov;
				oksSdtContainer->oc_psdt++;
				oksSdtContainer->oc_tsdt++;
			}
		}

		if(newCnt) {
			sp_shell_sort(oksShellSort, oksSdtContainer->oc_start, oksSdtContainer->oc_tsdt);
			printf("OKS---> nKeyword: %d\n", oksSdtContainer->oc_tsdt);
	
			buff_size_add(oksTermBuf, sprintf(buff_place_end(oksTermBuf), "%-*d\n", NTERMS_LEN_MAX, nTopTerms));
	
			pHandler = oksSdtContainer->oc_start;
			pSdt = pHandler->oh_sdt[0];
			nCir = (newCnt > nTopTerms) ? nTopTerms : newCnt;

			for(nLoop = 0; nCir > 0; pSdt = pHandler->oh_sdt[++nLoop], nCir--) {
				if(nLoop == SDT_PER_OKSHANDLER) {
					pHandler = pHandler->oh_next;
					pSdt = pHandler->oh_sdt[(nLoop = 0)];
				}

				buff_size_add(oksTermBuf, sprintf(buff_place_end(oksTermBuf), "%d %.*s %d\n",
					nLoop + 1, BYTE_CMP_MAX, &pSdt->sd_data, pSdt->sd_tmcon.tc_count));
			}

			if(pwrite(rankFileDesc, buff_place_start(oksTermBuf), buff_now_size(oksTermBuf), 0) == FUN_RET_NEG) {
				otbug_ks_error_deal("otbug_ks_thread_entrance - pwrite", errno);
	
				pthread_mutex_unlock(&oksThreadCtl.op_mutex);
				otbug_tcler_dec(tmCtl);
	
				kill(getpid(), SIGUSR2);
			}

			buff_stru_make_empty(oksTermBuf);
		}

		otbug_tcler_dec(tmCtl);

		oksThreadCtl.op_flags = OPS_UNDONE;
		pthread_mutex_unlock(&oksThreadCtl.op_mutex);
	}
}


/*------------------------------------------
	Part Eight: OKSSDT ctl

	1. otbug_ks_sdt_mov
	2. otbug_ks_sdt_assign
	3. otbug_ks_sdt_progress
	4. otbug_ks_sdt_cmprep

--------------------------------------------*/

/*-----otbug_ks_sdt_mov-----*/
static SDT **otbug_ks_sdt_mov(OSH *pOsh, SDT **pSdt, int nMov)
{
	OSH	*oshMov = pOsh;
	int	nCir, nDiff, nOff;

	nDiff = (pSdt - &pOsh->oh_sdt[0]);

	if(nDiff + nMov >= SDT_PER_OKSHANDLER) {
		nCir = (nDiff + nMov) / SDT_PER_OKSHANDLER;
		nOff = (nDiff + nMov) % SDT_PER_OKSHANDLER;

		for(; nCir > 0; nCir--, oshMov = oshMov->oh_next)
			;	/* nothing */

	} else {
		nOff = nMov + nDiff;
	}

	return	&oshMov->oh_sdt[nOff];
}


/*-----otbug_ks_sdt_assign-----*/
static SDT **otbug_ks_sdt_assign(OSH *pOsh)
{
	return	&(pOsh->oh_sdt[0]);
}


/*-----otbug_ks_sdt_progress-----*/
static SDT **otbug_ks_sdt_progress(OSH **pOshp, SDT **pSdt)
{
	if((pSdt - &(*pOshp)->oh_sdt[0]) == SDT_PER_OKSHANDLER - 1) {
		*pOshp = (*pOshp)->oh_next;
		return	&(*pOshp)->oh_sdt[0];
	}

	return	++pSdt;
}


/*-----otbug_ks_sdt_cmprep-----*/
static void otbug_ks_sdt_cmprep(SDT **pSdt, SDT **pCmp)
{
	SDT	*sdtSave;

	if((*pCmp)->sd_tmcon.tc_count > (*pSdt)->sd_tmcon.tc_count) {
		sdtSave = *pCmp;
		*pCmp = *pSdt;
		*pSdt = sdtSave;
	}
}


/*------------------------------------------
	Part Nine: Error dealing

	1. otbug_ks_error_deal

--------------------------------------------*/

/*-----otbug_ks_error_deal-----*/
static void otbug_ks_error_deal(char *errStr, int nError)
{
	printf("OKS---> %s - %s\n", errStr, strerror(nError));
}

