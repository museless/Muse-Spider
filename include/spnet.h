#ifndef	_SPNET_H
#define	_SPNET_H

/* include */
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

/* define */
#define	MIN_URL_LEN             0x80
#define MAX_URL_LEN             0x100

#define	LATEST_UPGRADE_LIMIT    0x10

#define	DBURL_ID_LEN            0x20    /* must be the same as spdb.h - URL_ID_LEN */
#define	WEB_TITLE_LEN           0x100
#define	WEB_CONTEXT_LEN         0x4000
#define	WEB_INDEX_LEN           0x30
#define	WEB_TIME_LEN            0x6
#define	WEB_DATE_LEN            0x8
#define	WEB_EXTRA_LEN           0x20

#define	WEB_COUNT_LEN           (WEB_EXTRA_LEN + WEB_INDEX_LEN + WEB_TIME_LEN + WEB_DATE_LEN)

#define	HTTP_GFILE_STR          "GET %s HTTP/1.1\r\nHost: %s\r\n%s"

/* server respone */
#define	RESP_CONNECT_OK         200
#define	RESP_CONNECT_ACCEPT     202

#define	RESP_PERM_MOVE          301
#define	RESP_TEMP_MOVE          302

#define	RESP_SERV_UNAVAIABLE    503

/* typedef */
typedef	struct	web_if		WEBIN;
typedef	struct	web		WEB;
typedef	struct	web_txt		TEXT;
typedef	struct	sockaddr_in	SOCKAD;

typedef	int	(*fcset)(unsigned char *);

struct	web {
	char	web_host[HOST_LEN];
	char	web_file[FILE_LEN];
	int	web_port;
};

struct	web_if {
	WEBIN	*w_next;
	BUFF	*w_buff;		/* store url */

	char	*w_url;			/* store url (a temp value) */
	char	*w_conbuf;		/* store web content */

	SOCKAD	w_sockaddr;
	WEB	w_ubuf;

	char	w_latest[LATEST_LEN];	/* use for saving latest time */
	int	w_lcnt;			/* when equal than LATEST_UPGRADE_LIMIT, update to mysql */

	int	w_pattern;
	int	w_size;			/* size of webpage */
};

struct	web_txt {
	char	*wt_pool;
	int	wt_plsize;

	fcset	wt_csetfun;
	int	wt_cinc;

	char	wt_url[URL_LEN];
	char	wt_id[DBURL_ID_LEN];
	char	wt_charset[CHARSET_LEN];

	char	wt_index[WEB_INDEX_LEN];
	char	wt_title[WEB_TITLE_LEN];
	char	wt_time[WEB_TIME_LEN];

	uChar	wt_pattern;
	uChar	wt_tlen;
};

/* extern data */
extern	char	*rPac;

#endif
