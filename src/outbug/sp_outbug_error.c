/*------------------------------------------
	Source file content Five part

	Part Zero:	Include
	Part One:	Local data
	Part Two:	Local function
	Part Three:	Define

	Part Four:	Error dealing

--------------------------------------------*/

/*------------------------------------------
	Part Zero: Include
--------------------------------------------*/

#include "spinc.h"
#include "spdb.h"
#include "spnet.h"
#include "spoglobal.h"
#include "spoutb.h"


/*------------------------------------------
	Part Four: Error dealing

	1. otbug_sig_error
	2. otbug_perror

--------------------------------------------*/

/*-----otbug_sig_error-----*/
void otbug_sig_error(void)
{
	kill(getpid(), SIGINT);
}


/*-----otbug_perror-----*/
void otbug_perror(char *errStr, int nErr)
{
	printf("Otbug---> %s: %s\n", errStr, strerror(nErr));
}
