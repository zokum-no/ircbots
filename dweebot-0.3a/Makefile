#
# DweeB0t 0.3A
#


# These defines are supposed to work on Linux, Ultrix and SunOs 4.1.*
# with a good ansi-C compiler (i.e. gcc)
#
#CFLAGS = -O2  # if you want optimization
CFLAGS = -g2   # if you want to debug
CC = gcc
RM = rm -f
CP = cp -r
INSTALL = install -s -m 700
DEFINES = 

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
# CFLAGS = -O6 -s
#

#
# It seems that on Solaris some libraries have to be linked with the bot.
# If anyone knows which, pls mail me
#

INSTALL_DIR = ..
INSTALL_NAME = Dweebie

#
# Don't change anything below here...
#

OBJECTS = main.o dweeb.o cfgfile.o parse.o d-ons.o fileio.o send.o \
          ctcp.o server.o channel.o chanuser.o userlist.o dcc.o function.o\
          log.o debug.o memory.o fnmatch.o \
	  commands.o sets.o toggles.o
SOURCES = main.c dweeb.c cfgfile.c parse.c d-ons.c fileio.c send.c \
	  ctcp.c server.c channel.c chanuser.o userlist.c dcc.c function.c\
	  log.c debug.c memory.c fnmatch.c commands.c sets.c toggles.c

all: bot

.c.o: ; ${CC} ${CFLAGS} ${DEFINES} -c $<
hr_ftext.o: ; ${CC} ${CFLAGS} ${DEFINES} -DSTAND_ALONE -c -o hr_ftext.o ftext.c

bot: ${OBJECTS}
	${CC} ${CFLAGS} ${DEFINES} -o ${INSTALL_NAME} ${OBJECTS} ${LIBS}

clean:
	${RM} bot ${INSTALL_NAME} helpread *.o *.bak core 

# Dependencies

cfgfile.o : cfgfile.c dweeb.h memory.h config.h userlist.h channel.h \
            chanuser.h dcc.h function.h cfgfile.h 
channel.o : channel.c chanuser.h config.h channel.h userlist.h dweeb.h \
            memory.h dcc.h function.h
chanuser.o : chanuser.c chanuser.h config.h function.h

ctcp.o : ctcp.c function.h ctcp.h debug.h dweeb.h memory.h config.h \
         userlist.h channel.h chanuser.h dcc.h 
dcc.o : dcc.c memory.h config.h dcc.h function.h debug.h dweeb.h userlist.h \
        channel.h chanuser.h 
debug.o : debug.c dweeb.h memory.h config.h userlist.h channel.h \
          chanuser.h dcc.h debug.h 
fileio.o : fileio.c fileio.h userlist.h config.h 

fnmatch.o : fnmatch.c fnmatch.h 

log.o : log.c dcc.h config.h dweeb.h memory.h userlist.h channel.h \
        chanuser.h log.h 
main.o : main.c config.h log.h memory.h userlist.h channel.h chanuser.h \
         parse.h debug.h dweeb.h dcc.h 

function : misc.c misc.h config.h 

parse.o : parse.c parse.h d-ons.h channel.h chanuser.h config.h debug.h \
          function.h dweeb.h memory.h userlist.h dcc.h 
send.o : send.c dcc.h config.h send.h debug.h 
server.o : server.c debug.h dweeb.h memory.h config.h userlist.h channel.h \
           chanuser.h dcc.h 
memory.o : memory.c dweeb.h memory.h config.h userlist.h channel.h \
           chanuser.h dcc.h d-ons.h function.h 
userlist.o : userlist.c userlist.h config.h function.h 
d-ons.o : d-ons.c channel.h chanuser.h config.h dcc.h debug.h \
             function.h memory.h userlist.h dweeb.h d-ons.h 
dweeb.o : dweeb.c channel.h chanuser.h config.h userlist.h fileio.h \
            dweeb.h memory.h dcc.h debug.h function.h 


