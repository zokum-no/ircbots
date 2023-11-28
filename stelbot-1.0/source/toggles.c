#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#ifndef MSDOS
	#include <strings.h>
#endif
#include <time.h>
#include <signal.h>
#include "config.h"
#include "channel.h"
#include "userlist.h"
#include "dcc.h"
#include "ftext.h"
#include "misc.h"
#include "commands.h"

extern char lastcmd[1024];
extern char current_nick[MAXLEN];
extern long uptime;
extern UserStruct *UserList, *ShitList;
extern ListStruct *KickSays, *KickSayChan;
extern ListStruct *SpyMsgList;
extern ListStruct *ValidAccountList;
extern UserHostStruct *UserHostList;
extern char cmdchar;
extern int klogtoggle, mutoggle, seentoggle, beeptoggle;
extern int pubtoggle, idlelimit, idletoggle, sdtoggle, prottoggle;
extern int aoptoggle, shittoggle, enfmtoggle, floodtoggle, masstoggle;
extern int weltoggle, soptoggle, cctoggle, logtoggle;
extern int masskicklimit, massdeoplimit, massbanlimit, floodlimit;
extern int minoplevel, minbanlevel, mindeoplevel, minunbanlevel;
extern char *progname, ircserver[255], helpfile[255], filelistfile[255];
extern char shitfile[255], usersfile[255], botlogfile[255], bot_dir[255];
extern char mailfile[255];
extern int holdingnick, holdnick, pmlogtoggle;
extern char nhmsg[255], ownernick[255], owneruhost[255], botdate[255];

void do_togseen( char *from, char *to, char *rest)
{
  if (seentoggle)
    {
      send_to_user( from, "\x2Public seen now disabled\x2");
      seentoggle = 0;
    }
  else
    {
      send_to_user( from, "\x2Public seen now enabled\x2");
      seentoggle = 1;
    }
}

void do_togaop( char *from, char *to, char *rest )
{
	if (aoptoggle)
	{
		send_to_user( from, "%cAuto-opping now disabled\x2",'\x2' );
		aoptoggle = 0;
	}
	else
	{
		send_to_user( from, "%cAuto-opping now enabled\x2", '\x2' );
		aoptoggle = 1;
	}
}

void do_togflood( char *from, char *to, char *rest )
{
	if (floodtoggle)
	{
		send_to_user( from, "%cFlood protection disabled\x2",'\x2' );
		floodtoggle = 0;
	}
	else
	{
		send_to_user( from, "%cFlood protection enabled\x2",'\x2' );
		floodtoggle = 1;
	}
}

void do_togenfm( char *from, char *to, char *rest )
{
	if (enfmtoggle)
	{
		send_to_user( from, "\x2Mode enforcement now disabled\x2" );
		enfmtoggle = 0;
	}
	else
	{
		send_to_user( from, "\x2Mode enforcement now enabled\x2" );
		enfmtoggle = 1;
	}
}

void do_togshit( char *from, char *to, char *rest )
{
	if (shittoggle)
	{
		send_to_user( from, "\x2Shitkicking now disabled\x2" );
		shittoggle = 0;
	}
	else
	{
		send_to_user( from, "\x2Shitkicking now enabled\x2" );
		shittoggle = 1;
	}
}

void do_togprot(char *from, char *to, char *rest)
{
  if (prottoggle)
	 {
		send_to_user(from, "\x2Protection now disabled\x2");
		prottoggle = 0;
	 }
  else
	 {
		send_to_user(from, "\x2Protection now enabled\x2");
		prottoggle = 1;
	 }
}

void do_togsd(char *from, char *to, char *rest)
{
  if (sdtoggle)
	 {
		send_to_user(from, "\x2Server-deop now disabled\x2");
		sdtoggle = 0;
	 }
  else
	 {
		send_to_user(from, "\x2Server-deop now enabled\x2");
		sdtoggle = 1;
	 }
}

void do_togik(char *from, char *to, char *rest)
{
  if (idletoggle)
	 {
		send_to_user(from, "\x2Idle-Kick now disabled\x2");
		idletoggle = 0;
	 }
  else
	 {
		send_to_user(from, "\x2Idle-Kick now enabled\x2");
		idletoggle = 1;
	 }
}

void do_togpub(char *from, char *to, char *rest)
{
  if (pubtoggle)
	 {
		send_to_user(from, "\x2Public commands now disabled\x2");
		pubtoggle = 0;
	 }
  else
	 {
		send_to_user(from, "\x2Public commands now enabled\x2");
		pubtoggle = 1;
	 }
}

void do_togmass(char *from, char *to, char *rest)
{
	if (masstoggle)
	{
		send_to_user( from, "Mass mode protection disabled" );
		masstoggle = 0;
	}
	else
	{
		send_to_user( from, "Mass mode protection enabled" );
		masstoggle = 1;
	 }
}

void do_togwm( char *from, char *to, char *rest )
{
	if (weltoggle)
	{
		send_to_user( from, "\x2Welcome messages disabled\x2");
		weltoggle = 0;
	}
	else
	{
		send_to_user( from, "\x2Welcome messages enabled\x2");
		weltoggle = 1;
	}
}

void do_togso( char *from, char *to, char *rest )
{
	if (soptoggle)
	{
		send_to_user( from, "\x2Strict opping disabled\x2");
		soptoggle = 0;
	}
	else
	{
		send_to_user( from, "\x2Strict opping enabled\x2");
		soptoggle = 1;
	}
}

void do_togcc( char *from, char *to, char *rest )
{
	if (cctoggle)
	{
		send_to_user( from, "%cCommand character now not necessary\x2", '\x2');
		cctoggle = 0;
	}
	else
	{
		send_to_user( from, "%cCommand character now necessary\x2", '\x2');
		cctoggle = 1;
	}
}

void do_toglog( char *from, char *to, char *rest )
{
  if (logtoggle)
    {
      send_to_user( from, "\x2Logging disabled\x2");
      logtoggle = 0;
    }
  else
    {
      send_to_user( from, "\x2Logging enabled\x2");
      logtoggle = 1;
    }
}

void do_toglogk( char *from, char *to, char *rest )
{
  if (klogtoggle)
    {
      send_to_user( from, "\x2Kill logging disabled\x2");
      klogtoggle = 0;
    }
  else
    {
      send_to_user( from, "\x2Kill logging enabled\x2");
      klogtoggle = 1;
    }
}

void do_togmuk( char *from, char *to, char *rest )
{
  if (mutoggle)
    {
      send_to_user( from, "\x2Max-user kicking disabled\x2");
      mutoggle = 0;
    }
  else
    {
      send_to_user( from, "\x2Max-user kicking enabled\x2");
      mutoggle = 1;
    }
}

void do_toglogpm( char *from, char *to, char *rest)
{
  if (pmlogtoggle)
    {
      send_to_user( from, "\x2Private messages no longer being logged\x2");
      pmlogtoggle = 0;
    }
  else
    {
      send_to_user( from, "\x2Private messages now being logged\x2");
      pmlogtoggle = 1;
    }
}

void do_togbk(char *from, char *to, char *rest)
{
  if (beeptoggle)
    {
      send_to_user(from, "\x2Multiple beep kicking disabled\x2");
      beeptoggle = 0;
    }
  else
    {
      send_to_user(from, "\x2Multiple beep kicking enabled\x2");
      beeptoggle = 1;
    }
}

