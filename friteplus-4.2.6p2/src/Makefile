#
# Makefile for fRitE+ v4.3
#

# These defines are supposed to work on Linux, Ultrix and SunOs 4.1.*
# with a good ansi-C compiler (i.e. gcc)
#
#CFLAGS = -O  # if you want optimization
#CFLAGS = -g -Wall -Wcast-align # if you want to debug
CFLAGS = -O6 -s
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
#CFLAGS = -O6 -s
#

#
# It seems that on Solaris some libraries have to be linked with the bot.
# If anyone knows which, pls mail me
#
#
# Don't change anything below here...
#
INSTALL_DIR = 
INSTALL_NAME = bot

#
# Don't change anything below here...
#
# I added VERSION_LABEL which is used by the distribution make section
# Pyber
VERSION_LABEL = v4.2__BETA__

OBJECTS = main.o frite_bot.o cfgfile.o parse.o frite_add.o file.o misc.o send.o \
          ctcp.o server.o channel.o chanuser.o userlist.o dcc.o \
          ftext.o log.o note.o debug.o session.o ftp_dcc.o fnmatch.o \
	  bonk.o greeting.o publics.o function.o frite_add2.o \
          email.o
SOURCES = main.c frite_bot.c cfgfile.c parse.c frite_add.c file.c misc.c send.c \
	  ctcp.c server.c channel.c chanuser.o userlist.c dcc.c \
	  ftext.c log.c note.c debug.c session.c ftp_dcc.c fnmatch.c \
	  bonk.c greeting.c publics.c function.c frite_add2.c \
          email.c

all: bot

.c.o: ; ${CC} ${CFLAGS} ${DEFINES} -c $<
hr_ftext.o: ; ${CC} ${CFLAGS} ${DEFINES} -DSTAND_ALONE -c -o hr_ftext.o ftext.c

bot: ${OBJECTS}
	${CC} ${CFLAGS} ${DEFINES} -o ${INSTALL_NAME} ${OBJECTS} ${LIBS}

install: all
	${INSTALL} ${INSTALL_NAME}  ${INSTALL_DIR}/${INSTALL_NAME}
	cp help.bot ${INSTALL_DIR}/help.bot
	@echo Don\'t forget to create/copy the \*.list, help.bot and infile.cfg files!

#distribution:
#	tar -cf frite_bot.tar *.c *.h *.list *.bot *.cfg *.pic *.song Makefile
#	gzip -9 frite_bot.tar
# I dicked out the original make disibutions..  It wasn't working anyways...
# Pyber
#
#
distribution:
	@echo This will DELETE the files ${INSTALL_NAME} \*.o \*.bak core.
	@echo If you are sure, use make Distribution instead

Distribution:
	${RM} ${INSTALL_NAME} *.o *~ *.bak core
	tar -pzcvf fRitE+_v${VERSION_LABEL}.tgz *

clean:
	${RM} bot ${INSTALL_NAME} helpread *.o *.bak core 

# Dependencies
bonk.o : config.h send.h frite_add.h misc.h function.h session.h frite_bot.h
cfgfile.o : cfgfile.c frite_bot.h session.h config.h userlist.h channel.h chanuser.h \
  dcc.h misc.h cfgfile.h 
channel.o : channel.c chanuser.h config.h channel.h userlist.h frite_bot.h session.h \
  dcc.h misc.h war.h
chanuser.o : chanuser.c chanuser.h config.h misc.h 
ctcp.o : ctcp.c misc.h ctcp.h debug.h frite_bot.h session.h config.h userlist.h \
  channel.h chanuser.h dcc.h 
dcc.o : dcc.c session.h config.h dcc.h misc.h debug.h frite_bot.h userlist.h channel.h \
  chanuser.h 
debug.o : debug.c frite_bot.h session.h config.h userlist.h channel.h chanuser.h \
  dcc.h debug.h 
email.o : email.c email.h frite_bot.h session.h config.h
file.o : file.c file.h userlist.h config.h 
fnmatch.o : fnmatch.c fnmatch.h 
ftext.o : ftext.c frite_bot.h session.h config.h userlist.h channel.h chanuser.h \
  dcc.h misc.h 
ftp_dcc.o : ftp_dcc.c config.h debug.h fnmatch.h ftp_dcc.h misc.h session.h \
  frite_bot.h userlist.h channel.h chanuser.h dcc.h 
greeting.o : send.h frite_add.h config.h misc.h function.h fnmatch.h frite_bot.h
helpread.o : helpread.c ftext.h config.h 
log.o : log.c dcc.h config.h frite_bot.h session.h userlist.h channel.h chanuser.h \
  log.h 
main.o : main.c config.h log.h session.h userlist.h channel.h chanuser.h parse.h \
  debug.h frite_bot.h dcc.h 
match.o : match.c fnmatch.h 
misc.o : misc.c misc.h config.h 
note.o : note.c note.h config.h frite_bot.h session.h userlist.h channel.h chanuser.h \
  dcc.h misc.h 
parse.o : parse.c parse.h frite_add.h channel.h chanuser.h config.h debug.h misc.h \
  frite_bot.h session.h userlist.h dcc.h 
path.o : path.c misc.h 
publics.o : publics.c config.h publics.h
send.o : send.c dcc.h config.h send.h debug.h 
server.o : server.c debug.h frite_bot.h session.h config.h userlist.h channel.h \
  chanuser.h dcc.h 
session.o : session.c frite_bot.h session.h config.h userlist.h channel.h chanuser.h \
  dcc.h frite_add.h misc.h 
userlist.o : userlist.c userlist.h config.h misc.h 
frite_add.o : frite_add.c channel.h chanuser.h config.h dcc.h debug.h ftext.h \
  ftp_dcc.h misc.h note.h session.h userlist.h frite_bot.h frite_add.h war.h
frite_add2.o : frite_add2.c channel.h chanuser.h config.h dcc.h debug.h ftext.h \
  ftp_dcc.h misc.h note.h session.h userlist.h frite_bot.h frite_add.h war.h
frite_bot.o : frite_bot.c channel.h chanuser.h config.h userlist.h file.h frite_bot.h \
  session.h dcc.h debug.h misc.h 

