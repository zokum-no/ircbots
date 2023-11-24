#
# ComBot 2.05
#

DEFINES=

#CFLAGS = -O2  # if you want optimization
CFLAGS = -g  # if you want to debug   -pg for profile

# SOLARIS:
#LIBS = -lnsl -lsocket
#DEFINES += -DNEED_BCOPY

# SOLARIS: (if the above doesn't work =)
#LIBS = -lnsl -lsocket -ltermcap -L/usr/ucblib -lucb
#DEFINES += -DNEED_DCOPY

# AIX:
#DEFINES += -DAIX

# HPUX:
#DEFINES += -DHPUX

CC = gcc

#
# Don't change anything below here...
#

RM = rm -f
CP = cp -r
INSTALL = install -s -m 700

INSTALL_DIR = ..
INSTALL_NAME = ComBot

INCLUDES = ../include

ALL_H  = ${INCLUDES}/config.h ${INCLUDES}/defines.h ${INCLUDES}/structs.h \
	${INCLUDES}/global.h ${INCLUDES}/h.h

OBJECTS = main.o combot.o cfgfile.o parse.o com-ons.o fileio.o send.o \
          ctcp.o server.o channel.o chanuser.o userlist.o dcc.o function.o\
          log.o debug.o memory.o match.o commands.o vars.o structs.o
SOURCES = main.c combot.c cfgfile.c parse.c com-ons.c fileio.c send.c \
	  ctcp.c server.c channel.c chanuser.o userlist.c dcc.c function.c\
	  log.c debug.c memory.c match.c commands.c vars.c structs.c

all: bot sendmsg

.c.o: ; ${CC} ${CFLAGS} ${DEFINES} -I${INCLUDES} -c $<
hr_ftext.o: ; ${CC} ${CFLAGS} ${DEFINES} -DSTAND_ALONE -c -o hr_ftext.o ftext.c

bot: ${OBJECTS}
	${CC} ${CFLAGS} ${DEFINES} -o ${INSTALL_NAME} ${OBJECTS} ${LIBS}

sendmsg: sendmsg.o
	${CC} ${CFLAGS} ${DEFINES} -o sendmsg sendmsg.o ${LIBS}

$(INSTALL_NAME)_pure: ${OBJECTS}
	purify -first-only -g++=yes -chain-length=15 -inuse-at-exit -max_threads=40 ${CC} ${CFLAGS} ${DEFINES} -o ${INSTALL_NAME}_pure ${OBJECTS} ${LIBS}


# Dependencies

sendmsg.o: sendmsg.c ${INCLUDES}/config.h
cfgfile.o: cfgfile.c ${ALL_H}
dcc.o: dcc.c ${ALL_H}
combot.o: combot.c ${ALL_H}
parse.o: parse.c ${ALL_H}
userlist.o: userlist.c ${ALL_H}
channel.o: channel.c ${ALL_H}
debug.o: debug.c ${ALL_H}
log.o: log.c ${ALL_H}
send.o: send.c ${ALL_H}
chanuser.o: chanuser.c ${ALL_H}
fileio.o: fileio.c ${ALL_H}
main.o: main.c ${ALL_H}
server.o: server.c ${ALL_H}
commands.o: commands.c ${ALL_H}
function.o: function.c ${ALL_H}
match.o: match.c ${ALL_H}
sets.o: sets.c ${ALL_H}
ctcp.o: ctcp.c ${ALL_H}
com-ons.o: com-ons.c ${ALL_H}
memory.o: memory.c ${ALL_H}
toggles.o: toggles.c ${ALL_H}
structs.o: structs.c ${ALL_H}
