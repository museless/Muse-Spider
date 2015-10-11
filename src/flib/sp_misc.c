#include "spinc.h"


/*------------------------------------------
	Part Zero: Debug function

	1. sp_stop
	2. sp_stop_str

--------------------------------------------*/

/*-----sp_stop-----*/
void sp_stop(void)
{
	while(FUN_RUN_OK)
		;	/* Dead loop */
}


/*-----sp_stop_str-----*/
void sp_stop_str(void)
{
	printf("\nMuseSp---> in sp_stop\n");
	sp_stop();
}
