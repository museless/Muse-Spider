/*------------------------------------------
	Source file content Thirteen part

	Part Zero:	Include
	Part One:	Local data
	Part Two:	Local function
	Part Three:	Define

	Part Four:	Msg function

--------------------------------------------*/

/*------------------------------------------
	Part Zero: Include
--------------------------------------------*/

#include "spinc.h"
#include "spdb.h"
#include "spmsg.h"

#include "spurlb.h"


/*------------------------------------------
	Part Two: Local function
--------------------------------------------*/

/* Part Four */
static	void	ubug_msg_ctime(void);
static	int	ubug_msg_send(int sendFd);


/*------------------------------------------
	Part Four: Msg function

	1. ubug_msg_init
	2. ubug_msg_ctime
	3. ubug_msg_send

--------------------------------------------*/

/*-----ubug_msg_init-----*/
void *ubug_msg_init(int msgFd)
{
	return	sp_msg_frame_init("URLBUG", PART_URLBUG, msgFd,
		TAKE_A_SHNAP, ubug_msg_ctime, NULL, ubug_msg_send);
}


/*-----ubug_msg_ctime-----*/
static void ubug_msg_ctime(void)
{
	ubug_time_change();
}


/*-----ubug_msg_send-----*/
static int ubug_msg_send(int sendFd)
{
	return	ubug_send_message(sendFd);
}
