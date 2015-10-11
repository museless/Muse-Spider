#include "spinc.h"
#include "spdb.h"
#include "spextb.h"
#include "speglobal.h"


/*------------------------------------------
	Part One: Error dealing

	1. exbug_sig_error

--------------------------------------------*/

/*-----exbug_sig_error-----*/
void exbug_sig_error(int nObj)
{
	if(nObj == PTHREAD_ERROR)
		mato_dec(&pthreadCtlLock);

	kill(getpid(), SIGINT);
}
