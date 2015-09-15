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


#ifndef	_SPDB_H
#define	_SPDB_H

/*-----------------------------
	include
-------------------------------*/

#include <mysql.h>
#include <errmsg.h>
#include <mysqld_error.h>


/*-----------------------------
	define
-------------------------------*/

#define	DBUSRNAME	"root"
#define	DBUSRKEY	"WdSr0922"

#define	SQL_TCOM_LEN	0x80	/* tiny */
#define	SQL_LICOM_LEN	0xC0	/* little */
#define	SQL_SCOM_LEN	0x100	/* small */
#define	SQL_MCOM_LEN	0x200	/* middle */
#define	SQL_BCOM_LEN	0x300	/* big */
#define	SQL_GCOM_LEN	0x600	/* great */
#define	SQL_LCOM_LEN	0x1000	/* large */
#define	SQL_HCOM_LEN	0x4000	/* huge */
#define	SQL_HHCOM_LEN	0x6000	/* huge huge */
#define	SQL_TABNAME_LEN	0x20

#define	SQL_DBNAME_LEN	0x80

#define	SQL_DOMAIN_LEN	32
#define	SQL_RELOC_LEN	64
#define	SQL_STRCMP_LEN	64
#define	SQL_SRCSTR_LEN	24
#define	SQL_CHARSET_LEN	8

#define	SQL_USTATE_LEN	0x30

#define	SQL_PERCOM_MLEN	0x200

#define	SQL_NKILST_DEF	0x800

/*-----------------------------
	sql command
-------------------------------*/

#define	DROP_VIEW	"drop view V%s"
#define	DROP_TABLE	"drop table %s"


/*-----------------------------
	for urlbug
-------------------------------*/

#define	GET_DIRECTORY	"select Url, Latest from UALL where Blockmask=1"
#define	CREAT_URL_TAB	"create table if not exists %s(ID bigint(32) not null primary key auto_increment, \
Url char(128) not null, Pattern int(2), State bool default 0, Errt tinyint default 0)"

#define	TRAN_URL_BEG	"insert %s(Url, Pattern) values(\"%.*s\",%d)"
#define	TRAN_URL	",(\"%.*s\",%d)"

#define	REVIEW_URL	"select * from %s where Url=\"%.*s\""
#define	UPDATE_LATEST	"update UALL set Latest=\"%.*s\" where Url=\"%s\""

/* for textbug */
#define	URL_ID_LEN	32

#define	GET_URL_LIMIT	"select ID, Url, Pattern from %s where Errt<%d and State=0 and \
ID not in (select Ind from %s.%s) limit %d"
#define	GET_CONF	"select * from UCONF"

/* News */
#define	CHECK_IND_EXI	"select Ind from %s where Ind=\"%s\""

#define	CREAT_NEWS_TAB	"create table if not exists %s(Ind char(48) not null primary key,Date char(10) not null, \
Time char(5) not null,Source char(24) character set utf8 not null,Title char(128) character set utf8 not null, \
Url char(128) not null, Pramark tinyint(1) default 0, Extmark tinyint(1) default 0, \
Content MEDIUMTEXT character set utf8 not null)"

#define	TRAN_NEWS_BEG	"insert %s(Ind, Date, Time, Source, Url, Title, Content) \
values(\"%s\", \"%.8s\", \"%.5s\", \"%s\", \"%s\", \""
#define	TRAN_NEWS	", (\"%s\", \"%.8s\", \"%.5s\", \"%s\", \"%s\", \""

#define	RTURL_STATE	"update %s set State=1 where ID=\"%s\""
#define	WGURL_STATE	"update %s set State=2, Errt=Errt+1 where ID=\"%s\""
#define	FUNWG_STATE	"update %s set State=3, Errt=Errt+1 where ID=\"%s\""
#define	NOTIT_STATE	"update %s set State=5, Errt=Errt+1 where ID=\"%s\""
#define	NOCONT_STATE	"update %s set State=6, Errt=Errt+1 where ID=\"%s\""
#define	USING_STATE	"update %s set State=9 where ID=\"%s\""
#define	INC_ERRT	"update %s set Errt=Errt+1 where ID=\"%s\""

#define	UPDATE_URL	"update %s set Url=\"%s\" where Url=\"%s\""

/* view */
#define	CR_NEW_VIEW	"create or replace view V%s as \
select concat(\"Urls num: \", count(ID)) Count from %s \
union select concat(\"News num: \", count(Ind)) Count from %s \
union select concat(\"State = 0: \", count(ID)) Count from %s where State=0 \
union select concat(\"State = 1: \", count(ID)) Count from %s where State=1 \
union select concat(\"State = 2: \", count(ID)) Count from %s where State=2 \
union select concat(\"State = 6: \", count(ID)) Count from %s where State=6 \
union select concat(\"Et = 1: \", count(ID)) Count from %s where Errt=1 \
union select concat(\"Et = 2: \", count(ID)) Count from %s where Errt=2"

/* Ext */
#define	REVIEW_WORD	"select Word from %s where Word=\"%.*s\""

#define	TRAN_WD_BEG_ST	"insert %s(Word, Len, State) values(\"%.*s\",%d,%d)"
#define	TRAN_WD_ST	",(\"%.*s\",%d,%d)"

#define	TRAN_WD_BEG	"insert %s(Word, Len) values(\"%.*s\",%d)"
#define	TRAN_WD		",(\"%.*s\",%d)"

#define	DOWN_WD_ST	"select Word, State from %s"
#define	DOWN_WD_NO_ST	"select Word from %s"

#define	DELETE_KEYWORD	"delete from %s where Word=\"%.*s\""

#define	GET_LIMIT_WORD	"select * from %s where Word like '%.3s%%'"

#define	CREAT_KEY_TAB	"create table if not exists %s(Ind char(48) not null primary key, Klist varchar(%d) not null, \
Keynum int(32) not null, Keyflags tinyint(1) default 0)"

#define	GET_NEWS_CONT	"select Ind, Content from %s where %s = 0 limit %d"
#define	SET_NEWS_FLAGS	"update %s set %s=1 where Ind=\"%s\""

#define	GET_PAPER_NUM	"select * from %s"
#define	UPDATE_WORD_IDF	"update %s set Times=Times+1 where Word=\"%.*s\""
#define	UPDATE_KEEP_WD	" or Word=\"%.*s\""

#define	INSERT_KEYWD	"insert %s(Ind, Klist, Keynum) values(\"%s\", \"%.*s\", %d)"
#define	INSERT_KW_NEXT	",(\"%s\", \"%.*s\", %d)"

#define	DOWN_WORD_IDF	"select Times from %s where Word=\"%.*s\""
#define	DOWN_KEEP_WD	" or Word=\"%.*s\""

/* Outbug */
#define	GET_KEYWD_LIM	"select Ind, Klist from %s where Keyflags=0 limit %d"

#define	SET_KEYWD_FLAGS	"update %s set Keyflags=1 where Ind=\"%s\""


/*-----------------------------
	typedef
-------------------------------*/

typedef	MYSQL_RES		MSLRES;
typedef	MYSQL_ROW		MSLROW;

typedef	struct	sstrlist	SSTRL;
typedef	struct	fconf		SCONF;



/*-----------------------------
	struct
-------------------------------*/

struct	sstrlist {
	SSTRL	*s_next;
	char	s_str[SQL_STRCMP_LEN];
};

struct	fconf {
	SSTRL	*c_timbeg;
	SSTRL	*c_conbeg;
	SSTRL	*c_conend;

	char	c_domain[SQL_DOMAIN_LEN];
	char	c_conrloc[SQL_RELOC_LEN];
	char	c_srcstr[SQL_SRCSTR_LEN];
	char	c_charset[SQL_CHARSET_LEN];
};


/*-----------------------------
	extern data
-------------------------------*/

extern	char	*sqlOpt[];
extern	int	nrOpt;


#endif
