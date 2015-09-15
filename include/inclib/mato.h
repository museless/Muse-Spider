#ifndef	_MATO_H
#define	_MATO_H


/*---------------------------
	define
-----------------------------*/

#define	MATO_FALSE	0x0
#define	MATO_TRUE	0x1


/*---------------------------
	typedef
-----------------------------*/

typedef	struct	matos		MATOS;
typedef	unsigned long int	mar_t;


/*---------------------------
	struct
-----------------------------*/

struct	matos {
	int	mato_cnt;
};


/*---------------------------
	global function
-----------------------------*/

void	mato_init(MATOS *pMato, int nSet);

mar_t	mato_inc(MATOS *pMato);
mar_t	mato_add(MATOS *pMato, int nInc);

mar_t	mato_dec(MATOS *pMato);
mar_t	mato_sub(MATOS *pMato, int nSub);

mar_t	mato_inc_and_test(MATOS *pMato);
mar_t	mato_add_and_test(MATOS *pMato, int nInc);

mar_t	mato_dec_and_test(MATOS *pMato);
mar_t	mato_sub_and_test(MATOS *pMato, int nSub);


#endif
