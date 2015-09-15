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


#ifndef	_SPOTOOL_H
#define	_SPOTOOL_H


/*---------------------------
	include
-----------------------------*/

#include "spinc.h"
#include "spdb.h"

#include "mmdpool.h"
#include "mipc.h"
#include "mpctl.h"

#include "spoutb.h"
#include "spoglobal.h"


/*---------------------------
	define
-----------------------------*/


/*---------------------------
	typedef
-----------------------------*/

typedef	void	(*otpefun)(char *, int);


/*---------------------------
	global function
-----------------------------*/

/* sp_outbug_tool_frame.c */
int	otbug_tool_frame_init(char *frameCaller, MGCH **garCollect, MSHM **shmReader, otpefun errFun);
int	otbug_tool_signup(char *pathBuf, TLCTL *pCtl, otpefun errFun);
int	otbug_tool_signal_init(sigfun fSignal, otpefun errFun);


#endif
