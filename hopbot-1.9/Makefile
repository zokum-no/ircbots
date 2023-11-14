#
#   HopBot   -  An experiment in Artificial Life
#   Version 1.8   
#

CFLAGS = -g 
CC = cc
RM = rm -f
CP = cp -r
INSTALL = install -s -m 700
DEFINES = 

INSTALL_NAME = /ip2/dbarberi/hopbot/hop

OBJECTS = hop-bot.o hop-ons.o misc.o 
SOURCES = hop-bot.c hop-ons.c misc.c 

all: bot 

bot: ${OBJECTS}
	${CC} ${CFLAGS} ${DEFINES} -o ${INSTALL_NAME} ${OBJECTS} ${LIBS}

.c.o: ; ${CC} ${CFLAGS} ${DEFINES} -c $<

# Dependencies
hop-bot.o: hop-bot.c hop.h  
hop-ons.o: hop-ons.c hop.h
misc.o: misc.c hop.h 

install: bot
	${INSTALL} bot ${INSTALL_NAME}

