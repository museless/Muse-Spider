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
	Part One:	Define
	Part Two:	Local data
	Part Three:	Local function

	Part Four:	Cleaning operate

--------------------------------------------*/


/*------------------------------------------
	Part Zero: Include
--------------------------------------------*/

#include "spinc.h"
#include "spmpool.h"
#include "spnet.h"
#include "spdb.h"
#include "sptextb.h"
#include "sptglobal.h"


/*------------------------------------------
	Part Four: Cleaning operate

	1. txbug_pool_free
	2. txbug_db_unlink_url
	3. txbug_db_unlink_cont
	4. txbug_db_close

--------------------------------------------*/

/*-----txbug_pool_free-----*/
void txbug_pool_free(TEXT *textPoint)
{
	wmpool_free(contMemPool, textPoint->wt_pool);
	wmpool_free(tsMemPool, textPoint);
}


/*-----txbug_db_unlink_url-----*/
void txbug_db_unlink_url(void)
{
	mysql_close(&urlDataBase);
}


/*-----txbug_db_unlink_cont-----*/
void txbug_db_unlink_cont(void)
{
	mysql_close(&txTranHand.tx_sql);
}


/*-----txbug_db_close-----*/
void txbug_db_close(void)
{
	mysql_library_end();
}

