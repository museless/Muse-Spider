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
static	int	otbug_msg_send(int sendSock);


/*------------------------------------------
	Part Four: Message module pluging

	1. otbug_msg_init
	2. otbug_msg_ctime
	3. otbug_msg_kwork
	4. otbug_msg_send

--------------------------------------------*/

/*-----otbug_msg_init-----*/
void *otbug_msg_init(void)
{
	return	sp_msgs_frame_init(PART_OUTBUG, otbug_msg_ctime, otbug_msg_kwork, otbug_msg_send);
}


/*-----otbug_msg_ctime-----*/
static void otbug_msg_ctime(void)
{
	otbug_time_change();
}


/*-----otbug_msg_kwork-----*/
static void otbug_msg_kwork(void *kpPara)
{
	otbug_keep_working(kpPara);
}


/*-----otbug_msg_send-----*/
static int otbug_msg_send(int sendSock)
{
	return	FUN_RUN_OK;
}
