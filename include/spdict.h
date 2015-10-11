#ifndef	_SPDICT_H
#define	_SPDICT_H

/*---------------------
	define
-----------------------*/

#define	FSCEL_WORD_OFF		0x2628

#define	UTF8_WORD_LEN		0x3
#define	UTF8_WORD_CNT		0x1200
#define	WORD_LEN_LIMIT		0xF

#define	UTF8_BUF_SIZE		0x30
#define	MAX_UTF8_LEN		UTF8_BUF_SIZE
#define	MAX_UNI_LEN		0x20

#define	U8WORD_DEBUG		0x0

#define	INDEX_STRING		"%.3s\t%d\t%d\n"

/* mempool */
#define	PER_MP_BLOCK_SIZE	0x1000

/* arg list */
#define	LEVEL_BITS		0x4
#define	LEVEL_MASK		0x0000000F

/* level one */
#define	DR_WRONG		0xF
#define	DR_ADD			0x1
#define	DR_DOWN			0x2
#define	DR_RELIST		0x3
#define	DR_MSCRIPT		0x4
#define	DR_DELETE		0x5

/* level two */
#define	DR_ALL_PHRASE		0x10
#define	DR_NOUN_PHRASE		0x20

#define	LEVEL_TWO_MASK		0x000000F0

/* level three */
#define	DR_NORMAL_FILE		0x100
#define	DR_SCEL_FILE		0x200

#define	LEVEL_THREE_MASK	0x00000F00

/* level four */
#define	DR_NO_STATE		0xF
#define	DR_IN_STATE		0x0
#define	DR_UNDETE_STATE		0x1
#define	DR_NOUN_STATE		0x2
#define	DR_VERB_STATE		0x3

#define	DR_WITHOUT_STATE	0x1
#define	DR_WITH_STATE		0x2
#define	DR_WITH_TIMES		0x3

#define	LEVEL_FOUR_MASK		0x0000F000
#define	LEVEL_FOUR_SHIFT	0xC

/* paramters limit */
#define	ADD_LIMIT		0x4
#define	DOWN_LIMIT		0x3
#define	RELIST_LIMIT		0x2

/*---------------------
	typedef
-----------------------*/
typedef	struct	dword	WORDS;
typedef	struct	wlist	WLISTS;
typedef	struct	wstr	WSTRS;
typedef	struct	wstr*	PWSTRS;

typedef	void	(*exfun)(int);
typedef	int	(*spfun)(char *, int, char *, int);
typedef	int	(*dnfun)(char *, MYSQL_ROW);

typedef	struct	dindex	DCONT;

typedef	struct	mkrun	MKRUN;

/* struct */
struct	dword {
	uChar	w_utf8[UTF8_WORD_LEN];
	short	w_smax;			/* the max word len */
};

struct	wlist {
	void	*w_n2;
	void	*w_n3;
	void	*w_n4;
	void	*w_n5;
	void	*w_n6;
	void	*w_n7;
	void	*w_n8;
	void	*w_n9;
	void	*w_n10;
	void	*w_n11;
	void	*w_n12;
	void	*w_n13;
	void	*w_n14;
	void	*w_n15;
	void	*w_n16;
};

struct	wstr {
	WSTRS	*w_next;
	uChar	*w_str;
};

struct	dindex {
	char	dc_utf8[UTF8_WORD_LEN];
	int	dc_off;
	int	dc_cnt;
};

struct	mkrun {
	int	ar_state;

	union {
		int	ar_cutlen;
		int	ar_instate;	/* when ar_state == DR_IN_STATE */

		char	*ar_selesent;	/* use in down part */
	};

	union {
		exfun	ar_extract;	/* for add part */
		dnfun	ar_dnfun;	/* for down part */
	};

	/* for add part */
	spfun	ar_spbeg;
	spfun	ar_spnext;
};

#endif
