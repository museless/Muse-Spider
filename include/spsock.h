#ifndef	_SPSOCK_H
#define	_SPSOCK_H

/* define */
#define	LIB_SOCK	0x1
#define	USR_SOCK	0x2

/* time */

/* sec */
#define	NO_TIME		0x0
#define	ONE_TIME	0x1
#define	TINY_TIME	0x2
#define	SMALL_TIME	0x4
#define	MIDDLE_TIME	0x8

/* usec */
#define	TINY_USEC	0x80000

/* typedef function */
typedef	int	(*lib_sock)(int, void *, size_t, int);
typedef	int	(*usr_sock)(int, void *, size_t, int, int);

/* typedef struct */
typedef	struct	sock_rw_control	SOC;

/* struct */
struct	sock_rw_control {
	lib_sock	so_lib;
	usr_sock	so_usr;

	int		so_flags;
	int		so_sock;

	int		so_lib_flags;

	uLong		so_sec;
	uLong		so_usec;
};

/* global function */
SOC	*sp_soc_init(lib_sock libFun, usr_sock usrFun);
void	sp_soc_set(SOC *socStru, int nSock, uLong nSec, uLong uSec, int setFlags, int libFlags);
int	sp_soc_run(SOC *pSoc, void *bufStore, int readLen);
void	sp_soc_destroy(void *socStru);


#endif
