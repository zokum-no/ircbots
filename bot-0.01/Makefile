CFLAGS = -g 
CC = cc
RM = rm -f
CP = cp -r
INSTALL = install -s -m 700
DEFINES = 

INSTALL_NAME = /ua/gt1/gt1679c/bots/bot/bot

OBJECTS = bot.o ons.o
SOURCES = bot.c ons.c

all: bot 

bot: ${OBJECTS}
	${CC} ${CFLAGS} ${DEFINES} -o ${INSTALL_NAME} ${OBJECTS} ${LIBS}

.c.o: ; ${CC} ${CFLAGS} ${DEFINES} -c $<

# Dependencies
ons.o: ons.c bot.h
bot.o: bot.c bot.h

install: bot
	${INSTALL} bot ${INSTALL_NAME}

