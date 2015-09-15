# flags #
CC	= gcc
NMFLAGS	= -rdynamic -g -Wall -march=i686 -I include/inclib -I include/inclib/mmdpool -I include/ -fstack-protector-all
PTFLAGS	= -pthread -lcrypt -lnsl -lm -lc 
MYFLAGS = -L /usr/local/mysql/lib/mysql -lmysqlclient -I /usr/local/mysql/include/mysql -lz

UBUGINC = -I src/urlbug
TBUGINC = -I src/textbug
EBUGINC = -I src/extbug
OBUGINC	= -I src/outbug
OBTLINC = -I src/outbug/tools

IPCINC	= -I include/inclib/museipc
ALGOINC = -I include/inclib/musealgo

# elf #
EXKERN	= bin/kernel
EXURLB	= bin/urlbug
EXTEXTB = bin/textbug
EXEXTB	= bin/extbug
EXOUTB	= bin/outbug
EXOTST	= bin/outsort
EXORELA	= bin/outrela
EXMKDIC	= bin/mkdic

# obj #
OKERN	= src/kernel/kernel.c src/flib/sp_pfun.c src/flib/musegc/musegc.c src/flib/sp_elog.c src/flib/sp_bufoper.c \
	  src/flib/mconfctl/museconfctl.c src/flib/museato/museato.c \
	  src/flib/message/sp_socket.c src/flib/mmdpool/musemdpool.c

OURLB	= src/urlbug/sp_urlbug.c src/urlbug/sp_urlbug_db.c src/urlbug/sp_urlbug_disc.c src/urlbug/sp_urlbug_signal.c \
	  src/urlbug/sp_urlbug_clean.c src/urlbug/sp_urlbug_msg.c src/urlbug/sp_urlbug_err.c \
	  src/flib/sp_global.c src/flib/sp_bufoper.c src/flib/sp_misc.c src/flib/sp_pfun.c \
	  src/flib/sp_elog.c src/flib/sp_frame.c \
	  src/flib/chunkmpool/mpool_web.c src/flib/dboper/dboper.c src/flib/mconfctl/museconfctl.c src/flib/museato/museato.c \
	  src/flib/musegc/musegc.c src/flib/museipc/muse_sem.c src/flib/musetpool/musepthctl.c src/flib/message/sp_socket.c

OTEXTB	= src/textbug/sp_textbug.c src/textbug/sp_textbug_global.c src/textbug/sp_textbug_signal.c \
	  src/textbug/sp_textbug_clean.c src/textbug/sp_textbug_disc.c src/textbug/sp_textbug_message.c \
	  src/textbug/sp_textbug_db.c src/textbug/sp_textbug_err.c src/textbug/sp_textbug_httpstate.c \
	  src/flib/sp_global.c src/flib/sp_misc.c src/flib/sp_elog.c src/flib/sp_pfun.c src/flib/sp_frame.c \
	  src/flib/sp_bufoper.c src/flib/mmdpool/musemdpool.c src/flib/musetpool/musepthctl.c \
	  src/flib/chunkmpool/mpool_web.c src/flib/mconfctl/museconfctl.c src/flib/museato/museato.c \
	  src/flib/museipc/muse_sem.c src/flib/musegc/musegc.c src/flib/message/sp_socket.c

OEXTB	= src/extbug/sp_extbug.c src/extbug/sp_extbug_message.c src/extbug/sp_extbug_signal.c src/extbug/sp_extbug_db.c \
	  src/extbug/sp_extbug_global.c src/extbug/sp_extbug_misc.c src/extbug/sp_extbug_mmseg.c src/extbug/sp_extbug_error.c \
	  src/extbug/sp_extbug_extract.c \
	  src/flib/sp_global.c src/flib/sp_misc.c src/flib/sp_elog.c src/flib/sp_pfun.c src/flib/sp_frame.c \
	  src/flib/sp_bufoper.c src/flib/mmdpool/musemdpool.c \
	  src/flib/chunkmpool/mpool_web.c src/flib/mconfctl/museconfctl.c src/flib/museato/museato.c \
	  src/flib/museipc/muse_sem.c src/flib/musegc/musegc.c src/flib/message/sp_socket.c

OOUTB	= src/outbug/sp_outbug.c src/outbug/sp_outbug_message.c src/outbug/sp_outbug_error.c src/outbug/sp_outbug_signal.c \
	  src/outbug/sp_outbug_global.c src/outbug/sp_outbug_db.c src/outbug/sp_outbug_shm.c \
	  src/outbug/sp_outbug_filectl.c src/outbug/sp_outbug_tool.c \
	  src/flib/sp_global.c src/flib/sp_misc.c src/flib/sp_elog.c src/flib/sp_pfun.c \
	  src/flib/sp_frame.c src/flib/sp_bufoper.c \
	  src/flib/mmdpool/musemdpool.c src/flib/mconfctl/museconfctl.c src/flib/musegc/musegc.c src/flib/museato/museato.c \
	  src/flib/museipc/muse_sem.c src/flib/museipc/muse_shm.c src/flib/malgo/hash_algo.c src/flib/musetpool/musepthctl.c \
	  src/flib/message/sp_socket.c

OOTSORT = src/outbug/tools/sp_outbug_sort.c src/outbug/tools/sp_outbug_tool_frame.c \
	  src/outbug/sp_outbug_global.c src/outbug/sp_outbug_shm.c \
	  src/flib/sp_misc.c src/flib/sp_pfun.c src/flib/sp_bufoper.c \
	  src/flib/mmdpool/musemdpool.c src/flib/mconfctl/museconfctl.c src/flib/musegc/musegc.c src/flib/museato/museato.c \
	  src/flib/museipc/muse_sem.c src/flib/museipc/muse_shm.c src/flib/malgo/sort_algo.c

OOTRELA	= src/outbug/tools/sp_outbug_relate.c src/outbug/tools/sp_outbug_tool_frame.c \
	  src/outbug/sp_outbug_global.c src/outbug/sp_outbug_shm.c \
	  src/flib/sp_misc.c src/flib/sp_pfun.c src/flib/sp_global.c src/flib/sp_bufoper.c \
	  src/flib/mmdpool/musemdpool.c src/flib/mconfctl/museconfctl.c src/flib/musegc/musegc.c src/flib/museato/museato.c \
	  src/flib/museipc/muse_sem.c src/flib/museipc/muse_shm.c src/flib/malgo/hash_algo.c src/flib/musetpool/musepthctl.c

OMKDIC	= src/extbug/sp_extbug_mkdic.c \
	  src/flib/mmdpool/musemdpool.c src/flib/dboper/dboper.c src/flib/mconfctl/museconfctl.c \
	  src/flib/musegc/musegc.c src/flib/museato/museato.c \
	  src/flib/sp_global.c src/flib/sp_bufoper.c src/flib/sp_pfun.c src/flib/sp_elog.c

# phony
.PHONY : build kern txbug ubug exbug otbug otsort orela dic

build:	$(EXKERN) $(EXURLB) $(EXTEXTB) $(EXMKDIC) $(EXEXTB) $(EXOUTB) $(EXOTST) $(EXORELA)
otall:	$(EXOUTB) $(EXOTST)
kern:	$(EXKERN)
txbug:	$(EXTEXTB)
ubug:	$(EXURLB)
exbug:	$(EXEXTB)
otbug:	$(EXOUTB)
otsort:	$(EXOTST)
orela:	$(EXORELA)
dic:	$(EXMKDIC)

$(EXKERN) : $(OKERN)
	$(CC) -o $(EXKERN) $(OKERN) $(NMFLAGS)

$(EXURLB) : $(OURLB)
	$(CC) -o $(EXURLB) $(OURLB) $(NMFLAGS) $(PTFLAGS) $(MYFLAGS) $(UBUGINC) $(IPCINC)

$(EXTEXTB) : $(OTEXTB)
	$(CC) -o $(EXTEXTB) $(OTEXTB) $(NMFLAGS) $(PTFLAGS) $(MYFLAGS) $(TBUGINC) $(IPCINC)

$(EXEXTB) : $(OEXTB)
	$(CC) -o $(EXEXTB) $(OEXTB) $(NMFLAGS) $(PTFLAGS) $(MYFLAGS) $(EBUGINC) $(IPCINC)

$(EXOUTB) : $(OOUTB)
	$(CC) -o $(EXOUTB) $(OOUTB) $(NMFLAGS) $(PTFLAGS) $(MYFLAGS) $(OBUGINC) $(IPCINC) $(ALGOINC)

$(EXOTST) : $(OOTSORT)
	$(CC) -o $(EXOTST) $(OOTSORT) $(NMFLAGS) $(PTFLAGS) $(MYFLAGS) $(OBUGINC) $(EBUGINC) $(OBTLINC) $(IPCINC) $(ALGOINC)

$(EXORELA) : $(OOTRELA)
	$(CC) -o $(EXORELA) $(OOTRELA) $(NMFLAGS) $(PTFLAGS) $(MYFLAGS) $(OBUGINC) $(EBUGINC) $(OBTLINC) $(IPCINC) $(ALGOINC)

$(EXMKDIC) : $(OMKDIC)
	$(CC) -o $(EXMKDIC) $(OMKDIC) $(NMFLAGS) $(MYFLAGS)
