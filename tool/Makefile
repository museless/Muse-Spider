# flags #
CC	= gcc
NMFLAGS = -g -Wall -I include/
OTFLAGS = -I ../include/ -I ../include/inclib -I ../include/inclib/musealgo
MSFLAGS	= -I /usr/local/include/mysql -L /usr/local/lib/mysql -lmysqlclient
PTFLAGS = -rdynamic -pthread -lz -lcrypt -lnsl -lm

# Exf
EXTOOL	= bin/sptool
EXRM	= bin/sprm
EXGA	= bin/gatool
EXFORM	= bin/format

# Obj
SRCTOOL	= src/sp_tool.c ../src/flib/mconfctl/museconfctl.c ../src/flib/sp_global.c ../src/flib/sp_pfun.c \
	  ../src/flib/sp_bufoper.c

SRCRM	= src/sp_remove.c

SRCGA	= src/sp_gather.c ../src/flib/sp_global.c ../src/flib/malgo/sort_algo.c 

SRCFORM	= src/sp_format.c ../src/flib/sp_global.c ../src/flib/sp_pfun.c

# phony
.PHONY : tool gath form rm

all	: tool gath form rm
tool	: $(EXTOOL)
rm	: $(EXRM)
gath	: $(EXGA)
form	: $(EXFORM)

$(EXTOOL) : $(SRCTOOL)
	$(CC) -o $(EXTOOL) $(SRCTOOL) $(OTFLAGS) $(NMFLAGS) $(MSFLAGS) $(PTFLAGS)

$(EXRM) : $(SRCRM)
	$(CC) -o $(EXRM) $(SRCRM) $(OTFLAGS) $(NMFLAGS)

$(EXGA) : $(SRCGA)
	$(CC) -o $(EXGA) $(SRCGA) $(OTFLAGS) $(NMFLAGS) $(MSFLAGS) $(PTFLAGS)

$(EXFORM) : $(SRCFORM)
	$(CC) -o $(EXFORM) $(SRCFORM) $(OTFLAGS) $(NMFLAGS) $(MSFLAGS) $(PTFLAGS)
