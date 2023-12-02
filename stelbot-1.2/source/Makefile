#
# StelBot Version 1.1
#


CFLAGS = -O2
#CFLAGS = 
#LFLAGS = -lsocket -ltermcap
#CFLAGS = -g

# use a good ANSI-C compiler
CC = gcc
RM = rm -f
CP = cp -r
INSTALL = install -s -m 700
DEFINES = 

INSTALL_DIR = .
INSTALL_NAME = StelBot

OBJECTS = main.o stelbot.o parse.o commands.o function.o sets.o toggles.o \
	  misc.o send.o match.o ctcp.o \
	  server.o channel.o chanuser.o userlist.o dcc.o ftext.o log.o
SOURCES = main.c stelbot.c parse.c commands.c function.c sets.c toggles.c \
	  misc.c send.c match.c ctcp.c \
	  server.c channel.c chanuser.c userlist.c dcc.c ftext.c log.c

all: bot

bot: ${OBJECTS}
	${CC} ${CFLAGS} ${LFLAGS} ${DEFINES} -o ${INSTALL_NAME} ${OBJECTS} ${LIBS}

.c.o: ; ${CC} ${CFLAGS} ${DEFINES} -c $<

ComBot:
channel.o: 	channel.c chanuser.h config.h channel.h userlist.h 
chanuser.o: 	chanuser.c chanuser.h config.h 
ctcp.o: 	ctcp.c ctcp.h config.h 
dcc.o: 		dcc.c dcc.h config.h misc.h 
ftext.o: 	ftext.c config.h 
log.o: 		log.c config.h
main.o: 	main.c config.h userlist.h channel.h chanuser.h parse.h
match.o: 	match.c 
misc.o: 	misc.c misc.h config.h 
parse.o: 	parse.c parse.h commands.h channel.h chanuser.h config.h misc.h 
send.o: 	send.c config.h 
server.o: 	server.c config.h
userlist.o: 	userlist.c userlist.h config.h 
commands.o: 	commands.c sets.o toggles.o function.o config.h channel.h chanuser.h userlist.h dcc.h \
		ftext.h misc.h commands.h 
stelbot.o: 	stelbot.c channel.h chanuser.h config.h 

