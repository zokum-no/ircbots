#
#   EnergyMech, IRC Bot software
#   Copyright (c) 1997-2001 proton, 2002-2003 emech-dev
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

MISCFILES =	COPYING README TODO VERSIONS Makefile configure checkmech mech.help sample.set

RANDFILES =	randfiles/randaway.e randfiles/randinsult.e randfiles/randkicks.e randfiles/randnicks.e \
		randfiles/randpickup.e randfiles/randsay.e randfiles/randsignoff.e randfiles/randversions.e

SRCFILES =	src/Makefile.in src/cfgfile.c src/channel.c src/com-ons.c src/combot.c src/commands.c \
		src/config.h.in src/dcc.c src/debug.c src/defines.h src/function.c src/global.h src/h.h \
		src/link.c src/main.c src/gencmd.c src/parse.c src/socket.c src/structs.h src/usage.h \
		src/userlist.c src/vars.c src/xmech.c

CONFFILES =	src/Makefile src/config.h

DISTFILES =	$(MISCFILES) $(RANDFILES) $(SRCFILES)

mech:		$(SRCFILES) $(CONFFILES)
		( cd src ; $(MAKE) $@ )

fresh:		FORCE
		rm -f core LinkEvents mech.debug mech.session mech.pid mech.levels coredebug debug.links *.seen 

cvs:		FORCE
		( cd contrib ; ./cvsupdate )
		( cd src ; $(MAKE) all )

dist:		FORCE
		$(MAKE) dist2 DISTDIR="emech-"`grep 'VERSION\[\].*=' src/global.h | cut -d'"' -f2`

dist2:		FORCE
		/bin/rm -rf /tmp/$(DISTDIR)
		/bin/mkdir /tmp/$(DISTDIR)
		tar cf - $(DISTFILES) | ( cd /tmp/$(DISTDIR) ; tar --preserve -xf - )
		cd /tmp ; tar cf - $(DISTDIR) | gzip -9 > $(DISTDIR).tar.gz
		/bin/rm -rf /tmp/$(DISTDIR)

.DEFAULT:	$(SRCFILES) $(CONFFILES)
		( cd src ; $(MAKE) $@ )

FORCE:

