/* 
 *  Rockbot's ircop commands
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>

#include "config.h"
#include "oper.h"
#include "send.h"

int ISOPERED = 0;

void send_oper(char *nick, char *pass)
{
  send_command(200, "OPER %s %s", nick, pass);
}

void send_kill(char *nick, char *reason)
{
  send_command(200,"KILL %s :\x2%s\x2", nick, reason);
}

void send_squit(char *server, char *uplink)
{
  if (!uplink)
    send_command(200, "SQUIT %s", server);
  else
    send_command(200, "SQUIT %s %s", server, uplink);
}

void send_connect(char *server, char *port, char *uplink)
{
  if (!port)
    send_command(200, "CONNECT %s", server);
  if (port && uplink)
    send_command(200, "CONNECT %s %s %s", server, port, uplink);
}

void c_oper(char *from, char *to, char *rest)
{
  char *o_nick;
  char *o_pass;

  o_nick = strtok(rest, " ");
  o_pass = strtok(NULL, " ");
  if (!o_pass || !o_nick)
  {
    send_to_user(from, "\x2With what credental's shall I oper, Sire?\x2");
    send_to_user(from, "\x2Usage: oper <nick> <password>\x2");
    return;
  }

  send_oper(o_nick, o_pass);
  send_to_user(from, "\x2Oper sent.\x2");
  ISOPERED = 1;
}

void c_kill(char *from, char *to, char *rest)
{
  char *reason;
  char *nick;

  if (ISOPERED == 1)
  {
    if (!(nick = strtok(rest, " ")))
    {
      send_to_user(from, "\x2Who shall i kill, Sire?\x2");
      send_to_user(from, "\x2Usage: kill <nick> [reason]\x2");
      return;
    }
    if (!(reason = strtok(NULL, "\n\0")))
      send_kill(nick, "RockBot Kill");
    else
      send_kill(nick, reason);
  }
  else
  {
    send_to_user(from, "\x2Need to Oper up first, Sir.\x2");
    return;
  }

}
