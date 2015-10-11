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
