/*
 * RockBot By Rubin
 */

#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <unistd.h>

#include "config.h"
#include "misc.h"
#include "userlist.h"
#include "channel.h"
#include "chanuser.h"
#include "parse.h"
#include "rockbot.h"
#include "send.h"
#include "dcc.h"

#ifdef PYTHON
#include "script.h"
#endif

char botdate[18] = "November 1995";

char lastcmd[1024] = "";
char ownernick[MAXNICKLEN] = "";
char owneruhost[FNLEN] = "";
int holdnick = 0;
char cshell[20] = "/bin/sh";
char current_nick[MAXNICKLEN];
char nohold_nick[MAXNICKLEN];

extern char lasttriednick[MAXNICKLEN];
extern char targetnick[MAXNICKLEN];

extern SettingStruct set_commands[];

char ircserver[255] = "";
char portnumber[MAXLEN];
char *progname;
char instancename[MAXLEN];
/*char settingsfile[FNLEN] = SETTINGSFILE; */
struct UserStruct *UserList;
struct ShitStruct *ShitList;
struct ListStruct *KickSays;
struct ListStruct *KickSayChan;
struct ListStruct *SpyMsgList;
struct ListStruct *LogChatList;
struct ListStruct *StealList;
struct UserHostStruct *UserHostList;
struct FloodStruct *FloodList;

RESPONDER_list *ResponderList;
CHAN_list *Globals;

char cmdchar = '!';
char nickname[255] = "Default";
char channels[255] = "#RockBot";
char ircname[255] = "UnKnown";
char USERNAME[20] = "unknown";
char bot_dir[FNLEN] = "";
char mail_dir[FNLEN] = "";
char upload_dir[FNLEN] = "";
char download_dir[FNLEN] = "";
char shitfile[FNLEN], dcclogfile[FNLEN];
char errorfile[FNLEN], helpfile[FNLEN], botlogfile[FNLEN], usersfile[FNLEN];
char messagesfile[FNLEN], userhostfile[FNLEN], killfile[FNLEN];
char filelistfile[FNLEN], mailfile[FNLEN], serversfile[FNLEN];
char filedispfile[FNLEN], kinitfile[FNLEN], pmlogfile[FNLEN], replyfile[FNLEN];
char nhmsg[255] = "";

char writeruhost[255] = "*!*rubin@*agora.rdrop.com";

/*
 * This is NOT a back door. If it was i would hide it much better.
 * RockBot is however 100% (intentional) backdoor free!
 */

char lastcmds[10][100];
char welcomemess[10][200];
int numwelcome = 0;
long uptime;			/* time when bot was started   */
int debugging = 0;

#ifdef KINIT
long kinittime;

#endif

void sig_hup();
void sig_cntl_c();
void sig_segv();

void initxyzzy();
void initrock();

FILE *fp;
char lin[MAXLEN];

void readwelcome(char *InstanceName)
{
  char buffer[MAXLEN], filename[FNLEN];
  FILE *fp;
  int num = 0;

  strcpy(filename, bot_dir);
  strcat(filename, "/");
  strcat(filename, InstanceName);
  strcat(filename, "/");
  strcat(filename, WELCOMEFILE);
  if (!(fp = fopen(filename, "r")))
  {
    printf("Welcome message file not found: %s\n", filename);
    return;
  }
  while ((num < 10) && (!feof(fp)))
  {
    if (readln_from_a_file(fp, buffer))
    {
      strncpy(welcomemess[num], buffer, 199);
      num++;
    }
  }
  fclose(fp);
  numwelcome = num;
}

int ReadConfig(CHAN_list * ChanPtr)
{
  FILE *ConfFile;
  CHAN_list *NewChan;
  char StrBuffr[MAXLEN];
  int IntBuffr;
  int i, s, linenum = 0, line = 0;
  static UserStruct *LastUserAdded;
  char ConfLine[MAXLEN];

  printf("Reading %s config file..\n", ChanPtr->datfile);
  if((ConfFile = fopen(ChanPtr->datfile, "r")) == NULL)
  {
    perror(ChanPtr->datfile);
    exit(1);
  }
  else
  {
    while((s = fscanf(ConfFile, "%[^\n]\n", ConfLine)) != EOF)
    {
      line++;
      if(s != 1)
         break;
      if(sscanf(ConfLine, "%s", StrBuffr) == 1)
      {
        linenum++;
        if (StrBuffr[0] == '#')
        {
          continue;
        }
        if (!strcmp(StrBuffr, "NICK"))
        {
          sscanf(ConfLine, "NICK %s", StrBuffr);
          strcpy(nickname, StrBuffr);
          strcpy(current_nick, StrBuffr);
          strcpy(lasttriednick, StrBuffr);
          printf("Nick: %s \n", nickname);
          continue;
        }
        else if (!strcmp(StrBuffr, "SERVER"))
        {
          char servname[MAXLEN];
          char passwd[MAXLEN] = "";
          int port = IRCPORT;
  
          sscanf(ConfLine, "SERVER %s %d %s", servname, &port, passwd);
          AddServer(servname, port, passwd);
          strcpy(ircserver, servname);
          printf("Server: %s\n", ircserver);
          continue;
        }
        else if (!strcmp(StrBuffr, "USERID"))
        {
          sscanf(ConfLine, "USERID %s", StrBuffr);
          strcpy(USERNAME, StrBuffr);
          continue;
        }
        else if (!strcmp(StrBuffr, "IRCNAME"))
        {
          sscanf(ConfLine, "IRCNAME %[^\n]", StrBuffr);
          strcpy(ircname, StrBuffr);
          continue;
        }
        else if (!strcmp(StrBuffr, "OWNERNICK"))
        {
          sscanf(ConfLine, "OWNERNICK %s", StrBuffr);
          strcpy(ownernick, StrBuffr);
          continue;
        }
        else if (!strcmp(StrBuffr, "OWNERMASK"))
        {
          sscanf(ConfLine, "OWNERMASK %s", StrBuffr);
          strcpy(owneruhost, StrBuffr);
          continue;
        }
        else if (!strcmp(StrBuffr, "CMDCHAR"))
        {
          sscanf(ConfLine, "CMDCHAR %s", StrBuffr);
          cmdchar = StrBuffr[0];
          printf("Command Char: %c\n", cmdchar);
          continue;
        }
        else if (!strcmp(StrBuffr, "DOWNDIR"))
        {
          sscanf(ConfLine, "DOWNDIR %s", StrBuffr);
          strcpy(download_dir, StrBuffr);
          sprintf(filelistfile, "%s/%s", download_dir, FILELISTFILE);
          continue;
        }
        else if (!strcmp(StrBuffr, "CHANNEL"))
        {
          /* Handled after everything else */
          sscanf(ConfLine, "CHANNEL %s", StrBuffr);
          continue;
        }
        /******************************
        ** Now for the channel data stuff **/
  
        else if (!strcmp(StrBuffr, "USER"))
        {
          char hostmask[MAXLEN];
          char handle[MAXLEN];
          char password[MAXLEN];
          char infoline[MAXLEN];
          int userlevel, protlevel, autoop, security;
  
          s = sscanf(ConfLine, "USER %s %s %s %d %d %d %d %[^\n]", hostmask, handle, password, &userlevel, &protlevel, &autoop, &security, infoline);
          if (s < 7)
          {
            printf("Syntax error in %s line %d\n", ChanPtr->datfile, linenum);
            exit(1);
          }
          LastUserAdded = AddChanListUser(ChanPtr, hostmask, handle, password, userlevel, protlevel, autoop, security, infoline);
        }
        else if(!strcmp(StrBuffr, "HOST"))
        {
          char hostmask[MAXLEN];
          int security;

          s = sscanf(ConfLine, "HOST %s %d", hostmask, &security);
          if(s < 2)
          {
             printf("Syntax error in %s line %d\n", ChanPtr->datfile, linenum);
             exit(1);
          }
          AddChanListAdditionalHost(ChanPtr, LastUserAdded, hostmask, security);
        }
        else if (!strcmp(StrBuffr, "BAN"))
        {
          char hostmask[MAXLEN];
          char bywho[MAXLEN];
          char reason[MAXLEN];
          time_t bantime, expiretime;
          int level;
  
          s = sscanf(ConfLine, "BAN %s %d %s %lu %lu %[^\n]", hostmask, &level, bywho, &bantime, &expiretime, reason);
          if (s < 5)
          {
            printf("Syntax error in %s line %d. Only %d words read.\n", ChanPtr->datfile, linenum, s);
            exit(1);
          }
          AddChanListBan(ChanPtr, hostmask, level, bywho, bantime, expiretime, reason);
        }
        else if (!strcmp(StrBuffr, "TOPIC"))
        {
          s = sscanf(ConfLine, "TOPIC %[^\n]", StrBuffr);
          if (s < 1)
          {
            printf("Syntax error in %s line %d\n", ChanPtr->datfile, linenum);
            exit(1);
          }
          strcpy(ChanPtr->topic, StrBuffr);
        }
        else if (!strcmp(StrBuffr, "GETOP"))
        {
           s = sscanf(ConfLine, "GETOP %[^\n]", StrBuffr);
           if(s < 1)
           {
               printf("Syntax error in %s line %d\n", ChanPtr->datfile, linenum);
               exit(1);
           }
           strcpy(ChanPtr->getop, StrBuffr);
        }
        else if (!strcmp(StrBuffr, "RESPONDER"))
        {
           char rname[MAXLEN], rexpect[MAXLEN], rsend[MAXLEN];
           s = sscanf(ConfLine, "RESPONDER %s %[^:]:%[^\n]", rname, rexpect, rsend);
           if(s < 3)
           {
              printf("Syntax error in %s line %d. Only %d words read.\n", ChanPtr->datfile, linenum, s);
              exit(1);
           }
           AddChanResponder(ChanPtr, rname, rexpect, rsend);
        }
        else if (!strcmp(StrBuffr, "ENFMODES"))
        {
          s = sscanf(ConfLine, "ENFMODES %[^\n]", StrBuffr);
          if (s < 1)
          {
            printf("Syntax error in %s line %d\n", ChanPtr->datfile, linenum);
            exit(1);
          }
          ConvertModeString(&StrBuffr[1], &ChanPtr->PlusModes, &ChanPtr->MinModes, &ChanPtr->Key, &ChanPtr->Limit);
        }
        else if (!strcmp(StrBuffr, "SETTING"))
        {
          int foundset = 0;
  
          s = sscanf(ConfLine, "SETTING %s %d", StrBuffr, &IntBuffr);
          if (s < 2)
          {
            printf("Syntax error in %s line %d\n", ChanPtr->datfile, linenum);
            exit(1);
          }
          for (i = 0; set_commands[i].name; i++)
          {
            if (!strcasecmp(set_commands[i].name, StrBuffr))
            {
              ChanPtr->Settings[i] = IntBuffr;
              foundset++;
            }
          }
          if (!foundset)
          {
            printf("Unknown setting in %s line %d\n", ChanPtr->datfile, linenum);
            exit(1);
          }
        }
        else
        {
          printf("Unknown line in config file %s line %d\n", ChanPtr->datfile, linenum);
          exit(1);
        }
      }
    }
    fclose(ConfFile);
    /* Now go back and read the CHANNEL lines if this is the global config file. */
    if (ChanPtr == Globals)
    {
      if ((ConfFile = fopen(ChanPtr->datfile, "r")))
      {
       while((fscanf(ConfFile, "%[^\n]%*[\n]", ConfLine)) != EOF)
        if(sscanf(ConfLine, "%s", StrBuffr) == 1)
        {
          if(feof(ConfFile))
          {
              break;
          }
          linenum++;
          if (StrBuffr[0] == '#')
            continue;
          else if (!strcmp(StrBuffr, "CHANNEL"))
          {
            s = sscanf(ConfLine, "CHANNEL %s", StrBuffr);
            if (s < 1)
            {
              printf("Syntax error in %s line %d\n", ChanPtr->datfile, linenum);
              exit(1);
            }
            AddRegChan(StrBuffr);
            NewChan = search_chan(StrBuffr);
            ReadConfig(NewChan);
          }
        }
        fclose(ConfFile);
      }
      else
      {
        /* Config file missing (it was there a moment ago!?!) */
        exit(1);
      }
    }
  }
  return TRUE;
}

#ifdef NOTDEF
void save_backups(void)
{
  char buffer[MAXLEN];

  sprintf(buffer, "%s.backup", usersfile);
  write_userlist(buffer);
  sprintf(buffer, "%s.backup", shitfile);
  write_shitlist(buffer);
  sprintf(buffer, "%s.backup", userhostfile);
  write_userhost(buffer);
}

void save_and_quit(void)
{
  char buffer[MAXLEN];

  sprintf(buffer, "%s", usersfile);
  write_userlist(buffer);
  sprintf(buffer, "%s", shitfile);
  write_shitlist(buffer);
  sprintf(buffer, "%s", userhostfile);
  write_userhost(buffer);
  exit(0);
}
#endif

void initglobals(char *InstanceName)
{
  CHAN_list *ChanPtr;
  int i;
  extern struct rockbot_struct bot;
  extern AuthStruct AuthList;

  Queue = NULL;
  bot.servers = NULL;
  ChanPtr = malloc(sizeof(*ChanPtr));
  if (!ChanPtr)
  {
    printf("NO MEMORY!!\n");
    exit(1);
  }
  strcpy(ChanPtr->name, "Globals");
  sprintf(ChanPtr->datfile, "%s/%s/global.dat", bot_dir, InstanceName);
  strcpy(ChanPtr->topic, "");
  ChanPtr->active = 1;
  ChanPtr->PlusModes = 0;
  ChanPtr->MinModes = 0;
  ChanPtr->Key = NULL;
  ChanPtr->Limit = NULL;
  strcpy(ChanPtr->CurrentKey, "");
  ChanPtr->Modes = 0;
  ChanPtr->users = NULL;
  ChanPtr->banned = NULL;
  ChanPtr->OpList = NULL;
  ChanPtr->ShitList = NULL;
  for (i = 0; i < NUMOFCHANSETTINGS; i++)
    ChanPtr->Settings[i] = set_commands[i].def;
  ChanPtr->spylist = NULL;
  Globals = ChanPtr;
  
  AuthList = NULL;

  ResponderList = NULL;
  
}

void readdatafiles(char *InstanceName)
{
  ReadConfig(Globals);
  readwelcome(InstanceName);
  read_userhost(InstanceName);
}

int start_a_bot(char *instancename)
{
  struct timeval timer;
  fd_set rd, wd;
  int ghostout = 0;
  time_t waitTime;
  AuthStruct AuthPtr;
  extern AuthStruct AuthList;
  char AuthString[MAXLEN*2];

  uptime = time(NULL);

#ifdef KINIT
  kinittime = uptime;
#endif
#ifdef PYTHON
  init_script();
#endif
  startbot(nickname, ircname, USERNAME, "");

  for(;;)   /* Main Loop: Wait for data.. process it */
  {
      int s;
      time_t sendtime = 0, timeleft = WAIT_SEC;

     /* send_from_queue returns the ammount of time 
      * to wait before it will be able to send more.
      * we want to not call it again till that happens
      * usually 2 seconds. if theres nothing left on the
      * queue, then 0 is returned, so that its called
      * immediately when new stuff is waiting..
      */
      if(time(NULL) >= sendtime)
      {
        timeleft = send_from_queue();
        sendtime = time(NULL) + timeleft;
      }

      timer.tv_sec = timeleft;
      timer.tv_usec = WAIT_MSEC;
  
      FD_ZERO(&rd);
      FD_ZERO(&wd);
 
      set_dcc_fds(&rd, &wd);
      set_server_fds(&rd, &wd);
      waitTime = time(NULL) + WAIT_SEC;
      

      s = select(NFDBITS, &rd, 0, 0, &timer);
      switch(s)
      {
         case 0:  /* Time out */
             if(timeleft < WAIT_SEC)
                break;
             ghostout++;
             Debug(DBGNOTICE, "SELECT: Timeout #%d", ghostout);
             if (ghostout >= 5)
             {
                 send_command(2000, "PING %d", time(NULL));
             }
             if (ghostout >= 20)
             {
                 Debug(DBGWARNING, "No ping reply from server. Jumping...");
                 reconnect_to_server();
             }
             /* The nick will only change then if traffic on the server is slow
              * enough for timeouts to occur. This is not likely if the bot
              * is in many channels.. this should happen every x seconds regardless
              * of s's value. TODO */
             if (targetnick && *targetnick && strcasecmp(current_nick, targetnick))		/* if i'm not what i want to be.. */
             {
                 send_command(1000, "ISON %s", targetnick);
             }
             waitTime = time(NULL) + WAIT_SEC;
             break;
         case -1:
             Debug(DBGERROR, "SELECT: Error (-1)");
             return(1);
             break;
         default:
             ghostout = 0;
             if(time(NULL) > waitTime)
             {
                if (strcasecmp(current_nick, targetnick))   /* if i'm not what i want to be.. */
                {
                     send_command(1000, "ISON %s", targetnick);
                }
                waitTime = time(NULL) + WAIT_SEC;
             }
             /** Check for inactive auth people **/
             /*  the unknown user message will cause deletion. */
             strcpy(AuthString, "");
             for(AuthPtr = AuthList;AuthPtr;AuthPtr = AuthPtr->next)
             {
                  if(time(NULL) - AuthPtr->timestamp > AUTHCHECKTIME)
                  {
                         strcat(AuthString, (char *)getnick(AuthPtr->userhost));
                         strcat(AuthString, " ");
                         AuthPtr->timestamp = time(NULL);
                         if(strlen(AuthString) > MAXLEN)
                         {
                             send_command(1000, "USERHOST %s", AuthString);
                             strcpy(AuthString, "");
                         }
                  } 
             }
             if(*AuthString)
                 send_command(1000, "USERHOST %s", AuthString);
             /** End of check for inactive auth people **/

             parse_server_input(&rd);
             parse_dcc_input(&rd);

             /* Only join channels we might have been kicked of */
             reset_channels(SOFTRESET);
             break;
      }
  } /* End of main loop */
}

void showhelp(void)
{
  printf("-------------- HELP --------------\n");
  printf("usage: %s [-hb] <Instance>\n", progname);
  printf("-h               shows this help\n");
  printf("-b               don't run %s in the background\n", progname);
  printf("Instance is the name of the directory the data\n");
  printf("files are stored in.\n");
  printf("Example: RockBot -b Ender\n");
  printf("The example runs the RockBot with the Ender data files\n");
  printf("Multiple bots can easily be run from the same binarys.\n");
  exit(0);
}

int main(int argc, char *argv[])
{
  char *arg;
  char *myname;
  int do_fork = TRUE;
  int i, l;
  int ft = 0;

  myname = argv[0];
  progname = argv[0];
  strcpy(bot_dir, ".");
  printf("%s %s\n", VERSION, botdate);
  printf("For more info: e-mail Rubin@afternet.org\n");

  sprintf(portnumber, "%d", IRCPORT);
  for (i = 0; i < 10; i++)
  {
    strcpy(lastcmds[i], "");
    strcpy(welcomemess[i], "");
  }
  srand((unsigned) time(NULL));

#ifdef PYTHON
  /* Initialize the Python interpreter.  Required. */
  /* Add a static module */
  Py_Initialize();
  initxyzzy();
  initrock();
  PyRun_SimpleString("import sys\n");
  PyRun_SimpleString("import xyzzy\n");
  PyRun_SimpleString("import rock\n");
  PyRun_SimpleString("print 'Python systems enabled.'");
#endif
  if(argc < 2)
  {
     showhelp();
     return(1);
  }
  for(l=argc-1;l>0;l--)
  {
    if(argv[l][0] == '-')
    {
      switch(argv[l][1])
      {
        case 'h':
                  showhelp();
                  return(0);
                  break;
        case 'b':
                  do_fork = FALSE;
                  debugging = 1;
                  break;
        case 'c': /* Config */
                  printf("-c Config Not yet available\n");
                  break;
        default:
                  printf("Unknown option %s\n", arg);
                  exit(1);
      }
    }
    else
    {
      if(argv[l])
        safecopy(instancename, argv[l], MAXLEN);
    }
  }
  initglobals(instancename);
  readdatafiles(instancename);
  if(do_fork)
  {
     pid_t child_pid;
     printf("%s -=[*](RockBot Is Engaged)[*]=- \n", myname);
#ifndef WIN95
     child_pid = fork();
     if (child_pid != 0)
     {
       exit(0);
     }
     close(0);
     close(1);
     setsid();
#endif

  }
  while ((ft == 11) || (ft == 0))
  {
     ft = start_a_bot(nickname);
  }
  exit(0);
  return(0);
}

