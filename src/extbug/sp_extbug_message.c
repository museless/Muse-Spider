#include "spinc.h"
#include "spnet.h"
#include "spmsg.h"
#include "spextb.h"


/* local data */
static	void	exbug_msg_ctime(void);
static	void	exbug_msg_kwork(void *kpPara);
static	int	exbug_msg_send(int sendFd);


/*------------------------------------------
	Source file content Three part

	Part Zero:	Define function
	Part One:	Extbug message frame

--------------------------------------------*/

/*------------------------------------------
	Part One: Extbug message frame

	1. exbug_msg_init
	2. exbug_msg_ctime
	3. exbug_msg_kwork
	4. exbug_msg_send

--------------------------------------------*/

/*-----exbug_msg_init-----*/
void *exbug_msg_init(int messageFd)
{
	return	sp_msg_frame_init(ebNameBuf, PART_EXTBUG, messageFd,
		TAKE_A_SHNAP, exbug_msg_ctime, exbug_msg_kwork, exbug_msg_send);
}


/*-----exbug_msg_ctime-----*/
static void exbug_msg_ctime(void)
{
	exbug_time_change();
}


/*-----exbug_msg_kwork-----*/
static void exbug_msg_kwork(void *pPara)
{
	exbug_keep_working(pPara);
}


/*-----exbug_msg_send-----*/
static int exbug_msg_send(int sendFd)
{
	return	FUN_RUN_OK;
}


/*------------------------------------------
	Part Two: Extbug message assist

	1. exbug_wait_arouse

--------------------------------------------*/

/*-----exbug_wait_arouse-----*/
void exbug_wait_arouse(int waitFd, int wTime)
{
	PMSG	waitMsg;
	int	nRet;

	while(FUN_RUN_OK) {
		if((nRet = sp_msg_select_read(waitFd, &waitMsg, wTime)) > FUN_RUN_END) {
			if(sp_msg_exam_recver(&waitMsg, PART_EXTBUG, tbNameBuf) == FUN_RUN_OK) {
				if(sp_msg_exam_command(&waitMsg, KEEP_WORKING))
					break;
			}
		}
	}
}
