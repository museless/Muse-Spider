/*------------------------------------------
	Source file content Five part

	Part Zero:	Include
	Part One:	Define
	Part Two:	Local data
	Part Three:	Local function

	Part Four:	Libary function packing

--------------------------------------------*/

/*------------------------------------------
	Part Zero: Include
--------------------------------------------*/

#include "spinc.h"
#include "spnet.h"
#include <sys/select.h>
#include <sys/time.h>


/*------------------------------------------
	Part Four: Libary function packing

	1. readn
	2. writen
	3. strnstr
	4. strnchr
	5. select_read
	6. strchr_backword
	7. atoin
	8. read_all_file
	9. examine_empty_string
	10. socket_set_timer

--------------------------------------------*/

/*-----readn-----*/
int readn(int rFd, void *rBuf, size_t bCount)
{
	int	fRet, nCir = 0;

	while(FUN_RUN_OK) {
		if((fRet = read(rFd, rBuf + nCir, bCount)) == bCount || fRet == 0) {
			nCir += fRet;
			return	nCir;

		} else if(fRet >= 0 && fRet < bCount) {
			nCir += fRet;
			bCount -= fRet;

		} else {
			return	FUN_RUN_FAIL;
		}
	}
}


/*-----writen-----*/
int writen(int wFd, void *wBuf, size_t bCount)
{
	int	fRet, nCir = 0;

	while(FUN_RUN_OK) {
		if((fRet = write(wFd, wBuf + nCir, bCount)) == bCount || fRet == 0) {
			nCir += fRet;
			return	nCir;

		} else if(fRet >= 0 && fRet < bCount) {
			nCir += fRet;
			bCount -= fRet;

		} else {
			if(errno != EINTR)
				return	-1;
		}
	}
}


/*-----strnstr-----*/
inline char *strnstr(char *findBuf, char *needStr, int nLimit)
{
	char	*pStr, sChar;

	if(findBuf == NULL || needStr == 0 || nLimit <= 0)
		return	NULL;

	sChar = findBuf[nLimit];
	findBuf[nLimit] = 0;

	pStr = strstr(findBuf, needStr);
	
	findBuf[nLimit] = sChar;
	return	pStr;
}


/*-----strnchr-----*/
inline char *strnchr(char *findBuf, char needCh, int nLimit)
{
	char	sChar, *pStr;

	if(findBuf == NULL || nLimit <= 0)
		return	NULL;

	sChar = findBuf[nLimit];
	findBuf[nLimit] = 0;

	pStr = strchr(findBuf, needCh);

	findBuf[nLimit] = sChar;
	return	pStr;
}


/*-----select_read-----*/
int select_read(int nSock, char *readBuf, int nRead, int nSec, int uSec)
{
	struct	timeval	timStru;
	fd_set		fdBuf;

	timStru.tv_sec = nSec;
	timStru.tv_usec = uSec;

	FD_ZERO(&fdBuf);
	FD_SET(nSock, &fdBuf);

	if(select(nSock + 1, &fdBuf, NULL, NULL, &timStru) > FUN_RUN_END) {
		if(FD_ISSET(nSock, &fdBuf))
			return	(read(nSock, readBuf, nRead));
	}

	return	FUN_RUN_END;
}


/*-----strchr_backward-----*/
char *strchr_backward(char *pEnd, int nLimit, char fCh)
{
	int	nCir;

	for(nCir = 0; nCir < nLimit; nCir++) {
		if(*(pEnd - nCir) == fCh)
			return	(pEnd - nCir);
	}

	return	NULL;
}


/*-----strchr_forward-----*/
char *strchr_forward(char *pEnd, int nLen, char findCh)
{
	int	nCir;

	for(nCir = nLen; nCir > 0; nCir--) {
		if(*(pEnd - nCir) == findCh)
			return	(pEnd - nCir);
	}

	return	NULL;
}


/*-----atoin-----*/
inline int atoin(char *datBuf, int nLen)
{
	char	saveCh;
	int	nRet;

	saveCh = datBuf[nLen];
	datBuf[nLen] = 0;

	nRet = atoi(datBuf);
	datBuf[nLen] = saveCh;

	return	nRet;
}


/*-----read_all_file-----*/
int read_all_file(char **pStore, char *ofName, int readOff)
{
	struct	stat	stBuf;
	int		readFd;

	if((readFd = open(ofName, O_RDWR)) == FUN_RUN_FAIL)
		return	FUN_RUN_FAIL;

	if(fstat(readFd, &stBuf) == FUN_RUN_FAIL)
		return	FUN_RUN_FAIL;

	if(readOff) {
		if(lseek(readFd, readOff, SEEK_SET) == FUN_RUN_FAIL)
			return	FUN_RUN_FAIL;

		stBuf.st_size -= readOff;
	}

	*pStore = NULL;

	if(stBuf.st_size) {
		if(!(*pStore = malloc(stBuf.st_size + 1)))
			return	FUN_RUN_FAIL;
	
		(*pStore)[stBuf.st_size] = 0;
	
		if(readn(readFd, *pStore, stBuf.st_size) == FUN_RUN_FAIL)
			return	FUN_RUN_FAIL;
	}

	close(readFd);

	return	stBuf.st_size;
}


/*-----examine_empty_string-----*/
inline int examine_empty_string(char *exaStr)
{
	return	(exaStr[0] == 0) ? FUN_RUN_OK : FUN_RUN_END;
}


/*-----socket_set_timer------*/
int socket_set_timer(int nSocket, int nSec, int uSec, int nFlags)
{
	struct	timeval	tvStru;

	if(nFlags != SO_RCVTIMEO && nFlags != SO_SNDTIMEO)
		return	FUN_RUN_FAIL;

	tvStru.tv_sec = nSec;
	tvStru.tv_usec = uSec;

	return	setsockopt(nSocket, SOL_SOCKET, nFlags, &tvStru, sizeof(tvStru));
}


/*-----time_str_extract-----*/
TMS *time_str_extract(char *timStr)
{
	TMS	*extTime;
	time_t	timType;

	timType = time(NULL);
	extTime = localtime(&timType);

	extTime->tm_year += 1900;
	extTime->tm_mon += 1;

	if(timStr && strlen(timStr) == DATEBUF_LEN) {
		int	nYear, nMon, nDay;

		nYear = atoin(timStr, 4);
		nMon = atoin(timStr + 4, 2);
		nDay = atoin(timStr + 6, 2);
	
		if(nYear <= extTime->tm_year && (nMon >= 1 && nMon <= 12) && (nDay >= 1 && nDay <= 31)) {
			extTime->tm_year = nYear;
			extTime->tm_mon = nMon;
			extTime->tm_mday = nDay;
		}
	}

	return	extTime;
}
