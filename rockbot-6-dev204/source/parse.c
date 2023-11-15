#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>

#include "parse.h"
#include "commands.h"
#include "channel.h"
#include "chanuser.h"
#include "misc.h"
#include "config.h"
#include "userlist.h"
#include "rockbot.h"
#include "ctcp.h"
#include "send.h"
#include "function.h"
#include "log.h"

extern long uptime, kinittime;
extern ListStruct *StealList;
extern char current_nick[MAXNICKLEN];

char targetnick[MAXNICKLEN] = "";
char lasttriednick[MAXNICKLEN] = "";
int ncount = 0;			/* keeps track of nick change count, from nick in use error handle */

	   /* this is set to 0 in do_nick() so we know were starting over */
extern char errorfile[255];
extern char ownernick[255], shitfile[FNLEN], kinitfile[FNLEN];
extern char owneruhost[255], userhostfile[FNLEN], usersfile[FNLEN];
extern int holdnick;
int holdingnick = 0;
extern char nhmsg[255];
extern char lastcmd[1024];
char frombuf[MAXLEN], tobuf[MAXLEN], restbuf[1024];

/*
 * Is try_nick even needed? lets just send a nick command every
 * so often if current_nick != targetnick.  the server will tell
 * us when our nick changed, or if it didnt.
 *
 *void try_nick()
 *{
 * if (*oldnick)
 * {
 *    send_to_user("rubin", "Trying to use %s", oldnick);
 *    strcpy(current_nick, oldnick);
 *    sendnick(current_nick);
 *    strcpy(oldnick, "");
 * }
 *}
 *
 */

void parse_privmsg(char *from, char *line)
{
  char *text;
  char *to;

  Debug(DBGINFO, "PRIVMSG from %s: %s", from, line);

  if (!from || !*from)
    return;			/* No from stuff */
/*
 * if( ( text = index( line, ' ' ) ) != 0 )
 */
  if((text = strchr(line, ' ')))
  {
    *(text) = '\0';
    text += 2;
  }
  else
    text = "";

  to = line;
  if (*text == ':')
    *(text++) = '\0';
  if (*text == '\x01' && *(text + strlen(text) - 1) == '\x01')
  {
    /* A ctcp message.. */
    *(text++) = '\0';
    *(text + strlen(text) - 1) = '\0';
    strcpy(frombuf, from);
    strcpy(tobuf, to);
    strcpy(restbuf, text);
    if (is_flooding(frombuf))
      return;
    on_ctcp(frombuf, tobuf, restbuf);
/*
    strcpy(restbuf, text);
    on_msg(frombuf, tobuf, restbuf);
*/
  }
  else
  {
    strcpy(frombuf, from);
    strcpy(tobuf, to);
    if (is_flooding(frombuf))
      return;
    strcpy(restbuf, text);
    on_msg(frombuf, tobuf, restbuf);
  }
}


void parse_notice(char *from, char *line)
{
  char *text;
  char *to;

  if (!from || !*from)
    return;
  Debug(DBGINFO, "Notice from %s: %s", from, line);

  if((text = strchr(line, ' ')))
  {
    *(text) = '\0';
    text += 2;
  }
  else
    text = "";

  to = line;
  if (*text == ':')
    *(text++) = '\0';
  strcpy(frombuf, from);
  strcpy(tobuf, to);
  strcpy(restbuf, text);
  if (is_flooding(frombuf))
    return;
  if (!strchr(from, '@'))
  {
    on_serv_notice(frombuf, tobuf, restbuf);
    return;
  }
  else
  {
    /* Not good to respond to notices. violates irc protocol etc */
/*
    // on_msg( frombuf, tobuf, restbuf );
*/
  }
}

/* Mode change */
void parse_324(char *from, char *rest)
{
  Debug(DBGINFO, "Mode from %s: %s", from, rest);
  rest = strchr(rest, ' ');
  on_mode(from, rest);
}

/* 352 = who reply */
void parse_352(char *line)
{
  char *channel;
  char *nick;
  char *user;
  char *host;
  char *server;			/* currently unused */
  char *mode;
  char buffer[WAYTOBIG];
  extern char botserver[2048];

  /* Skip target cookie */
  line = strtok(line, " ");
  /* Is it a header? return.. */
  if (*line == 'C')
    return;
  channel = strtok(NULL, " ");
  user = strtok(NULL, " ");
  host = strtok(NULL, " ");
  server = strtok(NULL, " ");
  nick = strtok(NULL, " ");
  mode = strtok(NULL, " ");
/* Hrm. we grab the host from the welcome message, so
 * i dont really think this is even necessary. we shouldn't
 * be wasting time whoising ourselves anyway.
 */
#ifdef USE_SERVER_HOST
  if (strcasecmp(current_nick, nick) == 0)
  {
     strncpy(botserver, host, 2000);   /* Use this for DCC if needed */
  }
#endif
  sprintf(buffer, "%s!%s@%s", nick, user, host);
  update_userhostlist(buffer, channel, time((time_t *) NULL));
  add_user_to_channel(channel, nick, user, host);
  while (*mode)
  {
    switch (*mode)
    {
    case 'H':
    case 'G':
    case '*':
      break;
    case '@':
      change_usermode(channel, nick, MODE_CHANOP);
      break;
    case 'd':
    case 'k':
    case '+':
      change_usermode(channel, nick, MODE_VOICE);
      break;
    default:
#ifdef DBUG
      printf("*** MODE: unknown mode %c\n", *mode);
#endif
      break;
    }
    mode++;
  }
  if (usermode(channel, current_nick) & CHFL_CHANOP)
    remove_from_list(&StealList, channel);
}

/* Channel ban list */
void parse_367(char *rest)
{
  char *nick;
  char *channel;
  char *banstring;
  char *person;
  char *bantime;

  CHAN_list *ChanPtr;

  nick = strtok(rest, " ");
  channel = strtok(NULL, " ");
  banstring = strtok(NULL, " ");
  person = strtok(NULL, " ");	/* person who did ban...only for
				 * servers with the patch */
  bantime = strtok(NULL, " "); /* Time stamp */
  

  if(!channel)
     return;
  ChanPtr = search_chan(channel);
  if(!ChanPtr)
     return;
  add_ban(&(ChanPtr->banned), banstring);

/*      add_channelmode(channel, MODE_BAN, banstring);
 ** Why not just add the ban here?  **
 */
}

void parseline(char *line)
{
  char *from;
  char *command;
  char *rest;
  char *channel;
  char *nick;
  char *tmp;
  int numeric;

  if(!line)
    return;
  Debug(DBGNOTICE, "<- %s", line);

  if(strchr(line, '\n'))
    *strchr(line, '\n') = '\0';
  if(strchr(line, '\r'))
    *strchr(line, '\r') = '\0';
  idle_kick();		/*  Kick idle users if toggled on */
  strcpy(lastcmd, line);
/* Check for login PING */
  if(line)
    if (strstr(line, "PING") == line)
    {
      line[1] = 'O';
      send_command(2000, "%s", line);
      return;
    }
/* End check for login PING */

  if (*line == ':')
  {
    if((command = strchr(line, ' ')) == NULL)
      return;
    *(command++) = '\0';
    from = line + 1;
  }
  else
  {
    command = line;
    from = "";
  }

  if(!(rest = strchr(command, ' ')))
    return;
  *(rest++) = '\0';
  if (*rest == ':')
    *(rest++) = '\0';
  if((numeric = atoi(command)))
  {
/** Lets do this with switch **/
    switch(numeric)
    {
        case   1: /* welcome */
                  if(!(lasttriednick && *lasttriednick))
                    strcpy(lasttriednick, current_nick);
                  else
                    strcpy(current_nick, lasttriednick);
                  send_command(2000, "Mode %s +is", current_nick);
                  reset_channels(HARDRESET);
                  ncount = 0;               /* if we got in, we must have a good nick */
                  return;
        case   2: /* Your host.. */
        case   3: /* Created... */
        case   4: /* MYINFO */
        case   5: /* Map reply */
        case   6: /* Map More */
        case   7: /* Map End */
        case   8: /* Server Notice Mask */
        case   9: /* StatMem Total */
        case  10: /* StatMem */
                  return;
        case 200: /* Trace Link */
        case 201: /* Trace Connecting.. */
        case 202: /* Trace Handshake */
        case 203: /* Trace Unknown */
        case 204: /* Trace oper */
        case 205: /* Trace User */
        case 206: /* Trace Server */
        case 208: /* TraceNewType */
        case 209: /* Trace Class */
        case 211: /* Stats Link Info */
        case 212: /* Stats commands */
        case 213: /* Stats C line */
        case 214: /* Stats N line */
        case 215: /* Stats I line */
        case 216: /* Stats K line */
        case 217: /* Stats P line */
        case 218: /* Stats Y line */
        case 219: /* end of Stats */
        case 220: /* NULL */
        case 221: /* UMODEIS */
        case 231: /* Service Info */
        case 232: /* End of Service Info */
        case 233: /* Service */
        case 234: /* Serv list */
        case 235: /* Serv list End */
        case 241: /* Stats L line */
        case 242: /* Stats Uptime */
        case 243: /* Stats O line */
        case 244: /* Stats H line */
        case 246: /* Stats T lien */
        case 247: /* Stats G line */
        case 248: /* Stats U line */
        case 250: /* Stats Conn */
        case 251: /* LuserClient */
        case 252: /* Luser Op */
        case 253: /* Luser Unknown */
        case 254: /* Luser Channels */
        case 255: /* Luser Me */
        case 256: /* Admin */
        case 257: /* AdminLOC1 */
        case 258: /* AdminLOC2 */
        case 259: /* Admin Email */
        case 261: /* TraceLog */
        case 262: /* TracePing */
        case 271: /* Silent list */
        case 272: /* End of silent list */
        case 275: /* Stats D lien */
        case 280: /* Glist */
        case 281: /* End of G list */
        case 283: /* QUAR Nick */
                  return;
        case 301: /* away reply */
        case 302: /* UserHost reply */
                  return;
        case 303: /* ISon reply */
                  tmp = strtok(rest, ":");
                  tmp = strtok(NULL, " ");
                  if (!(tmp))
                  {       /* if targetnick is not in the ISON list, switch to it.. */
                     strcpy(lasttriednick, targetnick);
                     sendnick(targetnick);
                  }
                  return;
        case 304: /* RPL_TEXT */
        case 305: /* No longer away */
        case 306: /* Now marked as away */
        case 307: /* userip reply */
        case 311: /* WHOIS USER */
        case 312: /* whois server */
        case 313: /* whois oper */
        case 314: /* whowas user */
        case 315: /* End of WHO */
        case 316: /* NULL */
        case 317: /* whois Idle */
        case 318: /* End of whois */
        case 319: /* whois chans */
        case 320: /* NULL */
        case 321: /* List Start */
        case 322: /* RPL_LIST */
        case 323: /* List end */
                  return;
        case 324: /* Channel Mode Is... */
                  parse_324(from, rest);
                  return;
        case 329: /* Creation Time */
        case 331: /* No Topic */
        case 332: /* RPL_TOPIC */
        case 333: /* Topic who time */
        case 334: /* List Usage */
        case 341: /* RPL_Inviting */
        case 351: /* Version reply */
                  return;
        case 352: /* Who reply */
                  parse_352(rest);
                  return;
        case 353: /* Name Reply */
        case 354: /* Whospcrpl */
        case 361: /* KILL DONE */
        case 362: /* CLOSING */
        case 363: /* CloseEnd */
        case 364: /* LINKS */
        case 365: /* End of links */
        case 366: /* End of Names */
                  return;
        case 367: /* Banlist */
                  parse_367(rest);
                  return;
        case 368: /* End of Banlist */
        case 369: /* End of WhoWas */
        case 371: /* INFO */
        case 372: /* MOTD */
        case 373: /* InfoStart */
        case 374: /* End of Info */
        case 375: /* MOTD Start */
        case 376: /* End of MOTD */
        case 381: /* Your now an ircop */
        case 382: /* Rehashing */
        case 384: /* My Port Is */
        case 385: /* Not Oper anymore */
        case 391: /* Time reply */
                  return;

        case 401: /* No Such Nick */
                  Debug(DBGINFO, "No such nick: %s", rest?rest:"");
                  if(rest)
                  {
                     char *nick;
                    
                     nick = strtok(rest, " ");
                     nick = strtok(NULL, " ");
                     if(!nick)
                     {
                        Debug(DBGERROR, "Error: Weird 401 reply.");
                        return;
                     }
                     DelAuth(nick);
                     clear_queue_to_one(nick); /*remove any pending NOTICES */
                  }
                  else
                    Debug(DBGERROR, "Error: Empty 401 reply.");
                  return;
        case 402: /* No such Server */
                  return;
        case 403: /* No such channel */
                  Debug(DBGINFO, "No such chanel");
                  return;
        case 404: /* Error: cannot send to channel */
                  Debug(DBGINFO, "Cannot send to channel.");
                  return;
        case 405: /* Too many Channels */
                  Debug(DBGINFO, "Too many channels");
                  return;
        case 406: /* Was no such nick */
        case 407: /* TooManyTargets */
        case 408: /* NULL */
        case 409: /* No Origin */
        case 410: /* NULL */
        case 411: /* NO Recipient */
                  return;
        case 412: /* No text to send */
                  Debug(DBGINFO, "No text to send");
                  return;
        case 413: /* No top level domain */
        case 414: /* Wildcard in toplevel domain */
        case 415: /* NULL */
        case 416: /* Query too long */
        case 417: /* NULL */
        case 421: /* Unknown command */
        case 422: /* No MOTD */
        case 423: /* No Admin Info */
        case 424: /* NULL */
                  return;
        case 431: /* No Nick Givin */
                  Debug(DBGINFO, "No nick givin");
                  return;
        case 432: /* Erroneus nick name */
                  Debug(DBGINFO, "Erroneus nick name");
                  return;
        case 433: /* Nick name in use */
                  if (ncount == 0)
                  {
                     if ((strlen(lasttriednick)) < 9)
                        strcat(lasttriednick, "_");
                     else
                        lasttriednick[strlen(lasttriednick) - 1] = '_';
                  }
                  else
                  {
                    if (lasttriednick[strlen(lasttriednick) - 1] == '9')
                      lasttriednick[strlen(lasttriednick) - 1] = '_';
                    else if (lasttriednick[strlen(lasttriednick) - 1] == '_')
                      lasttriednick[strlen(lasttriednick) - 1] = '0';
                    else if (lasttriednick[strlen(lasttriednick) - 1] == '`')
                      lasttriednick[strlen(lasttriednick) - 1] = '0';
                    else
                      lasttriednick[strlen(lasttriednick) - 1]++;
                  }
                  ncount++;
                  sendnick(lasttriednick);
                  return;
        case 434: /* NULL */
        case 435: /* NULL */
        case 436: /* Nick Collision */
        case 437: /* Baned user nick change error */
                  Debug(DBGINFO, "Cannot change nicks, i'm banned!");
                  return;
        case 438: /* Nick change to fast */
                  /* since ircd stupidly counts failed changes in its flood check,
                   * we have to resort to lame delay to be sure nick
                   * is changed when it needs to be like in  on_nick().
                   */
                  sleep(20);
                  sendnick(lasttriednick);
                  return;
        case 439: /* Target changed to fast */
        case 440: /* NULL */
        case 441: /* User Not In Channel */
        case 442: /* You arnt on channel */
        case 443: /* Already in channel */
        case 444: /* NULL */
        case 451: /* Not registered */
        case 452: /* NULL */
        case 461: /* Need more params */
        case 462: /* Already registered */
        case 463: /* No permmision for host */
        case 464: /* Pass mismatch */
        case 465: /* Your banned creep */
        case 466: /* You will be banned */
        case 467: /* Chan key already set */
        case 468: /* Invalid username */
        case 469: /* NULL */
        case 470: /* NULL */
                  return;

        case 472: /* Unknown mode */
                  return;
     /* Channel failures handled together. */
        case 471: /* Chan full */
        case 473: /* Invite only chan */
        case 474: /* Banned from chan */
        case 475: /* Bad key */
        case 476: /* Bad Channel Mask */
                  rest = strchr(rest, '#');
                  channel = strtok(rest, " ");
                  mark_failed(channel);
                  return;
        case 477: /* Chan does not support modes */
        case 478: /* Banlist full */
        case 479: /* NULL */
        case 480: /* NULL */
        case 481: /* No Privlidges (not oper)*/
        case 482: /* Your not channel op */
        case 483: /* Cant kill a server stupid */
        case 484: /* cant kill/kick/deop chan service (+k) */
        case 485: /* NULL */
        case 491: /* NO Olines for your host */
        case 501: /* Unknown Mode flag */
                  return;
        case 502: /* Cant change other users modes */
                  Debug(DBGINFO, "Cannot change other users modes");
        case 511: /* Silent list full */
        case 512: /* No such gline */
        case 513: /* Bad Ping */
        case 514: /* Cant kill ircop (afternet)*/
        
        default:   return;
    }
  }
  if (strcmp(command, "NOTICE") == 0)
  {
    parse_notice(from, rest);
    return;
  }
  else if (strcmp(command, "PRIVMSG") == 0)
  {
    parse_privmsg(from, rest);
    return;
  }
  else if (strcmp(command, "JOIN") == 0)
  {

    {
      char from_copy[MAXLEN];
      char *n, *u, *h;

      strcpy(from_copy, from);
      n = strtok(from_copy, "!");
      u = strtok(NULL, "@");
      h = strtok(NULL, "\n\0");
      if (h && u && n)
        add_user_to_channel(rest, n, u, h);
      else
        return;
    }
    /* if it's me I succesfully joined a channel! */
    if (!strcasecmp(current_nick, getnick(from)))
    {
      /* The modes get sent automaticaly.. no need for MODE #channel */
      send_command(1000, "WHO %s", rest);
      /* send_command(1000, "MODE %s", rest); */
      send_command(900, "MODE %s +b", rest);
      mark_success(rest);
      return; /* Does this need to do on_join for its own?*/
    }
    strcpy(frombuf, from);
    strcpy(restbuf, rest);
    on_join(frombuf, restbuf);
    return;
  }
  else if (strcmp(command, "PART") == 0)
  {
    update_userhostlist(from, rest, time((time_t *) NULL));
    /* again, put this is on_leave? */
    remove_user_from_channel(rest, getnick(from));
    on_part(from, rest);
    steal_channels();
    return;
  }
  else if (strcmp(command, "QUIT") == 0)
  {
    update_userhostlist(from, "", time((time_t *) NULL));
    on_quit(from);
    /* if the user quitting had our target nick, take it.. */
    if (!strcasecmp(getnick(from), targetnick))
      if (strcasecmp(getnick(from), targetnick) == 0)
	sendnick(targetnick);
    /* if were suposed to be holding nick for owner... */
    /* this could be done differently now that the better */
    /* nick system is in place. */
    if (!strcasecmp(getnick(from), ownernick))
    {
      if (!holdingnick && holdnick)
      {
	if (!*nhmsg)
	  send_command(1000, "AWAY :\x2not here\x2");
	else
	  send_command(1000, "AWAY :\x2%s\x2", nhmsg);
	holdingnick = 1;
	strcpy(targetnick, ownernick);
	sendnick(targetnick);
	strcpy(nhmsg, strtok(rest, "\n"));
      }
    }
    remove_user(getnick(from));
    steal_channels();
    return;
  }
  else if (strcmp(command, "NICK") == 0)
  {
    on_nick(from, rest);	/* currently null function.. */
    if (strcasecmp(getnick(from), current_nick) == 0)
    {				/* we saw our nickchange be successfull... */
      rest = strtok(rest, " ");	/* just in case somebodys playing with numerics on a service */
      strcpy(current_nick, rest);	/* set our new nick as our current nick. */
    }
    if (strcasecmp(getnick(from), targetnick) == 0)
    {				/* if the nick we want just got free.. */
      sendnick(targetnick);
    }

    /* This part below takes care of a pending hn command. */
    /* should really be in on_nick.. */
    if (!strcasecmp(getnick(from), ownernick) &&
	strcasecmp(rest, ownernick))
    {
      if (!holdingnick)
      {
	if (holdnick)
	{
	  if (!*nhmsg)
	    send_command(1000, "AWAY :\x2not here\x2");
	  else
	    send_command(1000, "AWAY :\x2%s\x2", nhmsg);
	  holdingnick = 1;
	  strcpy(targetnick, ownernick);
	  sendnick(targetnick);
	}
      }
      else
      {
	send_command(1000, "AWAY :");
	strcpy(nhmsg, "");
	holdingnick = 0;
      }
    }
    change_nick(getnick(from), rest);
    return;
  }
  else if (strcmp(command, "MODE") == 0)
  {
    strcpy(frombuf, from);
    strcpy(restbuf, rest);
    on_mode(frombuf, restbuf);
    return;
  }
  else if (strcmp(command, "KICK") == 0)
  {
    char *reason;

    channel = strtok(rest, " ");
    nick = strtok(NULL, " ");
    reason = strtok(NULL, "\n\0");

    if (reason && (*reason = ':'))
      reason++;
    strcpy(frombuf, from);
    strcpy(tobuf, channel);
    strcpy(restbuf, nick);
    on_kick(frombuf, tobuf, restbuf, reason);
    if (!strcasecmp(current_nick, nick))
      mark_failed(channel);
    else
      remove_user_from_channel(channel, nick);
    return;
  }
  else if (strcmp(command, "PING") == 0)
  {
    char *host;

    host = strtok(rest, " ");
    if(host)
      sendping(host);
    remove_oldfloods();
#ifdef KINIT
    if (getthetime() - kinittime > 21600)
    {
      make_process(kinitfile, "");	/* reissue ticklets   */
      kinittime = getthetime();	/* every 10 hrs */
    }
#endif
#ifdef NOTDEF
    write_userlist(usersfile);
    write_shitlist(shitfile);	/* Backups */
    write_userhost(userhostfile);
#endif
    return;
  }
  else if (strcmp(command, "ERROR") == 0)
  {
    botlog(errorfile, "SIGNING OFF: %s", rest);
    botlog(errorfile, "           - trying to reconnect");
    reconnect_to_server();
  }
}

void parse_server_input(fd_set *read_fds)
/*
 * If read_fds is ready, this routine will read a line from the 
 * the server and parse it.
 */
{
  char line[INBUFFSIZE+1];

  if (serversock_set(read_fds))
  {
    if (readln(line) > 0)
      parseline(line);
    else
    {
       Debug(DBGERROR, "Server read failed, reconnecting..");
       reconnect_to_server();
    }
  }
}
