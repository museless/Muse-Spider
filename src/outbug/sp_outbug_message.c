/*------------------------------------------
	Source file content Five part

	Part Zero:	Include
	Part One:	Local data
	Part Two:	Local function
	Part Three:	Define

	Part Four:	Message module pluging

--------------------------------------------*/

/*------------------------------------------
	Part Zero: Include
--------------------------------------------*/

#include "spinc.h"
#include "spnet.h"
#include "spmsg.h"
#include "spoutb.h"


/*------------------------------------------
	Part Two: Local function
--------------------------------------------*/

/* Part Four */
static	void	otbug_msg_ctime(void);
static	void	otbug_msg_kwork(void *kpPara);
static	int	otbug_msg_send(int sendFd);


/*------------------------------------------
	Part Four: Message module pluging

	1. otbug_msg_init
	2. otbug_msg_ctime
	3. otbug_msg_kwork
	4. otbug_msg_send

--------------------------------------------*/

/*-----otbug_msg_init-----*/
void *otbug_msg_init(int messageFd)
{
	return	sp_msg_frame_init(tbNameBuf, PART_OUTBUG, messageFd,
		TAKE_A_SHNAP, otbug_msg_ctime, otbug_msg_kwork, otbug_msg_send);
}


/*-----otbug_msg_ctime-----*/
static void otbug_msg_ctime(void)
{
	return;
}


/*-----otbug_msg_kwork-----*/
static void otbug_msg_kwork(void *kpPara)
{
	return;
}


/*-----otbug_msg_send-----*/
static int otbug_msg_send(int sendFd)
{
	return	FUN_RUN_OK;
}
