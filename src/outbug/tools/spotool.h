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
