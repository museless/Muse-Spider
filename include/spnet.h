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


#ifndef	_SPNET_H
#define	_SPNET_H

/* include */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

/* define */
#define	DBURL_ID_LEN	0x20	/* must be the same as spdb.h - URL_ID_LEN */
#define	WEB_TITLE_LEN	0x100
#define	WEB_CONTEXT_LEN	0x4000
#define	WEB_INDEX_LEN	0x30
#define	WEB_TIME_LEN	0x6
#define	WEB_DATE_LEN	0x8
#define	WEB_EXTRA_LEN	0x20

#define	WEB_COUNT_LEN	(WEB_EXTRA_LEN + WEB_INDEX_LEN + WEB_TIME_LEN + WEB_DATE_LEN)

#define	HTTP_GFILE_STR	"GET %s HTTP/1.1\r\nHost: %s\r\n%s"

/* server respone */
#define	RESP_CONNECT_OK		200
#define	RESP_CONNECT_ACCEPT	202

#define	RESP_PERM_MOVE		301
#define	RESP_TEMP_MOVE		302

#define	RESP_SERV_UNAVAIABLE	503

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

	char	*w_url;			/* store url (a temp value) */
	char	*w_conbuf;		/* store web content */

	SOCKAD	w_sockaddr;

	WEB	w_ubuf;
	char	w_latest[LATEST_LEN];

	int	w_pattern;
	int	w_size;			/* size of webpage */
	pth_t	w_tid;
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
