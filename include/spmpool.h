/*Copyright (c) 2015, William Muse
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


#ifndef	_SPMPOOL_H
#define	_SPMPOOL_H

/* define */
#define	WMP_MAPBIT	0x20	/* 32 */
#define	WMP_MAPBIT_SHI	0x5	/* 2 ^ 5 */
#define	WMP_PSIZE_BOR	0x4B000
#define	WMP_PAGESIZE	0x50000

#define	WMP_BIT_BLOCK	0x1
#define	WMP_BIT_UNBLOCK	0x0

/* web page mempool */
typedef	struct	w_mempool {
	char	*wmp_point;
	int	*wmp_map;

	int	wmp_psize;	/* page size */
	int	wmp_clip;	/* block num */
}WPOOL;

/* global function */
WPOOL	*wmpool_create(int nClip, int cSize);
void	*wmpool_malloc(WPOOL *pHandler);
void	*wmpool_calloc(WPOOL *pHandler);
void	wmpool_free(WPOOL *pHandler, void *aPoint);
void	wmpool_destroy(void *pHandler);

/* define function */
#define	wmpool_bit_is_block(nByte, nBit)	((nByte >> nBit) & WMP_BIT_BLOCK)

#endif
