#ifndef	_MIPC_H
#define	_MIPC_H


/*---------------------
	include
-----------------------*/

#include <sys/types.h>
#include <sys/ipc.h>
#include "msem.h"
#include "mshm.h"


/*---------------------
	define
-----------------------*/

#define	MIPC_OK		1
#define	MIPC_END	0
#define	MIPC_FAIL	-1

#define	USR_READ	0400
#define	USR_WRITE	0200
#define	USR_RDWR	(USR_READ | USR_WRITE)

#define	GRP_READ	040
#define	GRP_WRITE	020
#define	GRP_RDWR	(GRP_READ | GRP_WRITE)

#define	UG_RDWR		(USR_RDWR | GRP_RDWR)

#define	OTH_READ	04
#define	OTH_WRITE	02
#define	OTH_RDWR	(OTH_READ | OTH_WRITE)

#define	ALL_READ	(USR_READ | GRP_READ | OTH_READ)
#define	ALL_WRITE	(USR_WRITE | GRP_WRITE | OTH_WRITE)

#endif
