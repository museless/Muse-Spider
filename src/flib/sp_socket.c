#include "spinc.h"
#include "spnet.h"
#include "spdb.h"
#include "spmsg.h"
#include "spsock.h"
#include "sphtml.h"
#include "spmpool.h"


/*------------------------------------------
	Source file content Four part

	Part Zero:	Local data
	Part One:	Local function
	Part Two:	Define function
	Part Three:	Socket way

--------------------------------------------*/

/*------------------------------------------
	Part Three: Socket way

	1. sp_soc_init
	2. sp_soc_set
	3. sp_soc_run
	4. sp_soc_destroy

--------------------------------------------*/

/*-----sp_soc_init-----*/
SOC *sp_soc_init(lib_sock libFun, usr_sock usrFun)
{
	SOC	*socStru;

	if(!libFun || !usrFun)
		return	NULL;

	if(!(socStru = malloc(sizeof(SOC))))
		return	NULL;

	socStru->so_lib = libFun;
	socStru->so_usr = usrFun;
	socStru->so_sec = socStru->so_usec = 0;

	return	socStru;
}


/*-----sp_soc_set-----*/
void sp_soc_set(SOC *socStru, int nSock, uLong nSec, uLong uSec, int setFlags, int libFlags)
{
	socStru->so_sec = nSec;
	socStru->so_usec = uSec;
	socStru->so_sock = nSock;
	socStru->so_lib_flags = libFlags;

	socStru->so_flags = (socket_set_timer(nSock, nSec, uSec, setFlags) == FUN_RUN_FAIL) ? USR_SOCK : LIB_SOCK;
}


/*-----sp_soc_run-----*/
inline int sp_soc_run(SOC *pSoc, void *bufStore, int readLen)
{
	return	(pSoc->so_flags == LIB_SOCK) ?
		pSoc->so_lib(pSoc->so_sock, bufStore, readLen, pSoc->so_lib_flags) :
		pSoc->so_usr(pSoc->so_sock, bufStore, readLen, pSoc->so_sec, pSoc->so_usec);
}


/*-----sp_soc_destroy-----*/
void sp_soc_destroy(void *socStru)
{
	if(socStru)
		free(socStru);
}
