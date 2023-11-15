/*
 * ctcp.c - deals with most of the ctcp stuff (except for DCC).
 *
 *
 */

#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include "misc.h"
#include "ctcp.h"
#include "config.h"
#include "send.h"
#include "dcc.h"

extern char current_nick[MAXLEN];

struct
{
  char *name;
  void (*function) ();
}
ctcp_commands[] =
{
  {
    "FINGER", ctcp_finger
  }
  ,
  {
    "VERSION", ctcp_version
  }
  ,
  {
    "CLIENTINFO", ctcp_clientinfo
  }
  ,
  {
    "ACTION", ctcp_action
  }
  ,
  {
    "PING", ctcp_ping
  }
  ,
  {
    NULL, null(void (*))
  }
};

void on_ctcp(char *from, char *to, char *ctcp_msg)
{
  char *ctcp_command;
  char *rest;
  int i;

  Debug(DBGINFO, "Ctcp from %s to %s: %s", from, to, ctcp_msg);
  rest = strchr(ctcp_msg, ' ') + 1;
  if((rest = strchr(ctcp_msg, ' ')))
  {
    rest++;
  }
  if ((ctcp_command = strtok(ctcp_msg, " ")) == NULL)
    return;
  for (i = 0; ctcp_commands[i].name != NULL; i++)
    if (!strcasecmp(ctcp_commands[i].name, ctcp_command))
    {
      ctcp_commands[i].function(from, to, rest);
      return;
    }
  ctcp_unknown(from, to, ctcp_command);
}

void ctcp_finger(char *from, char *to, char *rest)
{
  char *nick;

  nick = getnick(from);

  send_ctcp_reply(nick, "FINGER \x2Shove your finger up your ass \x2");
  return;
}

void ctcp_version(char *from, char *to, char *rest)
{
  char *nick;

  nick = getnick(from);
  send_ctcp_reply(nick, "VERSION %s", VERSION);
  return;
}

void ctcp_clientinfo(char *from, char *to, char *rest)
{
  char *nick;

  nick = getnick(from);
/*
 * send_ctcp_reply( nick, "CLIENTINFO \x2Who says I'm a client?\x2" );
 */
  send_ctcp_reply(nick, "CLIENTINFO ACCEPT ACTION CLIENTINFO DCC ECHO FINGER PING RESUME SOUND TIME USERINFO VERSION");
  return;
}

void ctcp_dcc(char *from, char *to, char *rest)
{
  reply_dcc(from, to, rest);
}

void ctcp_ping(char *from, char *to, char *rest)
{
  char *nick;

  nick = getnick(from);

  if (rest)
  {
    send_ctcp_reply(nick, "PING %s", rest);
  }
}

void ctcp_action(char *from, char *to, char *rest)
{
  return;
}

void ctcp_unknown(char *from, char *to, char *ctcp_command)
{
  char *nick;

  nick = getnick(from);

/*      send_ctcp_reply( nick, "ERROR Unknown ctcp-command"); */
}
