#
# VladBot - v2.00
#
CFLAGS = -O6 -s
#CFLAGS = -g

# use a good ANSI-C compiler
CC = gcc
RM = rm -f
CP = cp -r
INSTALL = install -s -m 700
DEFINES = 

INSTALL_DIR = ./test
INSTALL_NAME = NederServ

OBJECTS = main.o vladbot.o parse.o vlad-ons.o file.o misc.o send.o match.o \
	  ctcp.o server.o channel.o chanuser.o userlist.o dcc.o ftext.o log.o
SOURCES = main.c vladbot.c parse.c vlad-ons.c file.c misc.c send.c match.c \
	  ctcp.c server.c channel.c chanuser.o userlist.c dcc.c ftext.c log.c

all: bot help

bot: ${OBJECTS}
	${CC} ${CFLAGS} ${DEFINES} -o ${INSTALL_NAME} ${OBJECTS} ${LIBS}

.c.o: ; ${CC} ${CFLAGS} ${DEFINES} -c $<

help:   helpread.o helpread.o
	${CC} ${CFLAGS} ${DEFINES} -o helpread helpread.c ftext.c

install: all
	${INSTALL} ${INSTALL_NAME}  ${INSTALL_DIR}/${INSTALL_NAME}
	cp help.bot ${INSTALL_DIR}/help.bot
	@echo Don\'t forget to create/copy the \*.list files!

distribution:
	tar -cf vladbot.tar *.c *.h *.list Makefile README vladbot.1
	gzip -9 vladbot.tar

clean:
	${RM} *.o bot core 2>/dev/null

# Dependencies
NederServ:	
channel.o: 	channel.c chanuser.h config.h channel.h userlist.h 
chanuser.o: 	chanuser.c chanuser.h config.h 
ctcp.o: 	ctcp.c ctcp.h config.h 
dcc.o: 		dcc.c dcc.h config.h misc.h 
file.o: 	file.c file.h userlist.h config.h 
ftext.o: 	ftext.c config.h 
log.o: 		log.c config.h
main.o: 	main.c config.h userlist.h channel.h chanuser.h parse.h 
match.o: 	match.c 
misc.o: 	misc.c misc.h config.h 
parse.o: 	parse.c parse.h vlad-ons.h channel.h chanuser.h config.h misc.h 
send.o: 	send.c config.h 
server.o: 	server.c config.h
userlist.o: 	userlist.c userlist.h config.h 
vlad-ons.o: 	vlad-ons.c config.h channel.h chanuser.h userlist.h dcc.h \
		ftext.h misc.h vlad-ons.h 
vladbot.o: 	vladbot.c channel.h chanuser.h config.h 
