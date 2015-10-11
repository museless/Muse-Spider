#ifndef	_SPEGLOBAL_H
#define	_SPEGLOBAL_H

/*-------------------------
	extern data
---------------------------*/

extern	MGCH	*exbGarCol;
extern	BUFF	*extSaveBuf;

extern	void	*extbugMsgSet;			/* MSGSET */
extern	void	*threadMemPool, *procMemPool;	/* DMPH */
extern	void	*ebSemControl;			/* MSEM */

extern	EXBSET	exbRunSet;
extern	MYSQL	dbNewsHandler, dbDicHandler, dbKeysHandler;

extern	CLISTS	charTermList, charHeadSave;

extern	MGCO	extResCol;

extern	char	sqlSeleCom[];
extern	char	tblNewsName[], dbNewsName[];
extern	char	tblWordName[], dbDicName[];
extern	char	tblKeysName[], dbKeysName[];

extern	MATOS	pthreadCtlLock, freeCtlLock, nPaperLock, dicDbLock;

extern	uLong	tPaperNum;
extern	int	exbugIpcFd, nExbugPthead, upMaxTerms, nKlistSize;

#endif
