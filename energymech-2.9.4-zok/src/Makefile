#
#   EnergyMech, IRC bot software
#   Copyright (c) 1997-2001 proton, 2002-2004 emech-dev
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program; if not, write to the Free Software
#   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#

INSTALLNAME =	mech
INSTALLDIR =	..
INSTALLMODE =	0700

CPROF =		
LPROF =		
LIBS =		 
CFLAGS =	-pipe -g -Wshadow -Wall -O2 -c
LFLAGS =	-g -o
CC =		/usr/bin/gcc

MV =		/usr/bin/mv
RM =		/usr/bin/rm
CHMOD =		/usr/bin/chmod
OBJCOPY =	/usr/bin/objcopy

INCS =		config.h mcmd.h defines.h global.h h.h structs.h text.h

OBJS =		cfgfile.o channel.o com-ons.o combot.o commands.o dcc.o debug.o function.o \
		link.o main.o parse.o socket.o userlist.o vars.o xmech.o trivia.o

CFILES =	cfgfile.c channel.c com-ons.c combot.c commands.c dcc.c debug.c function.c \
		link.c main.c parse.c socket.c userlist.c vars.c xmech.c trivia.c

all:		$(INSTALLNAME)

gencmd:		gencmd.c config.h
		$(CC) $(LFLAGS) gencmd gencmd.c

mcmd.h:		gencmd
		./gencmd > mcmd.h

install:	$(INSTALLNAME)
		$(CHMOD) $(INSTALLMODE) $(INSTALLNAME)
		$(MV) $(INSTALLNAME) $(INSTALLDIR)

clean:		FORCE
		$(RM) -f $(OBJS) $(INSTALL-NAME) gencmd mcmd.h core

cvs: 		FORCE
		( cd ../contrib ; ./cvsupdate )
		$(MAKE) all

mega:		$(CFILES) $(INCS)
		$(CC) $(LFLAGS) $(INSTALLNAME) main.c -DMEGA=1 $(LIBS) $(LPROF)
		objcopy --remove-section=.note --remove-section=.comment $(INSTALLNAME)

$(INSTALLNAME):	$(OBJS)
		$(CC) $(LFLAGS) $(INSTALLNAME) $(OBJS) $(LIBS) $(LPROF)
		objcopy --remove-section=.note --remove-section=.comment $(INSTALLNAME)

config.h:	defines.h global.h h.h structs.h text.h

cfgfile.o:	cfgfile.c $(INCS)
		$(CC) $(CFLAGS) cfgfile.c $(CPROF)

channel.o:	channel.c $(INCS)
		$(CC) $(CFLAGS) channel.c $(CPROF)

com-ons.o:	com-ons.c $(INCS) usage.h
		$(CC) $(CFLAGS) com-ons.c $(CPROF)

combot.o:	combot.c $(INCS)
		$(CC) $(CFLAGS) combot.c $(CPROF)

commands.o:	commands.c $(INCS)
		$(CC) $(CFLAGS) commands.c $(CPROF)

debug.o:	debug.c $(INCS)
		$(CC) $(CFLAGS) debug.c $(CPROF)

dcc.o:		dcc.c $(INCS)
		$(CC) $(CFLAGS) dcc.c $(CPROF)

function.o:	function.c $(INCS)
		$(CC) $(CFLAGS) function.c $(CPROF)

link.o:		link.c $(INCS)
		$(CC) $(CFLAGS) link.c $(CPROF)

main.o:		main.c $(INCS)
		$(CC) $(CFLAGS) main.c $(CPROF)

parse.o:	parse.c $(INCS)
		$(CC) $(CFLAGS) parse.c $(CPROF)

socket.o:	socket.c $(INCS)
		$(CC) $(CFLAGS) socket.c $(CPROF)

userlist.o:	userlist.c $(INCS)
		$(CC) $(CFLAGS) userlist.c $(CPROF)

vars.o:		vars.c $(INCS)
		$(CC) $(CFLAGS) vars.c $(CPROF)

xmech.o:	xmech.c $(INCS)
		$(CC) $(CFLAGS) xmech.c $(CPROF)

trivia.o:	trivia.c $(INCS)
		$(CC) $(CFLAGS) trivia.c $(CPROF)

FORCE:
