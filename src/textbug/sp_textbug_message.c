/*------------------------------------------
	Source file content Six part

	Part Zero:	Include
	Part One:	Local data
	Part Two:	Local function
	Part Three:	Define

	Part Four:	Textbug message frame
	Part Five:	Textbug message assist

--------------------------------------------*/

/*------------------------------------------
	Part Zero: Include
--------------------------------------------*/

#include "spinc.h"
#include "spnet.h"
#include "spmsg.h"
#include "sptextb.h"


/*------------------------------------------
	Part Two: Local function
--------------------------------------------*/

/* Part Four */
static	void	txbug_msg_ctime(void);
static	void	txbug_msg_kwork(void *kpPara);
static	int	txbug_msg_send(int sendFd);


/*------------------------------------------
	Part Four: Textbug message frame

	1. txbug_msg_init
	2. txbug_msg_ctime
	3. txbug_msg_kwork
	4. txbug_msg_send

--------------------------------------------*/

/*-----txbug_msg_init-----*/
void *txbug_msg_init(int messageFd)
{
	return	sp_msg_frame_init(tbNameBuf, PART_TEXTBUG, messageFd,
		TAKE_A_SHNAP, txbug_msg_ctime, txbug_msg_kwork, txbug_msg_send);
}


/*-----txbug_msg_ctime-----*/
static void txbug_msg_ctime(void)
{
	txbug_time_change();
}


/*-----txbug_msg_kwork-----*/
static void txbug_msg_kwork(void *kpPara)
{
	txbug_keep_working(kpPara);
}


/*-----txbug_msg_send-----*/
static int txbug_msg_send(int sendFd)
{
	return	txbug_send_message(sendFd);
}


/*------------------------------------------
	Part Five: Textbug message assist

	1. txbug_wait_arouse

--------------------------------------------*/

/*-----txbug_wait_arouse-----*/
void txbug_wait_arouse(int waitFd, int wTime)
{
	PMSG	waitMsg;
	int	nRet;

	while(FUN_RUN_OK) {
		if((nRet = sp_msg_select_read(waitFd, &waitMsg, wTime)) > FUN_RUN_END) {
			if(sp_msg_exam_recver(&waitMsg, PART_TEXTBUG, tbNameBuf) == FUN_RUN_OK) {
				if(sp_msg_exam_command(&waitMsg, KEEP_WORKING))
					break;
			}
		}
	}
}
