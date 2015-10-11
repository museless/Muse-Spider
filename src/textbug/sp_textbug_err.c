#include "spinc.h"
#include "spdb.h"
#include "spnet.h"
#include "spmpool.h"
#include "sptglobal.h"
#include "sptextb.h"


/*------------------------------------------
	Part One: Error dealing

	1. txbug_sig_error

--------------------------------------------*/

/*-----txbug_sig_error-----*/
void txbug_sig_error(void)
{
	kill(getpid(), SIGINT);
}


/*-----txbug_perror-----*/
void txbug_perror(char *errStr, int nErr)
{
	printf("Textbug---> %s: %s\n", errStr, strerror(nErr));
}
