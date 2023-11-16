/*

    ExtraMech, EnergyMech IRC bot software Extension
    Parts Copyright (c) 2001-2003 Obi-Wan

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/


void do_motd(char *, char *, char *, int);
void do_cstats(char *, char *, char *, int);
void do_phrase(char *, char *, char *, int);
void do_credits(char *, char *, char *, int);
void do_define(char *, char *, char *, int);
void do_defineadd(char *, char *, char *, int);
void do_definedel(char *, char *, char *, int);

/*
 * extramech.c
 */
 
#define TEXT_STARTMOTD		"[\037Mensaje del día - %s\037]"
#define TEXT_ENDMOTD		"[\037Fin del MOTD\037]"
#define TEXT_NOMOTD		"No se encuentra el MOTD"
#define TEXT_STARTCREDITS	"[\037 Creditos \037]"
#define TEXT_ENDCREDITS		"[\037Fin de los creditos\037]"
#define TEXT_WEBSTATS1		"Web de estadísticas: http://stats.irc-world.org/%s.html"
#define TEXT_WEBSTATS2		"Se actualizan aproximandamente cada 1 hora."
#define TEXT_NOWEBSTATS		"Actualmente %s no dispone de estadísticas." 
#define TEXT_OBJECTNICK		"El objeto debe ser un nick"
#define TEXT_NOSUCHDEFINEFILE	"No hay ayuda disponible para %s"

#define TEXT_CANTSAVEDEFINE	"Se ha producido un error guardando la definición %s"
#define TEXT_DEFINEALREADYEXIST	"La definición %s ya existe"
#define TEXT_DEFINESAVED	"La definición %s ha sido guardada"

#define TEXT_DEFINEADDEDBY	"(añadido por \002%s\002)\n"

#define TEXT_DEFINEREMOVED	"La definición %s ha sido eliminada"
#define TEXT_CANTREMOVEDEFINE	"Se ha producido un error al eliminar la definición %s"
#define TEXT_DEFINENOTEXIST	"La definición %s no existe"

