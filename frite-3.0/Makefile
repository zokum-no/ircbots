#
# VladBot - v2.10
#


# These defines are supposed to work on Linux, Ultrix and SunOs 4.1.*
# with a good ansi-C compiler (i.e. gcc)
#
CFLAGS = -O  # if you want optimization
#CFLAGS = -g #-O -Wall -Wcast-align # if you want to debug
CC = gcc
RM = rm -f
CP = cp -r
INSTALL = install -s -m 700
DEFINES = 

# Dathes send me this stuff for AIX:
#
# use these for AIX-3.2.5
# first add a stanza to /etc/xlc.cfg like this:
# bsdcc: use		= DEFLT
# 	 crt 		= /lib/crt0.o
#	 mcrt 		= /lib/mcrt0.o
#	 gcrt 		= /lib/gcrt0.o
#	 libraries	= -lbds,-lc
#	 proflibs	= -L/lib/profiled,-L/usr/lib/profiled
#	 options	= -H512,-T512,-qlanglvl=extended,-qnoro,\
#			  -D_BSD,-DBSD=43,-D_NONSTD_TYPES,-D_NO_PROTO,\
#			  -D_BSD_INCLUDES,-dnodelcsect,-U__STR__,-U__NATH__
#
# Then use these:
# CC = bsdcc
# CFLAGS = -O
# INSTALL = /usr/ucb/install -s -m 700
# DEFINES = -DAIX

#
# On SunOs and Linux, this'll make your binary a little smaller and more
# optimized:
#
CFLAGS = -O6 -s
#

#
# It seems that on Solaris some libraries have to be linked with the bot.
# If anyone knows which, pls mail me
#

INSTALL_DIR = 
INSTALL_NAME = bot

#
# Don't change anything below here...
#

OBJECTS = main.o vladbot.o cfgfile.o parse.o vlad-ons.o file.o misc.o send.o \
          ctcp.o server.o channel.o chanuser.o userlist.o dcc.o \
          ftext.o log.o note.o debug.o session.o ftp_dcc.o fnmatch.o \
	  bonk.o greeting.o publics.o actions.o news.o chanlog.o \
	  discquote.o sing.o spy.o ascii_lib.o function.o dance.o \
	  8ball.o inform.o kicksay.o splatk.o
SOURCES = main.c vladbot.c cfgfile.c parse.c vlad-ons.c file.c misc.c send.c \
	  ctcp.c server.c channel.c chanuser.o userlist.c dcc.c \
	  ftext.c log.c note.c debug.c session.c ftp_dcc.c fnmatch.c \
	  bonk.c greeting.c publics.c actions.c news.c chanlog.c \
	  discquote.c sing.c spy.c ascii_lib.c function.c dance.c \
	  8ball.c inform.c kicksay.c splatk.c

all: bot help 

.c.o: ; ${CC} ${CFLAGS} ${DEFINES} -c $<
hr_ftext.o: ; ${CC} ${CFLAGS} ${DEFINES} -DSTAND_ALONE -c -o hr_ftext.o ftext.c

bot: ${OBJECTS}
	${CC} ${CFLAGS} ${DEFINES} -o ${INSTALL_NAME} ${OBJECTS} ${LIBS}

help:   helpread.o hr_ftext.o 
	${CC} ${CFLAGS} ${DEFINES} -DSTAND_ALONE -o helpread helpread.o hr_ftext.o



install: all
	${INSTALL} ${INSTALL_NAME}  ${INSTALL_DIR}/${INSTALL_NAME}
	cp help.bot ${INSTALL_DIR}/help.bot
	@echo Don\'t forget to create/copy the \*.list, help.bot and infile.cfg files!

distribution:
	tar -cf vladbot.tar *.c *.h *.list *.bot *.cfg *.pic *.song Makefile
	gzip -9 vladbot.tar

clean:
	${RM} bot ${INSTALL_NAME} helpread *.o *.bak core 

# Dependencies

cfgfile.o : cfgfile.c vladbot.h session.h config.h userlist.h channel.h chanuser.h \
  dcc.h misc.h cfgfile.h 
channel.o : channel.c chanuser.h config.h channel.h userlist.h vladbot.h session.h \
  dcc.h misc.h 
chanuser.o : chanuser.c chanuser.h config.h misc.h 
ctcp.o : ctcp.c misc.h ctcp.h debug.h vladbot.h session.h config.h userlist.h \
  channel.h chanuser.h dcc.h 
dcc.o : dcc.c session.h config.h dcc.h misc.h debug.h vladbot.h userlist.h channel.h \
  chanuser.h 
debug.o : debug.c vladbot.h session.h config.h userlist.h channel.h chanuser.h \
  dcc.h debug.h 
file.o : file.c file.h userlist.h config.h 
fnmatch.o : fnmatch.c fnmatch.h 
ftext.o : ftext.c vladbot.h session.h config.h userlist.h channel.h chanuser.h \
  dcc.h misc.h 
ftp_dcc.o : ftp_dcc.c config.h debug.h fnmatch.h ftp_dcc.h misc.h session.h \
  vladbot.h userlist.h channel.h chanuser.h dcc.h 
helpread.o : helpread.c ftext.h config.h 
log.o : log.c dcc.h config.h vladbot.h session.h userlist.h channel.h chanuser.h \
  log.h 
main.o : main.c config.h log.h session.h userlist.h channel.h chanuser.h parse.h \
  debug.h vladbot.h dcc.h 
match.o : match.c fnmatch.h 
misc.o : misc.c misc.h config.h 
note.o : note.c note.h config.h vladbot.h session.h userlist.h channel.h chanuser.h \
  dcc.h misc.h 
parse.o : parse.c parse.h vlad-ons.h channel.h chanuser.h config.h debug.h misc.h \
  vladbot.h session.h userlist.h dcc.h 
path.o : path.c misc.h 
send.o : send.c dcc.h config.h send.h debug.h 
server.o : server.c debug.h vladbot.h session.h config.h userlist.h channel.h \
  chanuser.h dcc.h 
session.o : session.c vladbot.h session.h config.h userlist.h channel.h chanuser.h \
  dcc.h vlad-ons.h misc.h 
userlist.o : userlist.c userlist.h config.h misc.h 
vlad-ons.o : vlad-ons.c channel.h chanuser.h config.h dcc.h debug.h ftext.h \
  ftp_dcc.h misc.h note.h session.h userlist.h vladbot.h vlad-ons.h 
vladbot.o : vladbot.c channel.h chanuser.h config.h userlist.h file.h vladbot.h \
  session.h dcc.h debug.h misc.h 
