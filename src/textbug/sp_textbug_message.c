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
static	int	txbug_msg_send(int nSocket);


/*------------------------------------------
	Part Four: Textbug message frame

	1. txbug_msg_init
	2. txbug_msg_ctime
	3. txbug_msg_kwork
	4. txbug_msg_send

--------------------------------------------*/

/*-----txbug_msg_init-----*/
void *txbug_msg_init(void)
{
	return	sp_msgs_frame_init(PART_TEXTBUG, txbug_msg_ctime, txbug_msg_kwork, txbug_msg_send);
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
static int txbug_msg_send(int nSocket)
{
	return	txbug_send_message(nSocket);
}


/*------------------------------------------
	Part Five: Textbug message assist

	1. txbug_wait_arouse

--------------------------------------------*/

/*-----txbug_wait_arouse-----*/
void txbug_wait_arouse(void *msgSet)
{
	SOSET	*pSet = (SOSET *)msgSet;
	SOCKMG	waitMsg;

	printf("Textbug---> waiting for Urlbug to wake up\n");

	while(FUN_RUN_OK) {
		if(!sp_msgs_recv(pSet->sc_sock, &waitMsg, MSG_SIZE)) {
			if(sp_msgs_sender(&waitMsg, PART_URLBUG) && sp_msgs_exam(&waitMsg, KEEP_WORKING))
				break;
		}
	}

	printf("Textbug---> urlbug calling me\n");
}
