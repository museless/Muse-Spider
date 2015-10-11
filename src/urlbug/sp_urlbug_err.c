#include "spinc.h"
#include "spnet.h"
#include "spdb.h"
#include "spmsg.h"
#include "spmpool.h"
#include "spuglobal.h"
#include "spurlb.h"


/*------------------------------------------
	Part One: Urlbug error dispose

	1. ubug_sig_error
	2. ubug_perror

--------------------------------------------*/

/*-----ubug_sig_error-----*/
void ubug_sig_error(void)
{
	kill(getpid(), SIGINT);
}


/*-----ubug_perror-----*/
void ubug_perror(char *errStr, int nErr)
{
	printf("Urlbug---> %s: %s\n", errStr, strerror(nErr));
}
