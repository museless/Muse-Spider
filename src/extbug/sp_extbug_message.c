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

	Part Four:	Extbug message frame
	Part Five:	Extbug message assist

--------------------------------------------*/

/*------------------------------------------
	Part Zero: Include
--------------------------------------------*/

#include "spinc.h"
#include "spnet.h"
#include "spmsg.h"
#include "spextb.h"


/* local function */
static	void	exbug_msg_ctime(void);
static	void	exbug_msg_kwork(void *kpPara);
static	int	exbug_msg_send(int sendSock);


/*------------------------------------------
	Part Four: Extbug message frame

	1. exbug_msg_init
	2. exbug_msg_ctime
	3. exbug_msg_kwork
	4. exbug_msg_send

--------------------------------------------*/

/*-----exbug_msg_init-----*/
void *exbug_msg_init(void)
{
	return	sp_msgs_frame_init(PART_EXTBUG, exbug_msg_ctime, exbug_msg_kwork, exbug_msg_send);
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
static int exbug_msg_send(int sendSock)
{
	return	exbug_send_message(sendSock);
}


/*------------------------------------------
	Part Five: Extbug message assist

	1. exbug_wait_arouse

--------------------------------------------*/

/*-----exbug_wait_arouse-----*/
void exbug_wait_arouse(void *msgSet)
{
	SOSET	*pSet = (SOSET *)msgSet;
	SOCKMG	waitMsg;

	printf("Extbug---> waiting for Textbug to wake up\n");

	while(FUN_RUN_OK) {
		if(!sp_msgs_recv(pSet->sc_sock, &waitMsg, MSG_SIZE)) {
			if(sp_msgs_sender(&waitMsg, PART_TEXTBUG) && sp_msgs_exam(&waitMsg, KEEP_WORKING))
				break;
		}
	}

	printf("Extbug---> Textbug calling me\n");
}
