#
# IncuBot 1.02  (Based on ChRoniCbot)
#
# These defines are supposed to work on Linux, Ultrix and SunOs 4.1.*
# with a good ansi-C compiler (i.e. gcc)
#
#CFLAGS = -g  # for debugging with gdb/dbx
CFLAGS = -O2 -s  -Wall
CC = gcc
RM = rm -f
CP = cp -r
INSTALL = install -s -m 700
DEFINES = #-DFBSD #-DHPUX #-DSYSV #-DULTRIX
LIBS = #-lsocket -lnsl #for SYSV

# If its a FreeBSD system and the compiler says a system header file is corrupt...
# DEFINES = -DFBSD
# LIBS =

# Use these for unixware (thanks to Reichert) (chronic update)
# DEFINES = -DSYSV
# LIBS = -lresolv -lsocket -lnsl

# Use these for AIX-3.2.5:
# CC = cc
# CFLAGS = -O
# INSTALL = /usr/ucb/install -s -m 700
# DEFINES = -DAIX

# And these should make vladbot compile cleanly on Next
#CC = cc
#CFLAGS = -O2 -s -Wall
#DEFINES = -DNEXT -D_POSIX_SOURCE

#
# It seems that on Solaris some libraries have to be linked with the bot.
# If anyone knows which, pls mail me
#
# Perhaps something like
# INCLUDES = -I/usr/ucbinclude -L/usr/ucblib
# LIBS = -lucb

INSTALL_DIR = ..
INSTALL_NAME = incubot.new

#
# Don't change anything below here...
#

OBJECTS = main.o vladbot.o cfgfile.o parse.o vlad-ons.o file.o misc.o send.o \
          ctcp.o server.o channel.o chanuser.o userlist.o dcc.o \
          ftext.o log.o debug.o session.o ftp_dcc.o fnmatch.o parsing.o \
          tb-ons.o crc-ons.o botnet.o incu-ons.o shitlist.o
SOURCES = main.c vladbot.c cfgfile.c parse.c vlad-ons.c file.c misc.c send.c \
	  ctcp.c server.c channel.c chanuser.o userlist.c dcc.c \
	  ftext.c log.c debug.c session.c ftp_dcc.c fnmatch.c parsing.c \
	  tb-ons.c crc-ons.c botnet.c incu-ons.c shitlist.c

all: bot help mailer
	@echo 'IMPORTANT: read the file doc/INSTALLING and doc/Version.incubus'
.c.o:
	${CC} ${CFLAGS} ${DEFINES} -c $<

hr_ftext.o:
	${CC} ${CFLAGS} ${DEFINES} ${INCLUDES} \
	-DSTAND_ALONE -c -o hr_ftext.o ftext.c

bot: ${OBJECTS}
	${CC} ${CFLAGS} ${DEFINES} ${INCLUDES} -o ../${INSTALL_NAME} ${OBJECTS} ${LIBS}

help: helpread.o hr_ftext.o 
	${CC} ${CFLAGS} ${DEFINES} -DSTAND_ALONE -o ../helpread helpread.o hr_ftext.o

maintainer:
	${CC} ${DEFINES} -DBOT_FNAME=\"${INSTALL_NAME}\" \
	-o ../maintainer maintainer.c

mailer: 
	${CC} ${CFLAGS} ${DEFINES} -DSTAND_ALONE -o ../maillog maillog.c

install: all
	${INSTALL} ../${INSTALL_NAME}  ${INSTALL_DIR}/${INSTALL_NAME}
	${INSTALL} ../helpread  ${INSTALL_DIR}/helpread
#	cp help.bot ${INSTALL_DIR}/help.bot
	@echo Don\'t forget to create/copy the \*.list, help.bot and infile.cfg files!

dist: distribution

distribution:
	cd ..; \
	tar -cf chronic.tar source/Makefile source/*.c source/*.h \
		*.list *.list2 help.bot \
                *.cfg *.notify botcheck doc/*; \
	gzip -9 chronic.tar; \
	cd source

index:
	cat ../tmp/.index1 ../Version | fmt -80 > ../tmp/.tmp
	cat ../tmp/.tmp ../tmp/.index2 > ../tmp/.index
	rm ../tmp/.tmp

clean:
	${RM} bot ${INSTALL_NAME} helpread ../${INSTALL_NAME} ../helpread \
	maintainer ../maintainer *.o *.bak core 

# Dependencies

cfgfile.o : cfgfile.c cfgfile.h vladbot.h session.h config.h userlist.h channel.h \
  chanuser.h dcc.h debug.h misc.h parsing.h 
channel.o : channel.c channel.h chanuser.h config.h fnmatch.h misc.h send.h \
  userlist.h vladbot.h session.h dcc.h 
chanuser.o : chanuser.c chanuser.h config.h misc.h 
ctcp.o : ctcp.c config.h ctcp.h debug.h misc.h send.h vladbot.h session.h userlist.h \
  channel.h chanuser.h dcc.h 
dcc.o : dcc.c config.h dcc.h debug.h log.h misc.h send.h server.h session.h \
  vladbot.h userlist.h channel.h chanuser.h botnet.h
debug.o : debug.c vladbot.h session.h config.h userlist.h channel.h chanuser.h \
  dcc.h debug.h 
file.o : file.c file.h userlist.h config.h 
fnmatch.o : fnmatch.c fnmatch.h 
ftext.o : ftext.c vladbot.h session.h config.h userlist.h channel.h chanuser.h \
  dcc.h misc.h 
ftp_dcc.o : ftp_dcc.c config.h debug.h fnmatch.h ftp_dcc.h log.h misc.h \
  session.h vladbot.h userlist.h channel.h chanuser.h dcc.h 
helpread.o : helpread.c ftext.h config.h 
log.o : log.c dcc.h config.h vladbot.h session.h userlist.h channel.h \
  chanuser.h crc-ons.h log.h 
main.o : main.c channel.h chanuser.h config.h debug.h log.h parse.h session.h \
  userlist.h vladbot.h dcc.h
misc.o : misc.c config.h misc.h
parse.o : parse.c channel.h chanuser.h config.h ctcp.h debug.h log.h misc.h \
  parse.h send.h vladbot.h session.h userlist.h dcc.h vlad-ons.h crc-ons.h \
  botnet.h
parsing.o : parsing.c parsing.h misc.h config.h 
send.o : send.c dcc.h config.h send.h debug.h 
server.o : server.c config.h debug.h vladbot.h session.h userlist.h channel.h \
  chanuser.h dcc.h 
session.o : session.c config.h misc.h send.h session.h vladbot.h userlist.h \
  channel.h chanuser.h dcc.h vlad-ons.h crc-ons.h
userlist.o : userlist.c config.h fnmatch.h misc.h send.h userlist.h 
vlad-ons.o : vlad-ons.c channel.h chanuser.h config.h dcc.h debug.h ftext.h \
  ftp_dcc.h misc.h parsing.h send.h session.h userlist.h vladbot.h vlad-ons.h \
  tb-ons.h crc-ons.h botnet.h incu-ons.h
tb-ons.o : tb-ons.c channel.h chanuser.h config.h dcc.h debug.h ftext.h \
  ftp_dcc.h misc.h parsing.h send.h session.h userlist.h vladbot.h vlad-ons.h \
  crc-ons.h botnet.h tb-ons.h
vladbot.o : vladbot.c channel.h chanuser.h config.h dcc.h debug.h file.h \
  userlist.h misc.h parse.h send.h server.h vladbot.h session.h
crc-ons.o : crc-ons.c config.h channel.h chanuser.h userlist.h misc.h \
  fnmatch.h send.h vladbot.h crc-ons.h vlad-ons.h
botnet.o : botnet.c config.h dcc.h fnmatch.h ftext.h misc.h send.h crc-ons.h \
  vladbot.h vlad-ons.h botnet.h
