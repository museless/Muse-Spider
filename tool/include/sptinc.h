#ifndef	_SPTINC_H
#define	_SPTINC_H

/* define */
#define	DBUSER		"root"
#define	DBKEY		"WdSr0922"
#define	DBNAME		"Url"
#define	NEWDB		"News"

#define	SQL_LEN		0x200
#define	CONF_NMEM	0x7
#define	BUFF_LEN	0x20000

#define	DOMAIN_LEN	0x20
#define	CONBEG_LEN	0x80
#define	CONEND_LEN	0x80
#define	LOC_LEN		0x20
#define	TIMBEG_LEN	0x80
#define	SRCSTR_LEN	0x18

#define	MD_DNC_AOFF	0x2
#define	MD_UPDOWN_AOFF	0x3
#define	MD_MDPATT_AOFF	0x1

/* Door locks */
#define	URLS_DLOCKS	0x1
#define	CONF_DLOCKS	0x2
#define	UALL_DLOCKS	0x3

#define	CREAT_URLSTAB	"create table if not exists %s(ID bigint(32) not null primary key auto_increment, \
Url char(128) not null, Pattern int(2), State bool default 0, Errt tinyint default 0)"
#define	CREAT_CONFTAB	"create table if not exists UCONF(Domain char(32) not null primary key, ConBeg char(128) not null, \
ConEnd char(128) not null, ReLoc char(32), TimeBeg char(128) not null, SrcStr char(24) character set utf8 not null, \
CharSet char(8) not null)"
#define	CREAT_UALLTAB	"create table if not exists UALL(Url char(128) not null primary key, Latest char(64) default null, \
Blockmask tinyint(1) default 1)"

#define	INSERT_URLSTAB	"insert %s(Url, Pattern) values(\"%.*s\", %d)"
#define	INSERT_CONFTAB	"insert UCONF values(\"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\")"
#define	INSERT_UALLTAB	"insert UALL(Url) values(\"%.*s\")"

#define	SELECT_UALL	"select * from UALL"
#define	SELECT_UCONF	"select * from UCONF"
#define	SELECT_URLS	"select * from %s"

#define	SELECT_SRC	"select SrcStr from Url.UCONF"
#define	SELECT_DATA	"select n.Ind, n.Title, n.Time, n.Url, n.Content from News.%s n where n.Source=\"%s\""

/* typedef */
typedef	struct	spconf	SPCONF;
typedef	int	(*tool_tran)(char *, void *);
typedef	void	(*tool_up)(int);

/* struct */
struct	spconf {
	char	c_domain[DOMAIN_LEN];	/* 32 */
	char	c_conbeg[CONBEG_LEN];	/* 128 */
	char	c_conend[CONEND_LEN];	/* 128 */
	char	c_conrloc[LOC_LEN];	/* 32 */
	char	c_timbeg[TIMBEG_LEN];	/* 128 */
	char	c_srcstr[SRCSTR_LEN];	/* 24 */
	char	c_charset[CHARSET_LEN];	/* 8 */
};

#endif
