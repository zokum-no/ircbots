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

/* TEST */
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdarg.h>
#include <sys/time.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#include "config.h"
#include "rockbot.h"
#include "channel.h"
#include "userlist.h"
#include "dcc.h"
#include "ftext.h"
#include "misc.h"
#include "commands.h"
#include "oper.h"
#include "function.h"
#include "send.h"
#include "log.h"
#include "userlist.h"
#include "conv.h"
#include "match.h"
#include "infobot.h"
#include "server.h"

#define getrandom( min, max ) ((rand() % (int)(((max)+1) - (min))) + (min))

extern RockBotStruct bot;
extern char targetnick[MAXNICKLEN];
extern char lasttriednick[MAXNICKLEN];
extern int ncount;
extern char lastcmd[1024];
extern char current_nick[MAXLEN];
extern char nohold_nick[MAXLEN];
extern char globsign;
extern long uptime;
extern UserStruct *UserList, *ShitList;
extern ListStruct *KickSays, *KickSayChan, *LogChatList;
extern ListStruct *SpyMsgList;
extern ListStruct *StealList;
extern UserHostStruct *UserHostList;
extern char cmdchar;
extern char *progname, ircserver[FNLEN], helpfile[FNLEN], killfile[FNLEN];
extern char filelistfile[FNLEN], bot_dir[FNLEN], userhostfile[FNLEN];
extern char filedispfile[FNLEN], settingsfile[FNLEN], pmlogfile[FNLEN];
extern char shitfile[FNLEN], usersfile[FNLEN], botlogfile[FNLEN];
extern char mailfile[FNLEN], serversfile[FNLEN], kinitfile[FNLEN];
extern int holdingnick, holdnick;
extern char download_dir[FNLEN];
extern char nhmsg[255], ownernick[MAXNICKLEN], owneruhost[255], botdate[15];
extern char lastcmds[10][100];
extern char welcomemess[10][200];
extern int numwelcome;
char lockpass[9];
int locked;
char PYNICK[MAXNICKLEN];
char dcctmp[MAXLEN];
int pubseen = 0;

/* Since theres no main.h.. this goes here. */
int ReadConfig(CHAN_list * ChanPtr);

int ACTIVE_USERLEVEL; /* This is used by the priority queue
                       * to make sure that higher users 
                       * get first priority on the bot. 
                       */


struct
{
  char *name;
  void (*function) ();
  int userlevel;		/* Min userlevel required to do command */
  int shitlevel;		/* Max shitlevel allowed to do command */
  int forcedcc;			/* Should the bot ask for a dcc chat before doing this command? */
  int cmdchar;			/* Is the command charactor required? */
  int needchan;			/* Does this command require a channel paramiter */
}
on_msg_commands[] =

/*
 * Any command which produces more than 2 lines output can be 
 * used to make the bot lag.  It wont excess flood but can
 * get pretty far behind.  Set 'force dcc' to true to solve.
 */

#define BotOwner      81
/*        BotOwner      81-100   Owns the bot.  This person can
 * make it send raw stuff to the server
 * execute arbitrary script, delete files
 * anything.. 
 */
#define BotCoOwner    61
/*        BotCoOwner    61-80    This person can make it join/part channels
 * send msgs .. do almost anything pre-programmed.
 */
#define ChanOwner     41
/*        ChanOwner     41-60    This person owns a channel, and 60 is the
 * highest command available on a chan userlist.
 * He can add permbans, add other chan users,
 * make the bot say thing sin his channel ownly, etc.
 */
#define ChanCoOwner   31
/*
 * ChanCoOwner   31-40    This person cant make the bot talk in the channel,
 * or anything too raw, but can add bans/ops etc.
 */
#define ChanHelper    21
/*        ChanHelper    21-30    These people can add bans, set modes, add ops, etc
 */
#define ChanOp        11
/*        ChanOp        11-20    These people get auto-opped and can make the bot
 * do kicks n stuff but thats about it.
 */
#define Peon            1
/*        Peon          01-10    These people cant do anything or get ops, except
 * this access clears them to get info from the bot,
 * etc.  By default anyone who msgs the bot whos
 * not known, might be auto-added as level 1.
 */
#define All           0
/* *INDENT-OFF* */
/*         Command     function      userlvl  shitlvl forcedcc  cmdch  NeedChan*/ 
{
        { "AUTH",     c_auth,        All,          0,   FALSE,   TRUE, T},
        { "LOGIN",    c_auth,        All,          0,   FALSE,   TRUE, T},
        { "AUTHLIST", c_authlist,    BotOwner,     0,    TRUE,   TRUE, F},
        { "HELLO",    c_noop,        All,          0,   FALSE,   TRUE, F},
        { "LEVELS",   c_levels,      All,          0,    TRUE,   TRUE, F},
	{ "ACCESS",   c_access,      All,        100,   FALSE,   TRUE, F},
        { "SET",      c_set,         ChanCoOwner,  0,   FALSE,   TRUE, T},
        { "ADDRESP",  c_addresp,     ChanCoOwner,  0,   FALSE,   TRUE, T},
        { "LISTRESP", c_listresp,    ChanCoOwner,  0,   FALSE,   TRUE, T},
	{ "HELP",     c_help,        All,        100,    TRUE,   TRUE, F},
        { "8BALL",    c_8ball,       Peon,       100,   FALSE,   TRUE, F},
        { "CALC",     c_calc,        Peon,       100,   FALSE,   TRUE, F},
	{ "D",        c_roll,        Peon,       100,   FALSE,   TRUE, F},
#ifdef PYTHON
	{ "PY",       c_python,      BotOwner,     0,   FALSE,   TRUE, F},
        { "PYSCRIPT", c_pyscript,    BotOwner,     0,   FALSE,   TRUE, F},
#endif
        { "STATS",    c_stats,       Peon,       100,   FALSE,   TRUE, T},
	{ "SEEN",     c_seen,        Peon,       100,    TRUE,   TRUE, F},
	{ "INFO",     c_info,        Peon,       100,   FALSE,   TRUE, F},
        { "CINFO",    c_cinfo,       Peon,       100,   FALSE,   TRUE, F},
        { "UPTIME",   c_info,        Peon,       100,   FALSE,   TRUE, F},
	{ "TIME",     c_time,        Peon,       100,   FALSE,   TRUE, F},
	{ "PING",     c_ping,        Peon,       100,   FALSE,   TRUE, F},
        { "CHAT",     c_chat,        Peon,       100,   FALSE,   TRUE, F},
        { "LINK",     c_chat,        Peon,       100,   FALSE,   TRUE, F},
	{ "DOWN",     c_deopme,      All,        100,   FALSE,   TRUE, F},
	{ "USERHOST", c_getuserhost, Peon,       100,    TRUE,   TRUE, F},
	{ "TOTSTATS", c_totstats,    Peon,       100,    TRUE,   TRUE, F},
	{ "TOPTEN",   c_top10kills,  Peon,       100,    TRUE,   TRUE, F},
/*
	{ "DISPLAY",  c_display,     All,        100,    TRUE,   TRUE, F},
*/
	{ "UP",       c_opme,        ChanOp,       0,   FALSE,   TRUE, T},
        { "CHANNELS", c_channels,    Peon,         0,    TRUE,   TRUE, F},
	{ "CHANUSERS",c_chanusers,   ChanHelper,   0,    TRUE,   TRUE, T},
/*         Command     function      userlvl  shitlvl forcedcc  cmdch  NeedChan*/ 
	{ "LKS",      c_listks,      ChanHelper,   0,    TRUE,   TRUE, F},
	{ "LKSC",     c_listksc,     BotCoOwner,   0,    TRUE,   TRUE, F},
	{ "GETCH",    c_getch,       BotCoOwner, 100,   FALSE,   TRUE, F},
	{ "B",        c_ban,         ChanHelper,   0,   FALSE,   TRUE, T},
        { "UB",       c_unban,       ChanHelper,   0,   FALSE,   TRUE, T},
	{ "UNBAN",    c_unban,       ChanHelper,   0,   FALSE,   TRUE, T},
	{ "OP",       c_op,          ChanHelper,   0,   FALSE,   TRUE, T},
	{ "DEOP",     c_deop,        ChanHelper,   0,   FALSE,   TRUE, T},
	{ "K",        c_kick,        ChanHelper,   0,   FALSE,   TRUE, T},
        { "KICK",     c_kick,        ChanHelper,   0,   FALSE,   TRUE, T},
	{ "KB",       c_kickban,     ChanHelper,   0,   FALSE,   TRUE, T},
	{ "INVITE",   c_invite,      ChanHelper,   0,   FALSE,   TRUE, T},
        { "UBM",      c_unbanme,     ChanOp,       0,   FALSE,   TRUE, T},
	{ "TOPIC",    c_topic,       ChanHelper,   0,   FALSE,   TRUE, T},
        { "SCREW",    c_screwban,    ChanHelper,   0,   FALSE,   TRUE, T},
	{ "FUCK",     c_fuckban,     ChanHelper,   0,   FALSE,   TRUE, T},
        { "SB",       c_siteban,     ChanHelper,   0,   FALSE,   TRUE, T},
	{ "SKB",      c_sitekb,      ChanHelper,   0,   FALSE,   TRUE, T},
	{ "SPYMSG",   c_spymsg,      BotCoOwner,   0,   FALSE,   TRUE, F},
	{ "REPEAT",   c_repeat,      BotCoOwner, 100,    TRUE,   TRUE, F},
	{ "RSPYMSG",  c_nospymsg,    All,        100,   FALSE,   TRUE, F},
	{ "SPY",      c_spy,         ChanCoOwner,  0,   FALSE,   TRUE, T},
        { "RSPY",     c_rspy,        All,        100,   FALSE,   TRUE, T},
        { "WHO",      c_who,         ChanCoOwner,  0,   FALSE,   TRUE, T},
        { "TEST",     c_test,        BotOwner,     0,   FALSE,   TRUE, F},
        { "TEST1",    c_test1,       BotOwner,     0,   FALSE,   TRUE, F},
	{ "AOP",      c_aop,         ChanHelper, 100,   FALSE,   TRUE, T},
	{ "RAOP",     c_naop,        ChanHelper, 100,   FALSE,   TRUE, T},
	{ "USERLIST", c_userlist,    ChanHelper, 100,    TRUE,   TRUE, T},
	{ "SHITLIST", c_shitlist,    ChanHelper, 100,    TRUE,   TRUE, T},
	{ "DCCLIST",  c_listdcc,     BotCoOwner, 100,    TRUE,   TRUE, F},
        { "SAY",      c_say,         ChanOwner,    0,   FALSE,   TRUE, T},
	{ "ME",       c_me,          ChanOwner,    0,   FALSE,   TRUE, T},
	{ "CKS",      c_clrks,       ChanHelper,   0,   FALSE,   TRUE, F},
	{ "CLVL",     c_clvl,        ChanHelper,   0,   FALSE,   TRUE, F},
/*         Command     function      userlvl  shitlvl forcedcc  cmdch  NeedChan*/ 
        { "SETSECURITY", c_setsecurity, Peon,      0,   FALSE,   TRUE, T},
	{ "CKSC",     c_clrksc,      BotCoOwner,   0,   FALSE,   TRUE, F},
	{ "AKSC",     c_addksc,      BotCoOwner,   0,   FALSE,   TRUE, F},
	{ "RKSC",     c_delksc,      BotCoOwner,   0,   FALSE,   TRUE, F},
	{ "KS",       c_addks,       ChanHelper,   0,   FALSE,   TRUE, F},
	{ "RKS",      c_delks,       ChanHelper,   0,   FALSE,   TRUE, F},
	{ "MSG",      c_msg,         BotCoOwner,   0,   FALSE,   TRUE, F},
        { "CLEARQ",   c_clearq,      BotCoOwner,   0,   FALSE,   TRUE, F},
	{ "OPEN",     c_open,        ChanHelper,   0,   FALSE,   TRUE, T},
	{ "JOIN",     c_join,        BotCoOwner,   0,   FALSE,   TRUE, F},
	{ "CYCLE",    c_cycle,       ChanCoOwner,  0,   FALSE,   TRUE, T},
	{ "LEAVE",    c_leave,       BotCoOwner,   0,   FALSE,   TRUE, T},
        { "PART",     c_leave,       BotCoOwner,   0,   FALSE,   TRUE, F},
        { "REMCHAN",  c_remchan,     BotOwner,     0,   FALSE,   TRUE, T},
        { "UNREG",    c_remchan,     BotOwner,     0,   FALSE,   TRUE, T},
        { "CHANSTAT", c_chanstat,    All,          0,    TRUE,   TRUE, T},
	{ "NICK",     c_nick,        BotCoOwner,   0,   FALSE,   TRUE, F},
        { "CNU",      c_add,         ChanHelper,   0,   FALSE,   TRUE, T},
        { "ADDUSER",  c_add,         ChanHelper,   0,   FALSE,   TRUE, T},
        { "CNH",      c_addhost,     All,          0,   FALSE,   TRUE, T},
        { "ADDHOST",  c_addhost,     All,          0,   FALSE,   TRUE, T},
        { "DELHOST",  c_delhost,     Peon,         0,   FALSE,   TRUE, T},
        { "SETHANDLE",c_sethandle,   Peon,         0,   FALSE,   TRUE, T},
        { "SETINFO",  c_setinfo,     Peon,         0,   FALSE,   TRUE, T},
        { "PASSWD",   c_passwd,      Peon,         0,   FALSE,   TRUE, T},
        { "PASSWORD", c_passwd,      Peon,         0,   FALSE,   TRUE, T},
        { "PASS",     c_passwd,      Peon,         0,   FALSE,   TRUE, T},
        { "CNC",      c_newchan,     BotOwner,     0,   FALSE,   TRUE, F},
	{ "REM",      c_userdel,     ChanHelper,   0,   FALSE,   TRUE, T},
	{ "SHIT",     c_shit,        ChanHelper,   0,   FALSE,   TRUE, T},
	{ "RSHIT",    c_rshit,       ChanHelper,   0,   FALSE,   TRUE, T},
	{ "PROT",     c_prot,        ChanHelper,   0,   FALSE,   TRUE, T},
	{ "RPROT",    c_nprot,       ChanHelper,   0,   FALSE,   TRUE, T},
	{ "SPYLIST",  c_spylist,     ChanCoOwner,  0,   FALSE,   TRUE, T},
	{ "STEAL",    c_steal,       BotCoOwner,   0,   FALSE,   TRUE, F},
	{ "RSTEAL",   c_nosteal,     BotCoOwner,   0,   FALSE,   TRUE, F},
	{ "MKNU",     c_masskicknu,  ChanCoOwner,  0,   FALSE,   TRUE, T},
	{ "MOPU",     c_massopu,     ChanCoOwner,  0,   FALSE,   TRUE, T},
	{ "MDEOPNU",  c_massdeopnu,  ChanCoOwner,  0,   FALSE,   TRUE, T},
	{ "MOP",      c_massop,      ChanCoOwner,  0,   FALSE,   TRUE, T},
	{ "MDEOP",    c_massdeop,    ChanCoOwner,  0,   FALSE,   TRUE, T},
        { "MK",       c_masskick,    ChanCoOwner,  0,   FALSE,   TRUE, T},
	{ "MKB",      c_masskickban, ChanCoOwner,  0,   FALSE,   TRUE, T},
	{ "UBA",      c_massunban,   ChanCoOwner,  0,   FALSE,   TRUE, T},
/*         Command     function      userlvl  shitlvl forcedcc  cmdch  NeedChan*/ 
	{ "SEND",     c_send,        All,        100,   FALSE,   TRUE, F},
	{ "FILES",    c_flist,       All,        100,    TRUE,   TRUE, F},
	{ "FLIST",    c_flist,       All,        100,    TRUE,   TRUE, F},
	{ "CMDCHAR",  c_cmdchar,     BotOwner,     0,   FALSE,   TRUE, F},
	{ "SERVER",   c_server,      BotOwner,     0,   FALSE,   TRUE, F},
        { "ADDSERVER",c_addserver,   BotOwner,     0,   FALSE,   TRUE, F},
        { "REMSERVER",c_remserver,   BotOwner,     0,   FALSE,   TRUE, F},
        { "SLIST",    c_listservers, BotCoOwner,   0,    TRUE,   TRUE, F},
        { "SERVERLIST",c_listservers,BotCoOwner,   0,    TRUE,   TRUE, F},
	{ "NEXTSERVER",c_nextserver, BotCoOwner,   0,   FALSE,   TRUE, F},
	{ "RUH",      c_removeuh,    BotCoOwner,   0,   FALSE,   TRUE, F},
	{ "KINIT",    c_kinit,       BotOwner,   100,   FALSE,   TRUE, F},
	{ "HEART",    c_heart,       ChanOp,       0,   FALSE,   TRUE, F},
        { "LAST10",   c_last10,      BotCoOwner,   0,    TRUE,   TRUE, F},
	{ "CHACCESS", c_chaccess,    BotOwner,     0,   FALSE,   TRUE, F},
	{ "LOGCHAT",  c_logchat,     ChanCoOwner,  0,   FALSE,   TRUE, T},
	{ "RLOGCHAT", c_rlogchat,    ChanCoOwner,  0,   FALSE,   TRUE, T},
	{ "HN",       c_holdnick,    BotOwner,   100,   FALSE,   TRUE, F},
	{ "RHN",      c_noholdnick,  BotOwner,   100,   FALSE,   TRUE, F},
	{ "RESETUH",  c_resetuhost,  BotCoOwner, 100,   FALSE,   TRUE, F},
	{ "SAVE",     c_save,        BotOwner,     0,   FALSE,   TRUE, F},
	{ "DO",       c_do,          BotOwner,     0,   FALSE,   TRUE, F},
	{ "OPER",     c_oper,        BotCoOwner,   0,   FALSE,   TRUE, F},
/*
//	{ "SPAWN",    c_spawn,       BotOwner,     0,   FALSE,   TRUE, F},
*/
	{ "SLEEP",    c_die,         BotCoOwner,   0,   FALSE,   TRUE, F},
	{ "DIE",      c_die,         BotCoOwner,   0,   FALSE,   TRUE, F},
	{ "QUIT",     c_die,         BotCoOwner,   0,   FALSE,   TRUE, F},
        { "LOCK",     c_lock,        BotCoOwner,   0,   FALSE,   TRUE, F},
        { "UNLOCK",   c_unlock,      All,          0,   FALSE,   TRUE, F},
#ifdef OLINE
	{ "KILL",     c_kill,        BotOwner,     0,   FALSE,   TRUE, F },
#endif
	/*
	 :
	 :
	 */
	{ NULL,     null(void(*)()), 0,  0, FALSE }
};

SettingStruct set_commands[] =
/*   Command        min max def description set0 set1 set2 set3 set4 set5*/
{ 
    { "LOGGING",    LOGGING, 0,  3,  1, BotCoOwner,  "What the bot logs",
                    "The bot will log: ",
                    {"Nothing.",
                    "Kills only.",
                    "Successful commands",
                    "Every line it recieves", 
                    "", ""}},
    { "AUTOOP",     AUTOOP, 0,  5,  4, ChanCoOwner,  "who the bot auto ops on join",       
                    "I will auto-op: ",
                    {"No one.",
                    "Bot owner only.",
                    "Bot(Co)Owners and Chan(Co)Owners only.",
                    "Bot(Co)Owners, Chan(Co)Owners, ChanHelpers, and Ops.",
                    "Chan(Co)Owners, ChanHelpers, and Ops, but NOT Bot(Co)Owners.",
                    "Everyone"}},
    { "PROTECTION", PROTECTION, 0,  1,  1, ChanCoOwner, "If the bot will protect its protected people",
                    "Protected people will be :",
                    {"Not Protected.",
                    "Protected.",
                    "","","",""}},
    { "SHITPROT",   SHITPROT, 0,  1,  1, ChanCoOwner, "If the bot acts on shitlisted people" ,
                    "Shitlist protection is: ",
                    {"disabled.",
                    "enabled.",
                    "","","",""}},
    { "ENFM",       ENFM, 0,  2,  1,  ChanCoOwner, "If the bot enforces the default modes",
                    "Default modes may be changed by: ",
                    {"(Co)Owners only.",
                    "(Co)Owners and ChanHelpers.",
                    "(Co)Owners, ChanHelpers, and ChanOps.",
                    "","",""}},
    { "IDLEKICK",   IDLEKICK, 0,  5,  0, ChanCoOwner, "If the bot kicks idle users",
                    "Idle users will be kicked: ",
                    {"Never.",
                    "after 10 minutes idle.",
                    "after 30 minutes idle.",
                    "after 1 hour idle.",
                    "after 4 hours idle.",
                    "after 12 hours idle."}},
    { "CLONEKICK",  CLONEKICK, 0,  5,  0, ChanCoOwner, "Kicking of clones (max users with same host)",
                    "I will kick if i detect: ",
                    {"Nothing (disabled).",
                    "3 clones.",
                    "4 clones.",
                    "5 clones.",
                    "6 clones.",
                    "7 clones."}},
    { "FLOODKICK",  FLOODKICK, 0,  3,  0, ChanCoOwner,  "Flood protection",
                    "Flood protection is: ",
                    {"Disabled.",
                    "Low (value to be determined)",
                    "Medium ()",
                    "High",
                     "", ""}},
    { "RIDEPROT",   RIDEPROT, 0,  1,  0,  ChanCoOwner, "Deop users opped by a server",
                    "Net-ride server op protection is: ",
                    {"disabled.","enabled.",
                    "","","",""}},
    { "STRICTOP",   STRICTOP, 0,  3,  0, ChanCoOwner, "Strict ops: Only let ppl on the userlist get ops",
                    "Strict Op Protection is: ",
                    {"Disabled.",
                    "Deop non-users opped by other non-users.",
                    "Deop non-users opped by anyone",
                    "Deop non-users opped by other non-users AND deop the other.",
                    "",""}},
    { "WM",         WM, 0,  1,  0,  ChanCoOwner, "Welcome unknowns who say hi/hellow, etc",
                    "Welcome messages are: ",
                    {"Disabled.","Enabled.",
                    "","","",""}},
    { "PUBSEEN",    PUBSEEN, 0,  1,  0, ChanCoOwner,  "Respond to 'anyone seen blah?' in the channel",
                    "Public seen responce is: ",
                    {"Disabled.","Enabled.",
                    "","","",""}},
    { "TALKING",    TALKING, 0,  1,  0, ChanCoOwner,  "Filter public msgs though conv.c",
                    "Public conversation ability is: ",
                    {"Disabled.","Enabled.",
                    "","","",""}},
    { "MASSPROT",   MASSPROT, 0,  3,  0,  ChanCoOwner, "Mass kick/deop protection",
                    "Mass protection is set: ",
                    {"Off (disabled)",
                    "Low ()",
                    "Medium",
                    "High",
                    "",""}},
    { "KSLEVEL",    KSLEVEL, 0,  5,  1, ChanCoOwner, "Who gets kicked for saying banned words",
                    "The bot will kick for banned words: ",
                    {"Only Peons",
                    "ChanOps and Peons",
                    "ChanHelpers, ops and peons.",
                    "ChanCoOwners, helpers ops and peons.",
                    "ChanOwners, helpers, ops and peons.",
                    "BotCoOwners, ChanOwners, CoOwners, helpers, ops and peons."}},
    { "MPLEVEL",    MPLEVEL, 0,  2,  1, ChanCoOwner, "What happens during mass protection",
                    "Mass deop/kick/banning people are: ",
                    {"deoped.","kicked.","kickbanned.",
                    "","",""}},
    { "PUBTOGGLE",  PUBTOGGLE, 0,  1,  1, ChanCoOwner, "How to handle commands done in public",
                    "Public commands are: ",
                    {"Ignored.","Heard.",
                    "","","",""}},
    { "CCREQ",      CCREQ, 0,  1,  1, ChanCoOwner, "If the bot requires a command charactor",
                    "Public commands: ",
                    {"Don't need a command charactor.",
                    "Do need command charactor.",
                    "","","",""}},
    { "PMLOG",      PMLOG, 0,  1,  0, BotCoOwner, "Should we log private messages",
                    "Private messages are: ",
                    {"Not logged.","Logged.",
                    "","","",""}},
    { NULL,         0, 0,  0,  0, All, NULL,       
                    "",
                    {"","","","","",""}}
};

/* *INDENT-ON* */


void on_join(char *who, char *channel)
{
  UserHostStruct *user;
  char buffer[MAXLEN];
  CHAN_list *ChanPtr;
  UserStruct *OpPtr;

  sprintf(buffer, "\002%s has joined channel %s\002", who, channel);
  send_spy_chan(channel, buffer);
  if(!strcmp(getnick(who), current_nick))
     return; /* Its me */
  ChanPtr = search_chan(channel);
  if(!ChanPtr)
  {
    Debug(DBGWARNING, "on_join [%s] but i cant find the channel in memory!", channel);
    return;
  }
  if(!userlevel(who, ChanPtr) && ChanPtr->Settings[CLONEKICK] && (num_on_channel(channel, who) > (ChanPtr->Settings[CLONEKICK] + 2)))
  {
    char n[100], u[100], h[100], m[100];

    sep_userhost(who, n, u, h, m);
    sendprivmsg(channel, "\002Too many users with [%s@%s], clone kick activated!\002", fixuser(u), h);
    sprintf(m, "*!*%s@*", fixuser(u));
    channel_masskickban(channel, m);
  }
  if(is_in_list(LogChatList, channel))
  {
    char buffer[MAXLEN];

    sprintf(buffer, "%s has joined channel %s", who, channel);
    logchat(channel, buffer);
  }
  update_userhostlist(who, channel, getthetime());
  user = find_userhost(who);

  if((user) && (user->totmsg > user->msgnum))
  {
     send_to_user(who, "\002You have %i new message(s) waiting...\002", user->totmsg - user->msgnum);
     send_to_user(who, "\002Use \"%cgetmsg\" to read them\002", cmdchar);
  }
  if((shitlevel(who, ChanPtr)) && ChanPtr->Settings[SHITPROT])
    shitkick(who, channel);
  if(userlevel(who, ChanPtr) && !shitlevel(who, ChanPtr))
  {
    if(isaop(who, ChanPtr) && (ChanPtr->Settings[AUTOOP]))
    {
      switch(ChanPtr->Settings[AUTOOP])
      {
        case 0: /* Don't op anyone */
                break;
        case 1: /* Op the bot owner only */
                if(isowner(who))
                  giveop(channel, getnick(who));
                break;
        case 2: /* Op bot and chan owner/coowner */
               if(userlevel(who, ChanPtr) >= ChanCoOwner)
                  giveop(channel, getnick(who));
               break;
        case 3: /* Ops and up */
               if(userlevel(who, ChanPtr) >= ChanOp)
                  giveop(channel, getnick(who));
               break;
        case 4: /* Chan ppl but not bot co/owner */
               if(userlevel(who, ChanPtr) >= ChanOp)
                if(userlevel(who, ChanPtr) < BotCoOwner)
                  giveop(channel, getnick(who));
               break;
        case 5: /* all */
               giveop(channel, getnick(who));
               break;
       default: Debug(DBGERROR, "Unknown AUTOOP setting %d!", ChanPtr->Settings[AUTOOP]);
      }
    }
  }
  OpPtr = find_user(who, ChanPtr);
  if(!OpPtr)
    OpPtr = find_user(who, Globals);
  if(OpPtr && OpPtr->info && strcmp(OpPtr->info, "*"))
  {
    int i;
    char haystack[MAXLEN], needle[MAXLEN];
    strcpy(haystack, getnick(who));
    strcpy(needle, OpPtr->handle);
    for(i=0;i<MAXLEN-1;i++)
    {
      toupper(haystack[i]);
      toupper(needle[i]);
    }
    if(strstr(haystack, needle))
      sendprivmsg(ChanPtr->name, "[%s] %s", getnick(who), OpPtr->info);
    else
      sendprivmsg(ChanPtr->name, "[%s] %s (%s)", getnick(who), OpPtr->info, OpPtr->handle);
  }
  return;
}

void on_mode(char *from, char *rest)
{
  int did_change = FALSE;	/* If we have to change anything */
  int netsplit = FALSE;
  char *channel;
  char *mode;

  char *chanchars;		/* i.e. +oobli */
  char *params;
  char sign;			/* +/-, i.e. +o, -b etc */

  char unminmode[MAXLEN];	/* used to undo modechanges */
  char unminparams[MAXLEN];
  char unplusmode[MAXLEN];	/* used to undo modechanges */
  char unplusparams[MAXLEN];

  /* these are here merely to make things clearer */
  char *nick;
  char *banstring;
  char *key;
  char *limit;
  UserHostStruct *user;
  static time_t currenttime;
  CHAN_list *ChanPtr;


  currenttime = getthetime();

  Debug(DBGINFO, "Mode chane from %s: %s", from, rest);

  strcpy(unminmode, "");	/* min-modes, i.e. -bi */
  strcpy(unminparams, "");	/* it's parameters */
  strcpy(unplusmode, "");	/* plus-modes, i.e. +oo */
  strcpy(unplusparams, "");	/* parameters */

  channel = strtok(rest, " ");
  if (!strcasecmp(channel, current_nick))	/* if target is me... */
    return;			/* mode NederServ +i etc */
  ChanPtr = search_chan(channel);

  if (strchr(from, '@') != NULL)
  {
    update_userhostlist(from, channel, currenttime);
    user = find_userhost(from);
  }
  else
    user = NULL;

  mode = strtok(NULL, "\n\0");

  params = strchr(mode, ' ') + 1;
  chanchars = strtok(mode, " ");
  while (*chanchars)
  {
    switch (*chanchars)
    {
    case '+':
      sign = '+';
      globsign = '+';
      break;
    case '-':
      sign = '-';
      globsign = '-';
      break;
    case 'o':
      nick = strtok(params, " ");
      params = strtok(NULL, "\0\n");
      if (!nick)
	break;
      if (sign == '+')
      {
        if(!strcasecmp(nick, current_nick))
        {  /* Its me! */
          if (is_in_list(StealList, channel))
          {
            channel_massdeop(channel, "*!*@*");
            remove_from_list(&StealList, channel);
          }
        }
        if (user)
          user->totop++;
        /* Check for NET-RIDE. This is dumb on servers with timestamps.. */
        if(ChanPtr->Settings[RIDEPROT] && (strchr(from, '@') == NULL) && (userlevel(username(nick), ChanPtr) <= 0))
        {
          strcat(unminmode, "o");
          strcat(unminparams, nick);
          strcat(unminparams, " ");
          netsplit = TRUE;
          did_change = TRUE;
        }
        else if(shitlevel(username(nick), ChanPtr) > 0)
        {
          strcat(unminmode, "o");
          strcat(unminparams, nick);
          strcat(unminparams, " ");
          send_to_user(from, "Sorry sir, but %s is shitlisted!", nick);
          did_change = TRUE;
        }
        else if(user && ChanPtr->Settings[STRICTOP] != 0)
        {
          switch(ChanPtr->Settings[STRICTOP])
          {
            case 1:  /* Deop non-users opped by other non-users. */
                    if(!userlevel(username(nick), ChanPtr) && 
                       !userlevel(from, ChanPtr))
                    {
                        strcat(unminmode, "o");
                        strcat(unminparams, nick);
                        strcat(unminparams, " ");
                        did_change = TRUE;
                    }
                    break;
             case 2: /* Deop non-users opped by anyone */
                     if(!userlevel(username(nick), ChanPtr))
                     {
                       strcat(unminmode, "o");
                       strcat(unminparams, nick);
                       strcat(unminparams, " ");
                       did_change = TRUE;
                     }
                     break;
             case 3: /* Deop non-users and the opper */
                    if(!userlevel(username(nick), ChanPtr) &&
                       !userlevel(from, ChanPtr))
                    {
                        strcat(unminmode, "o");
                        strcat(unminparams, nick);
                        strcat(unminparams, " ");

                        strcat(unplusmode, "o");
                        strcat(unplusparams, getnick(user->name));
                        strcat(unplusparams, " ");
                        did_change = TRUE;
                    }
                    break;
             default: /* Unknown setting */
                     Debug(DBGWARNING, "Unknown STRICTOP setting of %d", ChanPtr->Settings[STRICTOP]);
                     break;
          }
        }
      }
      else /* sign == '-' */
      {
        if (user)
        {
          user->deopnum++;
          user->totdeop++;
        }
        if (protlevel(username(nick), ChanPtr) &&  /* Test for Setting[PROTECT] done in protlevel().. */
            !shitlevel(username(nick), ChanPtr) &&
            strcasecmp(getnick(from), nick) &&
            strcasecmp(getnick(from), current_nick))
        {
          strcat(unplusmode, "o");
          strcat(unplusparams, nick);
          strcat(unplusparams, " ");
          botlog(botlogfile, "%s deopped on %s by %s -- attempted reop", username(nick), channel, from);
          prot_action(from, username(nick), channel);
          did_change = TRUE;
        }
      }
      break;
    case 'v':
      nick = strtok(params, " ");
      params = strtok(NULL, "\0\n");
      if(!nick)
       break;
      if (sign == '+')
        ;
      else
        ;
      break;
    case 'b':
      banstring = strtok(params, " ");
      params = strtok(NULL, "\0\n");
      if (!banstring)
       break;
      if (sign == '+')
      {
        add_ban(&(ChanPtr->banned), banstring); 
        if(user)
        {
          user->bannum++;
          user->totban++;
        }
        if(strcasecmp(getnick(from), current_nick)) /* If its not me doing it.. */
        {
           if(strcasecmp(getnick(banstring), "*") &&
              protlevel(username(getnick(banstring)), ChanPtr))
           {
              botlog(botlogfile, "%s banned on %s by %s -- attempted unban", banstring, channel, from);
              prot_action(from, banstring, channel);
              strcat(unminmode, "b");
              strcat(unminparams, banstring);
              strcat(unminparams, " ");
              did_change = TRUE;
           }
           else if(!strcasecmp(getnick(banstring), "*") &&
                   (protlevel(banstring, ChanPtr) || protlevelbm(banstring, ChanPtr)))
           {
              botlog(botlogfile, "%s banned on %s by %s -- attempted unban", banstring, channel, from);
              prot_action(from, banstring, channel);
              strcat(unminmode, "b");
              strcat(unminparams, banstring);
              strcat(unminparams, " ");
              did_change = TRUE;
           }
        }
      }
      else /* mode == '-' */
      {
        delete_ban(&(ChanPtr->banned), banstring); 
        /* Nothing */
      }
      break;
    case 'p':
      if (sign == '+')
      {
        if(ChanPtr->MinModes & CMODEp)
        {
           switch(ChanPtr->Settings[ENFM])
           {
             case 0: /* (Co)Owners only */
                     if(userlevel(from, ChanPtr) < ChanCoOwner)
                     {
                        strcat(unminmode, "p");
                        did_change = TRUE;
                     }
                     break;
             case 1: /* (Co)owners and chan helpers */
                     if(userlevel(from, ChanPtr) < ChanHelper)
                     {
                        strcat(unminmode, "p");
                        did_change = TRUE;
                     }
                     break;
             case 2: /* CoOwners, ChanHelpers, and chanops */
                     if(userlevel(from, ChanPtr) < ChanHelper)
                     {
                        strcat(unminmode, "p");
                        did_change = TRUE;
                     } 
                     break;
            default: /* Unknown mode */
                      Debug(DBGWARNING, "Unknown ENFM mode %d", ChanPtr->Settings[ENFM]);
                      break;
           }
        }
      }
      else  /* mode == '-' */
      {
        if(ChanPtr->PlusModes & CMODEp)
        {
           switch(ChanPtr->Settings[ENFM])
           {
             case 0: /* (Co)Owners only */
                     if(userlevel(from, ChanPtr) < ChanCoOwner)
                     {
                        strcat(unplusmode, "p");
                        did_change = TRUE;
                     }
                     break;
             case 1: /* (Co)owners and chan helpers */
                     if(userlevel(from, ChanPtr) < ChanHelper)
                     {
                        strcat(unplusmode, "p");
                        did_change = TRUE;
                     } 
                     break;
             case 2: /* CoOwners, ChanHelpers, and chanops */
                     if(userlevel(from, ChanPtr) < ChanHelper)
                     {
                        strcat(unplusmode, "p");
                        did_change = TRUE;
                     }
                     break;
            default: /* Unknown mode */
                      Debug(DBGWARNING, "Unknown ENFM mode %d", ChanPtr->Settings[ENFM]);
                      break;
           }
        }
      }
      break;
    case 's':
      if (sign == '+')
      {
        if(ChanPtr->MinModes & CMODEs)
        {
           switch(ChanPtr->Settings[ENFM])
           {
             case 0: /* (Co)Owners only */
                     if(userlevel(from, ChanPtr) < ChanCoOwner)
                     {
                        strcat(unminmode, "s");
                        did_change = TRUE;
                     }
                     break;
             case 1: /* (Co)owners and chan helpers */
                     if(userlevel(from, ChanPtr) < ChanHelper)
                     {
                        strcat(unminmode, "s");
                        did_change = TRUE;
                     }
                     break;
             case 2: /* CoOwners, ChanHelpers, and chanops */
                     if(userlevel(from, ChanPtr) < ChanHelper)
                     {
                        strcat(unminmode, "s");
                        did_change = TRUE;
                     } 
                     break;
            default: /* Unknown mode */
                      Debug(DBGWARNING, "Unknown ENFM mode %d", ChanPtr->Settings[ENFM]);
                      break;
           }
        }
      }
      else  /* mode == '-' */
      {
        if(ChanPtr->PlusModes & CMODEs)
        {
           switch(ChanPtr->Settings[ENFM])
           {
             case 0: /* (Co)Owners only */
                     if(userlevel(from, ChanPtr) < ChanCoOwner)
                     {
                        strcat(unplusmode, "s");
                        did_change = TRUE;
                     }
                     break;
             case 1: /* (Co)owners and chan helpers */
                     if(userlevel(from, ChanPtr) < ChanHelper)
                     {
                        strcat(unplusmode, "s");
                        did_change = TRUE;
                     } 
                     break;
             case 2: /* CoOwners, ChanHelpers, and chanops */
                     if(userlevel(from, ChanPtr) < ChanHelper)
                     {
                        strcat(unplusmode, "s");
                        did_change = TRUE;
                     }
                     break;
            default: /* Unknown mode */
                      Debug(DBGWARNING, "Unknown ENFM mode %d", ChanPtr->Settings[ENFM]);
                      break;
           }
        }
      }
      break;
    case 'm':
      if (sign == '+')
      {
        if(ChanPtr->MinModes & CMODEm)
        {
           switch(ChanPtr->Settings[ENFM])
           {
             case 0: /* (Co)Owners only */
                     if(userlevel(from, ChanPtr) < ChanCoOwner)
                     {
                        strcat(unminmode, "m");
                        did_change = TRUE;
                     }
                     break;
             case 1: /* (Co)owners and chan helpers */
                     if(userlevel(from, ChanPtr) < ChanHelper)
                     {
                        strcat(unminmode, "m");
                        did_change = TRUE;
                     }
                     break;
             case 2: /* CoOwners, ChanHelpers, and chanops */
                     if(userlevel(from, ChanPtr) < ChanHelper)
                     {
                        strcat(unminmode, "m");
                        did_change = TRUE;
                     } 
                     break;
            default: /* Unknown mode */
                      Debug(DBGWARNING, "Unknown ENFM mode %d", ChanPtr->Settings[ENFM]);
                      break;
           }
        }
      }
      else  /* mode == '-' */
      {
        if(ChanPtr->PlusModes & CMODEm)
        {
           switch(ChanPtr->Settings[ENFM])
           {
             case 0: /* (Co)Owners only */
                     if(userlevel(from, ChanPtr) < ChanCoOwner)
                     {
                        strcat(unplusmode, "m");
                        did_change = TRUE;
                     }
                     break;
             case 1: /* (Co)owners and chan helpers */
                     if(userlevel(from, ChanPtr) < ChanHelper)
                     {
                        strcat(unplusmode, "m");
                        did_change = TRUE;
                     } 
                     break;
             case 2: /* CoOwners, ChanHelpers, and chanops */
                     if(userlevel(from, ChanPtr) < ChanHelper)
                     {
                        strcat(unplusmode, "m");
                        did_change = TRUE;
                     }
                     break;
            default: /* Unknown mode */
                      Debug(DBGWARNING, "Unknown ENFM mode %d", ChanPtr->Settings[ENFM]);
                      break;
           }
        }
      }
      break;
    case 't':
      if (sign == '+')
      {
        if(ChanPtr->MinModes & CMODEt)
        {
           switch(ChanPtr->Settings[ENFM])
           {
             case 0: /* (Co)Owners only */
                     if(userlevel(from, ChanPtr) < ChanCoOwner)
                     {
                        strcat(unminmode, "t");
                        did_change = TRUE;
                     }
                     break;
             case 1: /* (Co)owners and chan helpers */
                     if(userlevel(from, ChanPtr) < ChanHelper)
                     {
                        strcat(unminmode, "t");
                        did_change = TRUE;
                     }
                     break;
             case 2: /* CoOwners, ChanHelpers, and chanops */
                     if(userlevel(from, ChanPtr) < ChanHelper)
                     {
                        strcat(unminmode, "t");
                        did_change = TRUE;
                     } 
                     break;
            default: /* Unknown mode */
                      Debug(DBGWARNING, "Unknown ENFM mode %d", ChanPtr->Settings[ENFM]);
                      break;
           }
        }
      }
      else  /* mode == '-' */
      {
        if(ChanPtr->PlusModes & CMODEt)
        {
           switch(ChanPtr->Settings[ENFM])
           {
             case 0: /* (Co)Owners only */
                     if(userlevel(from, ChanPtr) < ChanCoOwner)
                     {
                        strcat(unplusmode, "t");
                        did_change = TRUE;
                     }
                     break;
             case 1: /* (Co)owners and chan helpers */
                     if(userlevel(from, ChanPtr) < ChanHelper)
                     {
                        strcat(unplusmode, "t");
                        did_change = TRUE;
                     } 
                     break;
             case 2: /* CoOwners, ChanHelpers, and chanops */
                     if(userlevel(from, ChanPtr) < ChanHelper)
                     {
                        strcat(unplusmode, "t");
                        did_change = TRUE;
                     }
                     break;
            default: /* Unknown mode */
                      Debug(DBGWARNING, "Unknown ENFM mode %d", ChanPtr->Settings[ENFM]);
                      break;
           }
        }
      }
      break;
    case 'i':
      if (sign == '+')
      {
        if(ChanPtr->MinModes & CMODEi)
        {
           switch(ChanPtr->Settings[ENFM])
           {
             case 0: /* (Co)Owners only */
                     if(userlevel(from, ChanPtr) < ChanCoOwner)
                     {
                        strcat(unminmode, "i");
                        did_change = TRUE;
                     }
                     break;
             case 1: /* (Co)owners and chan helpers */
                     if(userlevel(from, ChanPtr) < ChanHelper)
                     {
                        strcat(unminmode, "i");
                        did_change = TRUE;
                     }
                     break;
             case 2: /* CoOwners, ChanHelpers, and chanops */
                     if(userlevel(from, ChanPtr) < ChanHelper)
                     {
                        strcat(unminmode, "i");
                        did_change = TRUE;
                     } 
                     break;
            default: /* Unknown mode */
                      Debug(DBGWARNING, "Unknown ENFM mode %d", ChanPtr->Settings[ENFM]);
                      break;
           }
        }
      }
      else  /* mode == '-' */
      {
        if(ChanPtr->PlusModes & CMODEi)
        {
           switch(ChanPtr->Settings[ENFM])
           {
             case 0: /* (Co)Owners only */
                     if(userlevel(from, ChanPtr) < ChanCoOwner)
                     {
                        strcat(unplusmode, "i");
                        did_change = TRUE;
                     }
                     break;
             case 1: /* (Co)owners and chan helpers */
                     if(userlevel(from, ChanPtr) < ChanHelper)
                     {
                        strcat(unplusmode, "i");
                        did_change = TRUE;
                     } 
                     break;
             case 2: /* CoOwners, ChanHelpers, and chanops */
                     if(userlevel(from, ChanPtr) < ChanHelper)
                     {
                        strcat(unplusmode, "i");
                        did_change = TRUE;
                     }
                     break;
            default: /* Unknown mode */
                      Debug(DBGWARNING, "Unknown ENFM mode %d", ChanPtr->Settings[ENFM]);
                      break;
           }
        }
      }
      break;
    case 'n':
      if (sign == '+')
      {
        if(ChanPtr->MinModes & CMODEn)
        {
           switch(ChanPtr->Settings[ENFM])
           {
             case 0: /* (Co)Owners only */
                     if(userlevel(from, ChanPtr) < ChanCoOwner)
                     {
                        strcat(unminmode, "n");
                        did_change = TRUE;
                     }
                     break;
             case 1: /* (Co)owners and chan helpers */
                     if(userlevel(from, ChanPtr) < ChanHelper)
                     {
                        strcat(unminmode, "n");
                        did_change = TRUE;
                     }
                     break;
             case 2: /* CoOwners, ChanHelpers, and chanops */
                     if(userlevel(from, ChanPtr) < ChanHelper)
                     {
                        strcat(unminmode, "n");
                        did_change = TRUE;
                     } 
                     break;
            default: /* Unknown mode */
                      Debug(DBGWARNING, "Unknown ENFM mode %d", ChanPtr->Settings[ENFM]);
                      break;
           }
        }
      }
      else  /* mode == '-' */
      {
        if(ChanPtr->PlusModes & CMODEn)
        {
           switch(ChanPtr->Settings[ENFM])
           {
             case 0: /* (Co)Owners only */
                     if(userlevel(from, ChanPtr) < ChanCoOwner)
                     {
                        strcat(unplusmode, "n");
                        did_change = TRUE;
                     }
                     break;
             case 1: /* (Co)owners and chan helpers */
                     if(userlevel(from, ChanPtr) < ChanHelper)
                     {
                        strcat(unplusmode, "n");
                        did_change = TRUE;
                     } 
                     break;
             case 2: /* CoOwners, ChanHelpers, and chanops */
                     if(userlevel(from, ChanPtr) < ChanHelper)
                     {
                        strcat(unplusmode, "n");
                        did_change = TRUE;
                     }
                     break;
            default: /* Unknown mode */
                      Debug(DBGWARNING, "Unknown ENFM mode %d", ChanPtr->Settings[ENFM]);
                      break;
           }
        }
      }
      break;
    case 'k':
      key = strtok(params, " ");
      params = strtok(NULL, "\0\n");
      if(key)
      {
        strncpy(ChanPtr->CurrentKey, key, 29);
        ChanPtr->CurrentKey[29] = '\0';
      }
      if (sign == '+')
      {
        if(ChanPtr->MinModes & CMODEk)
        {
           switch(ChanPtr->Settings[ENFM])
           {
             case 0: /* (Co)Owners only */
                     if(userlevel(from, ChanPtr) < ChanCoOwner)
                     {
                        strcat(unminmode, "k");
                        strcat(unminparams, key);
                        strcat(unminparams, " ");
                        did_change = TRUE;
                     }
                     break;
             case 1: /* (Co)owners and chan helpers */
                     if(userlevel(from, ChanPtr) < ChanHelper)
                     {
                        strcat(unminmode, "k");
                        strcat(unminparams, key);
                        strcat(unminparams, " ");
                        did_change = TRUE;
                     }
                     break;
             case 2: /* CoOwners, ChanHelpers, and chanops */
                     if(userlevel(from, ChanPtr) < ChanHelper)
                     {
                        strcat(unminmode, "k");
                        strcat(unminparams, key);
                        strcat(unminparams, " ");
                        did_change = TRUE;
                     } 
                     break;
            default: /* Unknown mode */
                      Debug(DBGWARNING, "Unknown ENFM mode %d", ChanPtr->Settings[ENFM]);
                      break;
           }
        }
      }
      else /* sign == '-' */
      {
        if((ChanPtr->PlusModes & CMODEk) && ChanPtr->Key)
        {
           switch(ChanPtr->Settings[ENFM])
           {
             case 0: /* (Co)Owners only */
                     if(userlevel(from, ChanPtr) < ChanCoOwner)
                     {
                        strcat(unplusmode, "k");
                        strcat(unplusparams, ChanPtr->Key);
                        strcat(unplusparams, " ");
                        did_change = TRUE;
                     }
                     break;
             case 1: /* (Co)owners and chan helpers */
                     if(userlevel(from, ChanPtr) < ChanHelper)
                     {
                        strcat(unplusmode, "k");
                        strcat(unplusparams, ChanPtr->Key);
                        strcat(unplusparams, " ");
                        did_change = TRUE;
                     } 
                     break;
             case 2: /* CoOwners, ChanHelpers, and chanops */
                     if(userlevel(from, ChanPtr) < ChanHelper)
                     {
                        strcat(unplusmode, "k");
                        strcat(unplusparams, ChanPtr->Key);
                        strcat(unplusparams, " ");
                        did_change = TRUE;
                     }
                     break;
            default: /* Unknown mode */
                      Debug(DBGWARNING, "Unknown ENFM mode %d", ChanPtr->Settings[ENFM]);
                      break;
           }
        }
      }
      break;
    case 'l':
      if (sign == '+')
      {
	limit = strtok(params, " ");
	if (limit && (atoi(limit) != 0))
	  params = strtok(NULL, "\0\n");

        if(ChanPtr->MinModes & CMODEl)
        {
           switch(ChanPtr->Settings[ENFM])
           {
             case 0: /* (Co)Owners only */
                     if(userlevel(from, ChanPtr) < ChanCoOwner)
                     {
                        strcat(unminmode, "l");
                        did_change = TRUE;
                     }
                     break;
             case 1: /* (Co)owners and chan helpers */
                     if(userlevel(from, ChanPtr) < ChanHelper)
                     {
                        strcat(unminmode, "l");
                        did_change = TRUE;
                     }
                     break;
             case 2: /* CoOwners, ChanHelpers, and chanops */
                     if(userlevel(from, ChanPtr) < ChanHelper)
                     {
                        strcat(unminmode, "l");
                        did_change = TRUE;
                     } 
                     break;
            default: /* Unknown mode */
                      Debug(DBGWARNING, "Unknown ENFM mode %d", ChanPtr->Settings[ENFM]);
                      break;
           }
        }
      }
      else /* sign == '-' */
      {
        if(((ChanPtr->PlusModes & CMODEl)) && ChanPtr->Limit)
        {
           switch(ChanPtr->Settings[ENFM])
           {
             case 0: /* (Co)Owners only */
                     if(userlevel(from, ChanPtr) < ChanCoOwner)
                     {
                        strcat(unplusmode, "k");
                        strcat(unplusparams, ChanPtr->Limit);
                        strcat(unplusparams, " ");
                        did_change = TRUE;
                     }
                     break;
             case 1: /* (Co)owners and chan helpers */
                     if(userlevel(from, ChanPtr) < ChanHelper)
                     {
                        strcat(unplusmode, "k");
                        strcat(unplusparams, ChanPtr->Limit);
                        strcat(unplusparams, " ");
                        did_change = TRUE;
                     } 
                     break;
             case 2: /* CoOwners, ChanHelpers, and chanops */
                     if(userlevel(from, ChanPtr) < ChanHelper)
                     {
                        strcat(unplusmode, "k");
                        strcat(unplusparams, ChanPtr->Limit);
                        strcat(unplusparams, " ");
                        did_change = TRUE;
                     }
                     break;
            default: /* Unknown mode */
                      Debug(DBGWARNING, "Unknown ENFM mode %d", ChanPtr->Settings[ENFM]);
                      break;
           }
        }
      }
      break;
    default:
           Debug(DBGINFO, "Unknown mode (%c)", *chanchars);
           break;
    }
    chanchars++;
  }
  /* restore unwanted modechanges */
  if (netsplit)
    botlog(botlogfile, "Op by net split detected");
  if (did_change)
  {
    botlog(botlogfile, "Enf.Modes: MODE %s +%s-%s %s %s",
	   channel, unplusmode, unminmode, unplusparams, unminparams);
    sendmode(channel, "+%s-%s %s %s", unplusmode,
	     unminmode, unplusparams, unminparams);
  }
  if(netsplit)
    bot_reply(channel, 3);
  if(user)
  {
    if(currenttime - user->bantime >= 10)
    {
      user->bantime = currenttime;
      user->bannum = 0;
    }
    if(currenttime - user->deoptime >= 10)
    {
      user->deoptime = currenttime;
      user->deopnum = 0;
    }
/* this stuff disabled till i re-do it.
    sprintf(buffer, "\002NO MASS DEOPS HERE!!\002");
    if(user->deopnum >= massdeoplimit)
    {
      massprot_action(from, channel, buffer);
      botlog(botlogfile, "Mass-deop detected by %s", from);
    }
    sprintf(buffer, "\x2TRY BANNING NOW!\x2");
    if (user->bannum >= massbanlimit)
    {
      massprot_action(from, channel, buffer);
      botlog(botlogfile, "Mass-ban detected by %s", from);
    }
*/
  }
  return;
}


void on_serv_notice(char *from, char *to, char *msg)
{
  char *person = NULL, *temp = NULL, *nickname = "", *nuh;
  char buffer[MAXLEN];

  person = strstr(msg, "Received KILL message for");
  if (!person)
    return;
  person = strstr(msg, "for");
  if (!person)
    nickname = "<UNKNOWN>";
  else
  {
    person = strchr(person, ' ');
    if (person)
      person++;
    if (person)
    {
      nickname = person;
      person = strchr(nickname, '.');
      if (person)
      {
	*person = '\0';
	person++;
	msg = person;
      }
    }
    else
      nickname = "<UNKNOWN>";
  }

  if (person)
    person = strstr(msg, "From");
  if (!person)
    return;

  temp = strchr(person, '(');
  if (temp)
    temp++;
  person = strchr(person, ' ');
  if (!person)
    return;
  person++;
  if (!person)
    return;
  person = strtok(person, " ");
  if (!person)
    return;

  if (strchr(person, '.'))
    return;
  if (temp && (strrchr(temp, ')')))
    *strchr(temp, ')') = '\0';
  if (!temp)
    temp = person;

  nuh = getuserhost(person);
  if (!*nuh)
    nuh = getuserhost2(person);
  if (!*nuh)
    return;
  sprintf(buffer, "%s!%s", person, nuh);
  on_kill(buffer, nickname, temp);
}

void on_kill(char *from, char *nick, char *reason)
{
  UserHostStruct *user;

  update_userhostlist(from, "", getthetime());
  user = find_userhost(from);
  if (user)
  {
    user->totkill++;
    user->killtime = getthetime();
  }
  if (Globals->Settings[LOGGING] > 0)
  {
    botlog(killfile, "%s killed %s (%s)", from, nick, reason);
  }
}


void on_command(char *from, char *to, char *channel, char *command, char *rest)
{
  int i, ii;
  DCC_list *userclient;
  CHAN_list *ChanPtr;

  if (locked == TRUE)
  {	/* If the bot is locked, and the command is NOT 'unlock', return without doing anything. */
    if ((strcasecmp(command, "unlock") != 0) && (strcasecmp(command + 1, "unlock") != 0))
      return;
  }
  if (!strcasecmp(channel, "Globals"))
    ChanPtr = Globals;
  else
    ChanPtr = search_chan(channel);
  for (i = 0; on_msg_commands[i].name != NULL; i++)
  {
    if (!strcasecmp(on_msg_commands[i].name, command))
    {
/** We found a match. Now to see if its ok to run it **/
      if ((userlevel(from, ChanPtr) >= on_msg_commands[i].userlevel) &&
	  (shitlevel(from, ChanPtr) <= on_msg_commands[i].shitlevel) )
      {
        if((on_msg_commands[i].userlevel >= Peon) && !isauth(from, ChanPtr))
        {
           send_to_user(from, "\002Sir, since your host is marked as (I)nsecure, I need verification you are truely you. Please /msg %s auth <handle> <password>.", current_nick);
           return;
        }
/** The user has access, Check if DCC is needed... **/
        userclient = search_list("chat", from, DCC_CHAT, 1);
#ifdef REQUIREDCC
        if (on_msg_commands[i].forcedcc && (!userclient || (userclient->flags & DCC_WAIT)))
        {
          dcc_chat(from, to, dcctmp);	/* dcctmp has enough info to re-do their command after dcc is established. */
          sleep(1);
          sendnotice(getnick(from), "\002Sir, please accept my dcc chat request to continue [%s %s]", to, dcctmp);
          return;
        }
#endif
        if (on_msg_commands[i].needchan)
        {
          if (!channel || !(*channel))
          {
//            send_to_user(from, "\002What channel?\002");
//            return;
              ChanPtr = Globals;
              channel = "Globals";
          }
          if (!ChanPtr)
          {
            send_to_user(from, "\002Channel %s is not registered with me.\002", channel);
            return;
          }
        }
        ACTIVE_USERLEVEL = userlevel(from, ChanPtr);
        on_msg_commands[i].function(from, channel, rest, to, ChanPtr);
        ACTIVE_USERLEVEL = 0;
        for (ii = 8; ii >= 0; ii--)
         strcpy(lastcmds[ii + 1], lastcmds[ii]);
        strcpy(lastcmds[0], on_msg_commands[i].name);
        strcat(lastcmds[0], " done by ");
        strcat(lastcmds[0], left(from, 50));
        return;
      }
      else
      {
        send_to_user(from, "\002Access denied to [\002%s\002]\002", command);
        return;
      }
    }
  }
  send_to_user(from, "\002I'm sorry Sir, That command [\002%s\002] is not known to me\002", command);
}


void on_msg(char *from, char *to, char *msg)
{
  static time_t currenttime;
  char *command;
  char *rest;
  UserHostStruct *user;
  char *channel;

  strcpy(dcctmp, msg);
  currenttime = getthetime();
  if (ischannel(to))
  {				/* A public privmsg to a channel */
    update_userhostlist(from, to, currenttime);
    update_idletime(to, from);
    user = find_userhost(from);
    if (user)
      update_flood(to, from);
    if (msg[0] == cmdchar)
    {
      if(msg[1] == '#')	/* User spesified channel */
      {
	channel = strtok(&msg[1], " ");
	command = strtok(NULL, " ");
	rest = strtok(NULL, "\0");
      }
      else if(msg[1] == cmdchar)
      {
	channel = "globals";
	command = strtok(&msg[2], " ");
	rest = strtok(NULL, "\0");
      }
      else /* Use channel provided */
      {
        if(!isalnum(msg[1]))
           return; // ignore ppls ramblings not intended for us
	channel = to;
	command = strtok(&msg[1], " ");
	rest = strtok(NULL, "\0");
      }
      if(!command)
	return;			/* Empty command */
      if(!channel)
	channel = "\0\0\0";
      if(!rest)
	rest = "\0\0\0";
      on_command(from, to, channel, command, rest);
    }
    else
    {
      on_public(from, to, msg);
    }
  }
  else
  {				/*A private message to me */
    if (msg[0] == cmdchar)
    {
      msg++;
      channel = "Globals";
      command = strtok(msg, " ");
      rest = strtok(NULL, "\0");
    }
    else if (msg[0] == '#')		/* User spesified channel */
    {
      channel = strtok(msg, " ");
      command = strtok(NULL, " ");
      rest = strtok(NULL, "\0");
    }
    else /* No channel */
    {
      channel = "\0\0\0";
      command = strtok(msg, " ");
      rest = strtok(NULL, "\0");
    }
    if(!channel)
      channel = "\0\0\0";
    if(!command)
      return;			/* Empty command */
    if(!rest)
      rest = "\0\0\0";
    to = "\0\0\0";
    showspymsg(from, command);
    if((Globals->Settings[LOGGING] > 2) || holdingnick)
      botlog(pmlogfile, "[%s] %s", from, command);
    on_command(from, to, channel, command, rest);
  }
}


void on_public(char *from, char *to, char *rest)
{
  ListStruct *Dummy;
  int i;
  char buf[MAXLEN];
  CHAN_list *ChanPtr;

  Debug(DBGINFO, "<%s/%s> %s", from, to, rest);

  ChanPtr = search_chan(to);
  if(!ChanPtr)
  {
     Debug(DBGERROR, "Cant find %s in channel list\n");
     return;
  }
  sprintf(buf, "\002%s:\002<%s> %s", to, getnick(from), rest);
  send_spy_chan(to, buf);
  if(ChanPtr->Settings[TALKING])
  {
    converse(from, to, current_nick, rest);
  }
  if (ChanPtr->Settings[PUBSEEN] && (strcasestr(rest, "seen ") &&
		     (strchr(rest, '?') || strcasestr(rest, "anyone seen") ||
		      strcasestr(rest, "someone seen"))))
  {
    char *temp, stmp[52];
    int stmpcounter, st;

    temp = strcasestr(rest, "seen");
    if (temp)
    {
      temp = strchr(temp, ' ');
      if (temp)
      {
	temp++;
	stmpcounter = 0;
	st = 0;
	while ((temp[st] != '\0') && (st < 50))
	{
	  if (temp[st] != '?')
	  {
	    stmp[stmpcounter] = temp[st];
	    stmpcounter++;
	  }
	  st++;
	}
	stmp[stmpcounter] = '\0';
	strcpy(temp, stmp);
	if (temp && *temp)
	{
	  pubseen = 1;
	  c_seen(from, to, temp, to, ChanPtr);
	  pubseen = 0;
	}
      }
    }
  }
  if(is_in_list(LogChatList, to))
  {
    sprintf(buf, "<%s> ", getnick(from));
    strcat(buf, rest);
    logchat(to, buf);
  }
  if(is_in_list(KickSays, rest) && is_in_list(KickSayChan, to))
  {
    Dummy = find_list(KickSays, rest);
    if(!BannedWordOk(ChanPtr->Settings[KSLEVEL], userlevel(from, ChanPtr)))
    {
      sprintf(buf, "ACTION overhears %s say %s", getnick(from),
	      Dummy->name);
      send_ctcp(to, buf);
      sprintf(buf, "ACTION sticks a sock in %s's mouth and...", getnick(from));
      send_ctcp(to, buf);
      sendkick(to, getnick(from), "\002You're outta here\002");
      sendprivmsg(to, "\002%s shouldn't have said that!!!\002", getnick(from));
      send_to_user(from, "%cCareful, %s is banned!!!\002", '\002', Dummy->name);
    }
    else
    {
      sprintf(buf, "ACTION overhears %s say a banned word, but overlooks it", getnick(from));
      send_ctcp(to, buf);
    }
  }
  if(!holdingnick && !userlevel(from, ChanPtr) && ChanPtr->Settings[WM])
  {
    i = getrandom(0, numwelcome);
    if (i == numwelcome)
      i = 0;
    if (!matches("Re", rest) || !matches("Hello", rest) ||
	!matches("Hi", rest) || !matches("Hey", rest) ||
        !matches("howdy", rest) || !matches("Bonjour", rest) )
      sendwelcome(welcomemess[i], getnick(from), to);
    sprintf(buf, "*Hey %s*", current_nick);
    if (!matches("*Hey All*", rest) || !matches(buf, rest))
      sendwelcome(welcomemess[i], getnick(from), to);
    sprintf(buf, "*Re %s*", current_nick);
    if (!matches("*Re All*", rest) || !matches(buf, rest))
     sendwelcome(welcomemess[i], getnick(from), to);
    sprintf(buf, "*Hi %s*", current_nick);
    if (!matches("*Hi All*", rest) || !matches(buf, rest))
     sendwelcome(welcomemess[i], getnick(from), to);
    sprintf(buf, "*Hello %s*", current_nick);
    if (!matches("*Hello All*", rest) || !matches(buf, rest))
      sendwelcome(welcomemess[i], getnick(from), to);

    sprintf(buf, "*Bye %s*", current_nick);
    if (!matches("*Bye All*", rest) || !matches(buf, rest))
      sendprivmsg(to, "Bye %s", getnick(from));
  }
}

void on_part(char *from, char *channel)
{
  char buffer[MAXLEN];

  sprintf(buffer, "\x2%s has left channel %s\x2", from, channel);
  send_spy_chan(channel, buffer);
  sprintf(buffer, "%s has left channel %s", from, channel);
  if (is_in_list(LogChatList, channel))
    logchat(channel, buffer);
}

void on_nick(char *from, char *newnick)
{
   AuthStruct AuthPtr;
   extern AuthStruct AuthList;
   char *uhost;

   
   /* If a user changes nicknames and we see it, we need to
    * take it into account for things like DCC ownership
    * and password authentication. 
    */
    for(AuthPtr = AuthList;AuthPtr;AuthPtr = AuthPtr->next)
    {
         if(!strcasecmp(from, AuthPtr->userhost))
         {
             uhost = strchr(from, '!');
             uhost++;
             sprintf(AuthPtr->userhost, "%s!%s", newnick, uhost);
             return;
         }
    }
}

void on_quit(char *from)
{
    /* We want to delete someones authentication if they quit.. */
    /* TODO : If they are on a channel which is being spied on,
     * tell the spy about the quit.
     * send_spy_chan(channel, buf);
     */
    /* TODO : Is this supposed to free() AuthPtr? */

    

    AuthStruct AuthPtr, PrevPtr;
    extern AuthStruct AuthList;

    PrevPtr = NULL;
    for(AuthPtr = AuthList;AuthPtr;AuthPtr = AuthPtr->next)
    {
        if(!strcasecmp(from, AuthPtr->userhost))
        {
            if(PrevPtr)
               PrevPtr->next = AuthPtr->next;
            else
               AuthList = AuthPtr->next;
        }
        PrevPtr = AuthPtr;
    }
}

void on_kick(char *from, char *channel, char *nick, char *reason)
{
  char buffer[MAXLEN];
  CHAN_list *ChanPtr;

  UserHostStruct *user;
  static time_t currenttime;

  currenttime = getthetime();
  if (!reason)
    reason = "";

  ChanPtr = search_chan(channel);
  sprintf(buffer, "\x2%s has been kicked off channel %s by %s (%s)\x2",
	  nick, channel, getnick(from), reason);
  send_spy_chan(channel, buffer);
  if (is_in_list(LogChatList, channel))
  {
    sprintf(buffer, "%s has been kicked off channel %s by %s (%s)",
	    nick, channel, from, reason);
    logchat(channel, buffer);
  }

  user = find_userhost(from);

  if (user)
  {
    if (currenttime - user->kicktime > 10)
    {
      user->kicktime = currenttime;
      user->kicknum = 0;
    }
    user->kicknum++;
    user->totkick++;
/*
 * TODO : Why is this commented out? *
    if (user->kicknum >= masskicklimit)
    {
      sprintf(buffer, "\x2Too many Kicks by you\x2");
      botlog(botlogfile, "Mass-kick detected by %s", from);
      massprot_action(from, channel, buffer);
      user->kicknum = 0;
    }
*/
  }
  if (protlevel(username(nick), ChanPtr) &&
      !shitlevel(username(nick), ChanPtr) &&
      strcasecmp(getnick(from), nick) &&
      strcasecmp(getnick(from), current_nick))
  {
    botlog(botlogfile, "%s kicked from %s by %s", username(nick),
	   channel, from);
    prot_action(from, username(nick), channel);
  }
}

// show the responder list for givin chan.
void c_listresp(char *from, char *channel, char *rest, char *to, CHAN_list *ChanPtr)
{
  char *name;
  RESPONDER_list *rptr;

  name = strtok(rest, " ");
  send_to_user(from, "=--\002Name\002---=-----Expect/Send---------=");
  for(rptr=ChanPtr->Responder;rptr;rptr = rptr->next)
  {
     if(name)
     {
        if(matches(rptr->name, name))
        {
            continue; /* Skip */
        }
     }
     send_to_user(from, "%10s - Expect: %s", rptr->name, rptr->expect);
     send_to_user(from, "%10s -   Send: %s", rptr->name, rptr->send);
  }
  send_to_user(from, "=------- End of Responder list ---------=");
}

// Adds a responder to the list.
void c_addresp(char *from, char *channel, char *rest, char *to, CHAN_list *ChanPtr)
{
  char *name;
  char *expect;
  char *send;

  name = strtok(rest, " ");
  expect = strtok(NULL, ":");
  send = strtok(NULL, "\0");

  if(!(name && expect && send))
  {
    send_to_user(from, "Syntax: ADDRESP <name> <expect>:<send>");
    send_to_user(from, "Special strings: $C=Channel $N=nick");
    return;
  }
  AddChanResponder(ChanPtr, name, expect, send);
  send_to_user(from, "Added Responder %s as: Expect[%s] Send[%s].", name, expect, send);
  SaveDatFile(ChanPtr);
}

void c_set(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  char *option, *param;
  int i, j, value, curset;
  char ModeString[200];
  DCC_list *userclient;

  option = strtok(rest, " ");
  param = strtok(NULL, "\n");

  if (!option)
  {
#ifdef REQUIRE_DCC
    userclient = search_list("chat", from, DCC_CHAT, 1);
    if (!(userclient) || (userclient->flags & DCC_WAIT))
    {
      dcc_chat(from, to, dcctmp);	/* dcctmp has enough info to re-do their command after dcc is established. */
      sleep(1);
      send_to_user(from, "\002Sir, please accept my dcc chat request to continue [%s %s]", to, dcctmp);
      return;
    }
#endif
    send_to_user(from, "=---------- %s Settings ----------=", ChanPtr->name);
    send_to_user(from, "     \002Topic \002: %s", ChanPtr->topic);
    send_to_user(from, "     \002Modes \002: %s",
	    GetModeString(ModeString, ChanPtr->PlusModes, ChanPtr->MinModes, ChanPtr->Key, ChanPtr->Limit));
    for (i = 0; set_commands[i].name != NULL; i++)
    {
      send_to_user(from, "\002%10s\002 : %d  \002%s\002%s", set_commands[i].name,
		   ChanPtr->Settings[set_commands[i].index], set_commands[i].pre,
		   set_commands[i].settings[ChanPtr->Settings[set_commands[i].index]]);
    }
    send_to_user(from, "=-------------- End --------------=");
    return;
  }
  if (!strcasecmp(option, "TOPIC"))
  {
    if (!param)
      send_to_user(from, "\002Topic\002: %s", ChanPtr->topic);
    else
    {
      strcpy(ChanPtr->topic, param);
      send_to_user(from, "\002Default topic set to\002: %s", ChanPtr->topic);
      SaveDatFile(ChanPtr);
    }
    return;
  }
  else if (!strcasecmp(option, "MODES") || !strcasecmp(option, "MODE"))
  {
    if (!param)
    {
      send_to_user(from, "\002Mode: %s",
		   GetModeString(ModeString, ChanPtr->PlusModes,
				 ChanPtr->MinModes, ChanPtr->Key, ChanPtr->Limit));
    }
    else
    {
      if (userlevel(from, ChanPtr) < ChanCoOwner)
      {
	send_to_user(from, "\002You must be a ChanCoOwner or higher to set perm modes.\002");
	return;
      }
      ConvertModeString(param, &ChanPtr->PlusModes, &ChanPtr->MinModes, &ChanPtr->Key, &ChanPtr->Limit);
      send_command(1000, "MODE %s -k %s", ChanPtr->name, ChanPtr->CurrentKey);
      send_command(1000, "MODE %s %s", ChanPtr->name,
		     GetModeString(ModeString, ChanPtr->PlusModes, ChanPtr->MinModes,
				   ChanPtr->Key, ChanPtr->Limit));
      send_to_user(from, "\002Default mode set to: %s", ModeString);
      SaveDatFile(ChanPtr);
    }
    return;
  }
  if (param)
  {
    if (sscanf(param, "%d", &value) != 1)
    {
      send_to_user(from, "\002Error: Parameter must be a number\002");
      return;
    }
  }
  for (i = 0; set_commands[i].name != NULL; i++)
  {
    if (!strcasecmp(option, set_commands[i].name))
    {
      if (!param)		/* No parameter. */
      {
	curset = ChanPtr->Settings[set_commands[i].index];
	send_to_user(from, "=-----------------------------=");
	send_to_user(from, "\002%s\002: %s\002", set_commands[i].name, set_commands[i].pre);
	send_to_user(from, "=---- \002*\002 indicates current ----=");
	for (j = 0; j < 6; j++)
	{
	  if (set_commands[i].settings[j][0] == 0)
	    break;
	  if (j == curset)
	    send_to_user(from, "  %d: [\002*\002] %s", j, set_commands[i].settings[j]);
	  else
	    send_to_user(from, "  %d: [ ] %s", j, set_commands[i].settings[j]);
	}
	send_to_user(from, "=------------ End ------------=");
	return;
      }
      else if (userlevel(from, ChanPtr) < set_commands[i].changelevel)
      {
	send_to_user(from, "\002You don't have high enough access to change this setting.\002");
	return;
      }
      if ((value < set_commands[i].min) || (value > set_commands[i].max))
      {
	send_to_user(from, "\002Error, %s may only be %d-%d", set_commands[i].name,
		     set_commands[i].min, set_commands[i].max);
	return;
      }
      /* It checks out so lets change it */
      ChanPtr->Settings[set_commands[i].index] = value;
      send_to_user(from, "\002%s\002 set to %d. \"\002%s\002%s\"", set_commands[i].name,
		   value, set_commands[i].pre, set_commands[i].settings[value]);
      SaveDatFile(ChanPtr);
      return;
    }
  }
  send_to_user(from, "\002Unknown setting [\002%s\002].\002", option);
}

void c_levels(char *from, char *channel, char *rest, char *to, CHAN_list *ChanPtr)
{
   send_to_user(from, "=---\002Name\002---------\002Value\002---=");
   send_to_user(from, "  BotOwner      \002%d\017 - \002%d\017", BotOwner, 100);
   send_to_user(from, "  BotCoOwner    \002%d\017 - \002%d\017", BotCoOwner, BotOwner-1);
   send_to_user(from, "  ChanOwner     \002%d\017 - \002%d\017", ChanOwner, BotCoOwner-1);
   send_to_user(from, "  ChanCoOwner   \002%d\017 - \002%d\017", ChanCoOwner, ChanOwner-1);
   send_to_user(from, "  ChanHelper    \002%d\017 - \002%d\017", ChanHelper, ChanCoOwner-1);
   send_to_user(from, "  ChanOp        \002%d\017 - \002%d\017", ChanOp, ChanHelper-1);
   send_to_user(from, "  Peon          \002%d\017 - \002%d\017", Peon, ChanOp-1);
   send_to_user(from, "=------------------------=");
}

/* This allows some commands to be silently ignored. such as those done to
 * look for bots.  (HELLO, for example) */
void c_noop(char *from, char *channel, char *rest, char *to, CHAN_list *ChanPtr)
{
   return;
}

void c_auth(char *from, char *channel, char *rest, char *to, CHAN_list *ChanPtr)
{
   UserStruct *UsrPtr;
   char *password;
   char *handle;

   handle = strtok(rest, " ");
   password = strtok(NULL, " ");

   if(!password || !handle)
   {
      if(!handle)
        send_to_user(from, "\002No handle spesified.\002");
      else
        send_to_user(from, "\002No password spesified.\002");
      send_to_user(from, "\002Usage: AUTH <handle> <password>\002");
      return;
   }
   UsrPtr =  FindOpByHandle(ChanPtr, handle);
   if(!UsrPtr)
   {
      send_to_user(from, "%s Login incorrect: Username not found.", ChanPtr->name);
      return;
   }
   if(strcmp(UsrPtr->password, password))
   {
      send_to_user(from, "%s Login incorrect: Wrong password.", ChanPtr->name);
      return;
   }
   else     /* Add to the authlist & add host if missing */
   {
      AuthStruct AuthPtr;
      extern AuthStruct AuthList;
      char *userhost;
      if(!find_user(from, ChanPtr))
      {
         userhost = cluster(from);
         AddChanListAdditionalHost(ChanPtr, UsrPtr, userhost, INSECURE);
         send_to_user(from, "\002Added hostmask %s to your %s account as insecure.\002", userhost, ChanPtr->name);
         SaveDatFile(ChanPtr);
      }

      send_to_user(from, "Access granted. Hello %s.", UsrPtr->handle);
      for(AuthPtr = AuthList;AuthPtr;AuthPtr = AuthPtr->next)
      {
          if(!strcasecmp(AuthPtr->userhost, from))
          {
              Debug(DBGINFO, "Not adding duplicate Auth info");
              return;
          }
      }
      AuthPtr = malloc(sizeof(*AuthPtr));
      if(!AuthPtr)
      {
          Debug(DBGERROR, "Out of memory making authstruct in c_auth()!");
          return;
      }
      strcpy(AuthPtr->userhost, from);
      AuthPtr->timestamp = time(NULL);
      AuthPtr->next = AuthList;
      AuthList = AuthPtr;
   }
}

void c_authlist(char *from, char *channel, char *rest, char *to, CHAN_list *ChanPtr)
{
   AuthStruct AuthPtr;
   extern AuthStruct AuthList;

   if(!AuthList)
   {
       send_to_user(from, "No auth records.");
       return;
   }
   send_to_user(from, "\002--Host-------------------------------------------+----Time-\002");
   for(AuthPtr = AuthList;AuthPtr;AuthPtr = AuthPtr->next)
   {
       send_to_user(from, "\002%50s - %lu", AuthPtr->userhost, AuthPtr->timestamp);
   }
   send_to_user(from, "\002----- End of Auth list -----\002");
}

void c_access(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  char *nuh = from;
  UserStruct *OpPtr;

  if (*rest)
  {
    nuh = strtok(rest, " ");
    nuh = find_person(from, nuh);
  }
  if (!*nuh)
    return;
  if((OpPtr = find_user(nuh, Globals)))
  {
    send_to_user(from, "%s is a global \002%s\002(%d) under the handle \002%s\002",
		 getnick(nuh), GetLevelName(OpPtr->level), OpPtr->level, OpPtr->handle);
    send_to_user(from, "%s %s authenticated.", getnick(nuh), isauth(nuh, Globals)?"is":"is NOT");
    return;
  }
  if (ChanPtr)
  {
    if((OpPtr = find_user(nuh, ChanPtr)))
    {
      send_to_user(from, "%s is a \002%s\002(%d) on channel %s, under the handle \002%s\002",
		   getnick(nuh), GetLevelName(OpPtr->level), OpPtr->level, ChanPtr->name, OpPtr->handle);
      send_to_user(from, "%s %s authenticated.", getnick(nuh), isauth(nuh, ChanPtr)?"is":"is NOT");
    }
    else
    {
      if(isowner(nuh))
        send_to_user(from, "%s matches 'bot owner' (level 100) but has no account.", getnick(nuh));
      send_to_user(from, "\002%s is not in my %s userlist.\002", getnick(nuh), ChanPtr->name);
    }
  }
  else
  {
    send_to_user(from, "\002%s is not a global user.\002", getnick(nuh), ChanPtr->name);
  }
}

#ifdef PYTHON
void c_alias(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  add_alias(from, to, rest);
  return;
}

void c_script(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  list_script(from);
  return;
}
#endif

void c_stats(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  char *nuh = from;

  if (*rest)
  {
    nuh = strtok(rest, " ");
    nuh = find_person(from, nuh);
  }
  if (!*nuh)
    return;

  send_to_user(from, "Stats for %s in %s list", nuh, ChanPtr->name);
  send_to_user(from, "--------------------------");
  send_to_user(from, "      User level: %i", userlevel(nuh, ChanPtr));
  send_to_user(from, " Protected level: %i", protlevel(nuh, ChanPtr));
  send_to_user(from, "      Auto Opped: %s", isaop(nuh, ChanPtr) ? "Yes" : "No");
  send_to_user(from, "Shitlisted level: %i", shitlevel(nuh, ChanPtr));
  return;
}

void c_help(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  FILE *in;
  char lin2[MAXLEN];
  char *cmd1, *cmd2 = lin2;
  char lin[MAXLEN];
  int num = 0, next = 0;
  char thefile[MAXLEN];

  if (!*rest)
  {
    send_to_user(from, "\x2Usage: \"%chelp\" <topic>\x2", cmdchar);
    send_to_user(from, "For a list of topics, do \"%chelp topics\"",
		 cmdchar);
    return;
  }

  sprintf(thefile, "%s/%s", bot_dir, HELPFILE);
  if ((in = fopen(thefile, "r")) == NULL)
  {
    send_to_user(from, "\x2Sorry Sir, %s not found. Try \"TOPICS\" for a list of topics.\x2", helpfile);
    return;
  }
  cmd1 = strtok(rest, " ");
  if (!cmd1)
  {
    send_to_user(from, "\x2Specify a valid command or topic\x2");
    return;
  }
  while (!feof(in))
  {
    if (readln_from_a_file(in, lin))
    {
      if (next)
      {
	sscanf(lin, "%s", cmd2);
	if (!strchr(cmd1, ','))
	  strcat(cmd1, ",");
	if (!strcasecmp(cmd2, cmd1))
	{
	  next = 0;
	  while (!next && !feof(in))
	  {
	    send_to_user(from, "\x2%s\x2", lin);
	    next = !readln_from_a_file(in, lin);
	    if (*lin == ' ')
	      strcpy(lin, strip(lin, ' '));
	    if (!*lin || ((int) (*lin) == 10))
	      next = 1;
	  }
	  num++;
	}
	next = 0;
      }
      if (lin && (*lin == ' '))
	strcpy(lin, strip(lin, ' '));
      if (!*lin || ((int) (*lin) == 10))
	next = 1;
    }
  }
  if (num == 0)
    send_to_user(from, "\x2That command not found...\x2");
  fclose(in);

}

void c_cinfo(char *from, char *channel, char *rest, char *to, CHAN_list *ChanPtr)
{
  int i;

  rest = strtok(rest, " ");
  if(!rest)
  {
       send_to_user(from, "Usage: CINFO <Command>");
       return;
  }
  for (i = 0; on_msg_commands[i].name != NULL; i++)
  { 
     if(!strcasecmp(rest, on_msg_commands[i].name))
     {
          send_to_user(from, "=-Command Info for %s-=", rest);
          send_to_user(from, "   Minimum Access level: %d", on_msg_commands[i].userlevel);
          send_to_user(from, "   Shit level          : %d", on_msg_commands[i].shitlevel);
          send_to_user(from, "   Force DCC           : %s", on_msg_commands[i].forcedcc?"YES":"NO");
          send_to_user(from, "   Channel required    : %s", on_msg_commands[i].needchan?"YES":"NO");
          send_to_user(from, "   Needs command char  : %s", on_msg_commands[i].cmdchar?"YES":"NO");
          send_to_user(from, "=-----------------------------=");
     }
  }
    
}

void c_info(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  send_to_user(from, "\x2( RockBot )  /  Version  %s!\x2", VERSION);
  send_to_user(from, "\x2Written by: rubin@agora.rdrop.com\x2");
  send_to_user(from, "\x2Started: %-20.20s\x2", time2str(uptime));
  send_to_user(from, "\x2Up: %s\x2", idle2str(time(NULL) - uptime));
  return;
}

void c_getch(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  send_to_user(from, "%cCurrent channel: %s\x2", '\x2',
	       currentchannel());
  return;
}

void c_time(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  char buf[MAXLEN];

  strcpy(buf, "%cCurrent time: %s\x2");

  if (ischannel(to))
    sendprivmsg(to, buf, '\x2', time2str(time(NULL)));
  else
    send_to_user(from, buf, '\x2', time2str(time(NULL)));
}

void c_userlist(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  UserStruct *OpPtr;
  HostListStruct *Host;
  int showit;
  char *filter;
  char shorthandle[MAXLEN];

  if (!ChanPtr)
  {
    return;
  }
  if (!ChanPtr->OpList)
  {
    send_to_user(from, "\002Channel %s has no registered users.\002", ChanPtr->name);
    return;
  }
  filter = strtok(rest, " ");
  send_to_user(from, "%s User list", ChanPtr->name);
  send_to_user(from, "=--\002Level\002-------------\002Prot\002-\002Aop\002----\002Handle\002------\002Mask\002---------=");
  for (OpPtr = ChanPtr->OpList; OpPtr; OpPtr = OpPtr->next)
  {
    if(filter)  /* Look for matching handles, or wildcard matching any of the userhostmasks. */
    {
       showit=FALSE;
       if(strcasecmp(OpPtr->handle, filter))  /* If it doesnt match the handle, check for wildcard */
       {
         for(Host=OpPtr->userhosts;Host;Host=Host->next)
         {
            if(!matches(filter, Host->userhost))
               showit = TRUE;
         }
         if(!showit)
           continue;
       }
    }
    strcpy(shorthandle, OpPtr->handle);
    if(strlen(shorthandle)>8)
       shorthandle[7] = '~';
    shorthandle[8] = '\0';
    send_to_user(from, "%3d \002%12s %2d\002 - [%c] \002%8s%c-\017%s%s (%s) %s\017",
                 OpPtr->level,
                 GetLevelName(OpPtr->level),
		 OpPtr->protlevel,
		 (OpPtr->isaop == 1) ? '@' : ' ',
		 shorthandle, /*OpPtr->handle,*/
                 (OpPtr->userhosts->next)?'+':'-',
                 (showit && filter && (!matches(filter, OpPtr->userhosts->userhost)))?"\037":"", /* highlight.. */
		 OpPtr->userhosts->userhost,
                 (OpPtr->userhosts->security)?"I":"S",
		 (strcmp(OpPtr->password, "*")) ? "<pass>" : "");
    for(Host=OpPtr->userhosts->next;Host;Host=Host->next)
    {
       if(showit && filter && !matches(filter, Host->userhost))
         send_to_user(from, "                                  \002%c-\017\037%s\017 (%s)", Host->next?'|':'`', Host->userhost, Host->security?"I":"S");
       else
         send_to_user(from, "                                  \002%c-\017%s (%s)", Host->next?'|':'`', Host->userhost, Host->security?"I":"S");
    }
  }
  send_to_user(from, "=----End of Userlist--+---------(S)=secure-(I)=Insecure--=");
}


void c_shitlist(char *from, char *channel, char *rest, char *to, CHAN_list *ChanPtr)
{
  ShitStruct *BanPtr;

  send_to_user(from, "  Shitlist for channel %s", ChanPtr->name);
  send_to_user(from, "=------------\002Userhost\002--------------\002Lev\002---\002Reason\002-=");
  for(BanPtr=ChanPtr->ShitList;BanPtr;BanPtr=BanPtr->next)
     send_to_user(from, "%35s %d %s", BanPtr->userhost, BanPtr->level, BanPtr->reason);
  send_to_user(from, "=---------------- End of shitlist ----------------=");
}

void c_opme(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  giveop(channel, getnick(from));
}

void c_deopme(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  if (usermode(channel, getnick(from)) & MODE_CHANOP)
    takeop(channel, getnick(from));
}

void c_op(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  if (rest != NULL)
  {
    mode3(channel, "+ooooooooo", rest);
  }
  else
    send_to_user(from, "Who do you want me to op Sir?");
}

void c_deop(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  if (*rest)
  {
    mode3(channel, "-ooooooooo", rest);
  }
  else
    send_to_user(from, "\002Who do you want me to deop Sir?\002");
}


void c_invite(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  char thenick[15];

  strcpy(thenick, getnick(from));
  if (*rest)
  {
    strncpy(thenick, rest, 15);
    if (strchr(thenick, ' ') != NULL)
      *strchr(thenick, ' ') = '\0';
  }
  if (!*getuserhost(thenick))
  {
    send_to_user(from, "\x2Sir, i can not find that user.\x2");
    return;
  }
  if (!invite_to_channel(getnick(thenick), channel))
    send_to_user(from, "\x2I'm not on channel %s, Sir.\x2", channel);
  else
    send_to_user(from, "\x2%s invited to %s\x2", getnick(thenick), channel);
}

void c_open(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  if (!open_channel(channel))
    send_to_user(from, "I could not open %s!", rest);
  else
    channel_unban(channel, from);
}

void c_chat(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  dcc_chat(from, to, "\0");
  sendnotice(getnick(from), "\002Okay Sir, please type: /dcc chat %s\002", current_nick);
}

void c_send(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  char buf[MAXLEN];
  char desc[MAXLEN*2];
  FILE *in;
  int ulevel, level;

  if (!*rest)
  {
    send_to_user(from, "\x2Please specify a filename or use \"%cfiles\" for a list.\x2", cmdchar);
    return;
  }

  ulevel = userlevel(from, Globals);
  if ((in = fopen(filelistfile, "r")) == NULL)
  {
    send_to_user(from, "\x2Problem opening %s\x2", filelistfile);
    return;
  }
  while (!feof(in))
  {
    fscanf(in, "%s %i :%[^\n]\n", buf, &level, desc);
    if (!strcmp(rest, buf))
    {
      if (level > ulevel)
      {
	send_to_user(from, "\x2Sorry, you need access level %i to get this file\x2", level);
	return;
      }
      fclose(in);
      dcc_sendfile(from, rest);
      return;
    }
  }
  send_to_user(from, "\x2That file does not exist...use \"%cfiles\" for a list.\x2", cmdchar);
  fclose(in);
}

void c_flist(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  FILE *ls_file;
  char buffer[255];
  char desc[MAXLEN*2];
  int level;

  if ((ls_file = fopen(filelistfile, "r")) == NULL)
  {
    send_to_user(from, "\x2No files available\x2");
    return;
  }
  send_to_user(from, "\x2 File list: /msg %s %csend <filename> to get files\x2", current_nick, cmdchar);
  send_to_user(from, "\x2-------- Filename --------+- Level Needed -+-Description-\x2");
  while (!feof(ls_file))
  {
    if (fscanf(ls_file, "%s %i :%[^\n]\n", buffer, &level, desc))
      send_to_user(from, "*%25s | %3i   | %s", buffer, level,desc);
  }
  send_to_user(from, "\x2--------------------------+----------------\x2");

  fclose(ls_file);
}

void c_say(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  if (!*rest)
  {
    send_to_user(from, "\x2What do you want me to say, Sir?\x2");
    return;
  }
  sendprivmsg(channel, "%s", rest);
  return;
}

void c_me(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  char text[MAXLEN];

  strcpy(text, "ACTION ");
  if (!*rest)
  {
    send_to_user(from, "\x2What do you want me to ACTION, Sir?\x2");
    return;
  }
  strcat(text, rest);
  send_ctcp(channel, "%s", text);
}

void c_clearq(char *from, char *channel, char *rest, char *to, CHAN_list *ChanPtr)
{
   clear_lowpri_queue();
}

void c_msg(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  char *theperson;
  char *themsg;

  if (!*rest)
  {
    bot_reply(from, 4);
    return;
  }
  if (!strchr(rest, ' '))
  {
    send_to_user(from, "\x2What's the message?\x2");
    return;
  }

  theperson = strtok(rest, " ");
  if (theperson && !strcasecmp(theperson, current_nick))
  {
    bot_reply(from, 6);
    return;
  }
  themsg = strtok(NULL, "\r\n\0");
  sendprivmsg(theperson, "%s", themsg);
  send_to_user(from, "\x2Message sent\x2");
}

void c_do(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  if (*rest)
    send_command(0, "%s", rest);
  else
    send_to_user(from, "What do you want me to send to the server, Sir?");
  return;
}

void c_chanusers(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  show_chanusers(from, channel);
}

void c_channels(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  show_channellist(from);
}

void c_botinfo(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  send_to_user(from, "\002 RockBot Version %s! By Rubin\002", VERSION);
  send_to_user(from, "\002 Created :  June, 1998\002");
  send_to_user(from, "\002 Available at : ftp://agora.rdrop.com/pub/users/rubin\002");
}

char *togstate(int togname)
{
  if (togname == 1)
    return "ENABLED";
  else
    return "DISABLED";
}

void c_join(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  char fbuf[MAXLEN];
  FILE *TheFile;
  char *chan;
  char *key;

  chan = strtok(rest, ", ");
  if (!*chan)
  {
    send_to_user(from, "\002What channel do you want me to Join, Sir?\002");
    return;
  }
  if(chan[0] != '#')
  {
      send_to_user(from, "\002Usage: JOIN <#Channel>\002");
      return;
  }
  key = strtok(NULL, " ");
  if(!mychannel(chan))
  {
    sprintf(fbuf, "%s/%s", bot_dir, chan);
    if((TheFile = fopen(fbuf, "r")))
    {   /* The file exists, lets open it.. */
         fclose(TheFile);
         AddRegChan(chan);
         ChanPtr = search_chan(chan);
         if(!ChanPtr)
         {
              send_to_user(from, "\002Unknown error adding the channel.\002");
              return;
         }
         ReadConfig(ChanPtr);
         if(ChanPtr && key)
             safecopy(ChanPtr->CurrentKey, key, 30);
         send_to_user(from, "\002Using existing data file for %s. Attempting Join...\002", chan);
         join_channel(chan, 0);
         return;
    }
    else
    {
         AddRegChan(chan);
         ChanPtr = search_chan(chan);
         if(ChanPtr && key)
             safecopy(ChanPtr->CurrentKey, key, 30);
         SaveDatFile(ChanPtr);
         SaveDatFile(Globals);
         send_to_user(from, "\002Created data file for %s. Attempting join..", chan);
         join_channel(chan, 0);
     }
  }
  else
  {
    ChanPtr = search_chan(chan);
    sendjoin(chan, ChanPtr->CurrentKey);
    setcurrentchannel(chan);
    send_to_user(from, "Joined %s and marked it active", chan);
  }
}

void c_chanstat(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  int ulcount, slcount, cucount, cbcount;
  int k;
  UserStruct *OpPtr;
  ShitStruct *ShitPtr;
  USER_list  *UPtr;
  BAN_list   *BPtr;
  char ModeString[MAXLEN];

  if(!ChanPtr)
     return;
  ulcount = slcount = cucount = cbcount = 0;
  for(OpPtr=ChanPtr->OpList;OpPtr;OpPtr = OpPtr->next)
    ulcount++;
  for(ShitPtr = ChanPtr->ShitList;ShitPtr;ShitPtr=ShitPtr->next)
     slcount++;
  for(UPtr = ChanPtr->users;UPtr;UPtr = UPtr -> next)
     cucount++;
  for(BPtr = ChanPtr->banned;BPtr;BPtr = BPtr -> next)
     cbcount++;

  send_to_user(from, "=-------------------------------=");
  send_to_user(from, "       Channel Statistics        ");
  send_to_user(from, "=-------------------------------=");
  send_to_user(from, "Channel Name   : \002%s\002", ChanPtr->name);
  if(userlevel(from, ChanPtr) >= BotCoOwner)
   send_to_user(from, "Data file      : \002%s\002", ChanPtr->datfile);
  send_to_user(from, "Default Topic  : %s", ChanPtr->topic);
  GetModeString(ModeString, ChanPtr->PlusModes, ChanPtr->MinModes, ChanPtr->Key, ChanPtr->Limit);
  send_to_user(from, "Enforced Modes : %s", ModeString?ModeString:"");
  send_to_user(from, "\002\002");
  send_to_user(from, "%d in userlist, %d in shitlist", ulcount, slcount);
  send_to_user(from, "%d people currently in the channel", cucount);
  send_to_user(from, "%d bans currently set.", cbcount);
  send_to_user(from, "\002\002");
  for(k=0;set_commands[k].name!=NULL;k++)
  {
    send_to_user(from, "\002%s\002%s", 
        set_commands[k].pre, 
        set_commands[k].settings[ChanPtr->Settings[set_commands[k].index]]);
  }
  send_to_user(from, "=-------------------------------=");
}

void c_leave(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  char *thechan;

  if(rest && *rest)
  {
    thechan = (char *) strtok(rest, " "); 
  }
  else
  {
     thechan = channel;
  }
  if(!thechan)
  {
       send_to_user(from, "\002USAGE: LEAVE <#Channel>\002");
       return;
  }
  if(is_in_list(StealList, thechan))
  {
    send_to_user(from, "\x2Sorry Sir, I'm trying to steal %s. Use RSTEAL to stop first.\x2", thechan);
    return;
  }
  if((ChanPtr = search_chan(thechan)))
  {
     sendpart(thechan);
     send_to_user(from, "I've parted %s and marked it inactive.", thechan);
  }
  else
  {
     /* I'm in a channel thats not registered.. */
     sendpart(thechan);
     send_to_user(from, "I've parted %s", thechan);
  }
}

void c_remchan(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  char chanfile[MAXLEN];
  char *thechan;

  if(rest && *rest)
  {
      rest = strtok(rest, " ");
      if(rest[0] != '#')
      {
         send_to_user(from, "Usage: REMCHAN <CHANNEL>");
         return;
      }
      thechan = rest;
      ChanPtr = search_chan(thechan);
      if(!ChanPtr || ChanPtr == Globals)
      {
        send_to_user(from, "I'm not in %s.", thechan);
        return;
      }
  }
  else
  {
      send_to_user(from, "Usage: REMCHAN <CHANNEL>");
      return;
  }
  if (is_in_list(StealList, thechan))
    send_to_user(from, "\x2Sorry Sir, I'm trying to steal %s\x2", thechan);
  else
  {
    if(ChanPtr)
    {
       strcpy(chanfile, ChanPtr->datfile);
       leave_channel(thechan);
       send_to_user(from, "Channel %s removed from my database.", thechan);
       SaveDatFile(Globals);
       unlink(chanfile);
    }
    else
       send_to_user(from, "Channel %s is not in my database.", thechan);
  }
}

void c_nick(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{

  rest = strtok(rest, " ");
  if (rest && *rest)
  {
    strcpy(targetnick, rest);
    strcpy(lasttriednick, targetnick);
    /* strncpy( current_nick, rest, MAXNICKLEN); */
    ncount = 0;
    sendnick(targetnick);
  }
  else
    bot_reply(from, 8);
  return;
}

void c_lock(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  if (*rest && !ischannel(to))
  {
    if (locked == 0)
    {
      if (strchr(rest, ' ') != NULL)
	*strchr(rest, ' ') = '\0';
      strncpy(lockpass, rest, 8);
      locked = 1;
      send_to_user(from, "\x2Shutting down non-essential systems...\x2");
      send_to_user(from, "\x2RockBot \x2=\x2Locked\x2=");
    }
    else
    {
      send_to_user(from, "\x2Already locked! use UNLOCK.\x2");
      return;
    }
  }
  else
  {
    send_to_user(from, "\x2Syntax: LOCK <password>\x2");
    send_to_user(from, "\x2NOTE: must be in a /msg\x2");
  }
}

void c_unlock(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  if (locked == 1)
  {
    if (rest)
    {
      if (strcmp(rest, lockpass) == 0)
      {
	locked = 0;
	send_to_user(from, "\x2RockBot \x2=\x2UnLocked\x2=");
	return;
      }
      else
      {
	send_to_user(from, "%sAccess denied.\x2", "\x2");
	return;
      }
    }
  }
  else
    send_to_user(from, "\x2I'm not locked, Sir.\x2");
}

void c_die(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  char buf[100];

  botlog(botlogfile, "KILL issued by: %s", from);
  send_to_user(from, "\002Starting termination sequence...\002");
  sprintf(buf, "%s is sending me to bed.. ", getnick(from));
  if (*rest)
    signoff(from, rest);
  else
    signoff(from, buf);
}

/* TODO : Figure out why theres 4 different ways to join a channel, and pick one. Delete
 * the rest. */
void c_newchan(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  char *chan;

  chan = strtok(rest, " ");
  if(chan && chan[0] == '#')
  {
     AddRegChan(chan);
     SaveDatFile(ChanPtr);
     SaveDatFile(Globals);
     send_to_user(from, "\002Channel %s added to my database.\002", chan);
     join_channel(chan, 1);
  }
  else
  {
    send_to_user(from, "\002Invalid syntax.\002");
    send_to_user(from, "\002Usage: CNC <#Channel>\002");
  }
}

void c_setinfo(char *from, char *channel, char *rest, char *to, CHAN_list *ChanPtr)
{
  UserStruct *OpPtr;

  if(!rest)
  {
    send_to_user(from, "\002Syntax: SETINFO <new info line>\002");
    return;
  } 
  OpPtr = find_user(from, ChanPtr);
  if(!OpPtr)
  {
    if((OpPtr = find_user(from, Globals)))
      ChanPtr = Globals;
    
    if(!OpPtr)
    {
      send_to_user(from, "\002I can't find an account for you.\002");
      return;
    }
  }
  strcpy(OpPtr->info, rest);
  send_to_user(from, "\002%s info changed to:\002 %s", ChanPtr->name, OpPtr->info);
  SaveDatFile(ChanPtr);
}

void c_sethandle(char *from, char *channel, char *rest, char *to, CHAN_list *ChanPtr)
{
  char *oldhandle;
  char *newhandle;
  char rplold[MAXLEN];
  UserStruct *OpPtr;

  oldhandle = strtok(rest, " ");
  newhandle = strtok(NULL, " ");

  if(!oldhandle)
  {
     send_to_user(from, "\002Syntax: SETHANDLE [Old handle] <New handle>\002");
     return;
  }
  if(!newhandle)
  {
    newhandle = oldhandle;
    if(!(OpPtr = find_user(from, ChanPtr)))
    {
      if((OpPtr = find_user(from, Globals)))
        ChanPtr = Globals;
      else
      {
        /* Shouldnt be able to get here */
        Debug(DBGERROR, "Cant find user in db for sethandle()");
        return;
      }
    }
  }
  else
  {
    if(!(OpPtr=FindOpByHandle(ChanPtr, oldhandle)))
    {
      if((OpPtr = FindOpByHandle(Globals, oldhandle)))
        ChanPtr = Globals;
      else
      {
         send_to_user(from, "\002Can't find any users with handle %s in channel %s\002", oldhandle, ChanPtr->name);
         return;
      }
    }
  }
  strcpy(rplold, OpPtr->handle);
  strcpy(OpPtr->handle, newhandle);
  send_to_user(from, "\002%s handle for %s changed to %s\002", ChanPtr->name, rplold, newhandle);
  SaveDatFile(ChanPtr);
}

void c_passwd(char *from, char *channel, char *rest, char *to, CHAN_list *ChanPtr)
{
  UserStruct *OpPtr;
  char *oldpass, *newpass;

  if(!(OpPtr = find_user(from, ChanPtr)))
  {
    if((OpPtr=find_user(from, Globals)))
      ChanPtr = Globals;
    else
    {
      Debug(DBGERROR, "Can't find user in passwd()");
      return;
    }
  }
  oldpass = strtok(rest, " ");
  newpass = strtok(NULL, " ");

  if(!oldpass)
  {
    send_to_user(from, "\002Syntax: PASSWD [oldpassword] <newpassword>\002");
    send_to_user(from, "\002If you have a password set you MUST supply it as oldpass. To clear your password set it to '*'.\002");
    return;
  }
  if(!newpass)
  {
    if(!strcmp(OpPtr->password, "*"))
      newpass = oldpass;
    else
    {
       send_to_user(from, "\002Incorrect Password.\002");
       return;
    }
  }
  else
  {
    if(strcmp(OpPtr->password, oldpass))
    {
      send_to_user(from, "\002Incorrect Password.\002");
      return;
    }
  }
  strcpy(OpPtr->password, newpass);
  send_to_user(from, "\002Your password for [\002%s\002] has been changed.\002", ChanPtr->name);
  SaveDatFile(ChanPtr);
}

void c_delhost(char *from, char *channel, char *rest, char *to, CHAN_list *ChanPtr)
{
  char *handle;
  char *userhost;
  UserStruct *OpPtr;
  HostListStruct *HostPtr, *LastHost;
  
  handle = strtok(rest, " ");
  userhost = strtok(NULL, " ");
  if(!handle)
  {
     send_to_user(from, "\002Syntax: DELHOST <handle> <hostmask>\002");
     return;
  }
  if(!userhost)
  {
     OpPtr = find_user(from, ChanPtr);
     if(!OpPtr)
     {
        send_to_user(from, "\002Can't find an account for you in %s userlist.\002", ChanPtr->name);
        return;
     }
     userhost = handle;
     handle = OpPtr->handle;
  }
  else
  {
     OpPtr = FindOpByHandle(ChanPtr, handle);
  }
  if(OpPtr)
  {
    if(!OpPtr->userhosts->next)
    {
       send_to_user(from, "\002Accounts with me must have at least one hostmask.\002");
       return;
    }
    LastHost = OpPtr->userhosts;
    for(HostPtr=OpPtr->userhosts;HostPtr;HostPtr=HostPtr->next)
    {
       if(!strcasecmp(HostPtr->userhost, userhost))
       {
          if(HostPtr == OpPtr->userhosts)
            OpPtr->userhosts = HostPtr->next;
          else
            LastHost->next = HostPtr->next;
          free(HostPtr);
          send_to_user(from, "\002Host %s deleted from %s account in %s userlist.\002",
                         userhost, handle, ChanPtr->name);
          SaveDatFile(ChanPtr);
          return;
       }
       LastHost = HostPtr;
    }
  }
  send_to_user(from, "\002No host %s in account %s in %s userlist.\002", userhost, handle, ChanPtr->name);
}

void c_addhost(char *from, char *channel, char *rest, char *to, CHAN_list *ChanPtr)
{
  char *userhost;
  char *handle;
  char *pass;
  UserStruct *OpPtr;

    handle = strtok(rest, " ");
    pass = strtok(NULL, " ");
    userhost = strtok(NULL, " ");
    if(!handle || !pass)
    {
        send_to_user(from, "\002Syntax: ADDHOST <YourHandle> <YourPassword> [*!*new@*hostmask]\002");
        return;
    }
    if(!userhost)
    {
       userhost = cluster(from);
    }
    OpPtr = FindOpByHandle(ChanPtr, handle);
    if(!OpPtr)
    {
       send_to_user(from, "\002No user with handle %s in channel %s\002", handle, ChanPtr->name);
       return;
    }
    if(!strcmp(OpPtr->password, "*"))
    {
       send_to_user(from, "\002You MUST have a password set to use this command. see HELP PASSWD\002");
       return;
    }
    if(strcmp(OpPtr->password, pass))
    {
       send_to_user(from, "\002Incorrect Password\002");
       return;
    }
    AddChanListAdditionalHost(ChanPtr, OpPtr, userhost, SECURE);
    send_to_user(from, "\002Added %s to your %s account.\002", userhost, ChanPtr->name);
    SaveDatFile(ChanPtr);
}

void c_add(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  char *handle;
  char *hostmask;
  char *buff;
  char strtmp[MAXLEN];
  int level = 0, protlevel = 1, autoop = 1;
  USER_list *UserPtr;
  char *arg1, *arg2;

/* CNU <nick|hostmask handle> <level> [protect] [autoop] */

  arg1 = strtok(rest, " ");
  arg2 = strtok(NULL, " ");
  if(!arg2)
  {
    send_to_user(from, "\002Syntax: CNU <handle> <*!*host@*mask> <level> [protect] [autoop]\002");
    send_to_user(from, "\002    OR: CNU <nick> <level> [protect] [autoop]\002");
    return;
  }
  if(ismask(arg2))  /* If its by handle and hostmask.. */
  {
    handle = arg1;
    hostmask = arg2;
    buff = strtok(NULL, " ");
    level = ConvertLevel(buff);
    if(level < 1 || level > 100)
    {
      send_to_user(from, "\002Incorrect <level> paramiter. Must be a number between 1 and 100 inclusive, Or one of the following: BotOwner, BotCoOwner, ChanOwner, ChanCoOwner, ChanHelper, ChanOp or Peon\002");
      return;
    }
    buff = strtok(NULL, " ");
    if(buff)
    {
      protlevel = ConvertProt(buff);
      if(protlevel < 0 || protlevel > 5)
      {
        send_to_user(from, "\002Incorrect <protect> paramiter. Must be a number between 0 and 5.\002");
        return;
      }
      buff = strtok(NULL, " ");
      if (buff)
      {
        autoop = ConvertAOp(buff);
        if(autoop < 0 || autoop > 1)
        {
          send_to_user(from, "\002Incorrect <autoop> paramiter. Must be either 0 or 1.\002");
          return;
        }
      }
    }
    if (!AddyOk(hostmask))
    {
      send_to_user(from, "\002%s is not a valid hostmask for use in a userlist.\002", hostmask);
      return;
    }
    else
    {
      AddChanListUser(ChanPtr, hostmask, handle, "*", level, protlevel, autoop, SECURE, "*");
      send_to_user(from, "\002User %s Created with handle %s!!\002", hostmask, handle);
      send_to_user(from, "\002Access Level: %i  Auto-Op: %s  Protect Level: %i\002",
                   level, autoop ? "Yes" : "No", protlevel);
      SaveDatFile(ChanPtr);
    }
  }
  else /* Its by nick */
  {
    /* The paramiter is a nick, so use that for getting the info */
    if (!(UserPtr = finduserbynick(arg1)))
    {
      send_to_user(from, "\002I cant find any users nicked %s right now.\002", arg1);
      return;
    }
    sprintf(strtmp, "%s!%s@%s", UserPtr->nick, UserPtr->user, UserPtr->host);
    handle = UserPtr->nick;
    if(isuser(strtmp, ChanPtr))
    {
      send_to_user(from, "\002%s is already a level %d in %s\002", strtmp, userlevel(strtmp, ChanPtr), ChanPtr->name);
      send_to_user(from, "\002The CLVL command can be used to change a users level.\002");
      return;
    }
    else
      /* We made it! lets get them added */
    {
      buff = arg2;	/* get level */
      level = ConvertLevel(buff);
      if(level < 1 || level > 100)
      {
	send_to_user(from, "\002Incorrect <level> paramiter. Must be a number between 1 and 99.\002");
	return;
      }
      buff = strtok(NULL, " ");
      if (buff)
      {
        protlevel = ConvertProt(buff);
	if(protlevel < 0 || protlevel > 5)
	{
	  send_to_user(from, "\002Incorrect <protect> paramiter. Must be a number between 0 and 5.\002");
	  return;
	}
	buff = strtok(NULL, " ");
	if (buff)
	{
          autoop = ConvertAOp(buff);
	  if(autoop < 0 || autoop > 1)
	  {
	    send_to_user(from, "\002Incorrect <autoop> paramiter. Must be either 0 or 1.\002");
	    return;
	  }
	}
      }
      hostmask = cluster(strtmp);	/* cluster calls strtok() so we have to do this last */
      AddChanListUser(ChanPtr, hostmask, handle, "*", level, protlevel, autoop, SECURE, "*");
      send_to_user(from, "\002User %s Created with handle %s!!\002", hostmask, handle);
      send_to_user(from, "\002Access Level: %i  Auto-Op: %s  Protect Level: %i\002",
		   level, autoop ? "Yes" : "No", protlevel);
      SaveDatFile(ChanPtr);
      if (UserPtr)		/* If we were able to obtain their nick lets tell them they are added */
      {
	send_to_user(UserPtr->nick, "\002You have been added to my %s userlist as %s(%s)", ChanPtr->name, handle, hostmask);
	send_to_user(UserPtr->nick, "\002You are a level %d with protection level %d and you are %sbeing auto-opped.",
                     level, protlevel, autoop ? "" : "not "); 
        send_to_user(UserPtr->nick, "\002Please set a password: /msg %s %s pass \002<NewPassword>\002", current_nick, ChanPtr->name);
      }
    }
  }
}


void c_aop(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  char *nuh;

  nuh = strtok(rest, " ");
  if (!nuh || !*nuh)
  {
    bot_reply(from, 4);
    return;
  }
  nuh = find_person(from, nuh);
  if (!*nuh)
    return;

  if (!isuser(nuh, ChanPtr))
  {
    send_to_user(from, "\x2%s is not in my users list\x2", nuh);
    return;
  }
  if (isaop(nuh, ChanPtr))
  {
    send_to_user(from, "\x2%s is already being auto-opped\x2", nuh);
    return;
  }
  if (!change_userlist(nuh, -1, 1, -1, ChanPtr))
  {
    send_to_user(from, "\x2%s is not in my users list\x2", nuh);
    return;
  }
  send_to_user(from, "\x2%s is now being auto-opped\x2", nuh);
  return;
}

void c_naop(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  char *nuh;

  nuh = strtok(rest, " ");
  if (!nuh || !*nuh)
  {
    bot_reply(from, 4);
    return;
  }
  nuh = find_person(from, nuh);
  if (!*nuh)
    return;

  if (!isuser(nuh, ChanPtr))
  {
    send_to_user(from, "\x2%s is not in my users list\x2", nuh);
    return;
  }
  if (!isaop(nuh, ChanPtr))
  {
    send_to_user(from, "\x2%s is already not being auto-opped\x2", nuh);
    return;
  }
  if (!change_userlist(nuh, -1, 0, -1, ChanPtr))
  {
    send_to_user(from, "\x2%s is not in my users list\x2", nuh);
    return;
  }
  send_to_user(from, "\x2%s is now not being auto-opped\x2", nuh);
  return;
}

void c_save(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
   SaveDatFile(ChanPtr);
   write_userhost("blah");
   send_to_user(from, "\002%s data saved.\002", ChanPtr->name);
}

void c_userdel(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  char *handle;
  UserStruct *OpPtr;

  handle = strtok(rest, " ");
  if (!handle || !*handle)
  {
    send_to_user(from, "\002Syantax: REM <handle>\002");
    return;
  }
  if((OpPtr = FindOpByHandle(ChanPtr, handle)))
  {
     if(userlevel(from, ChanPtr) <= OpPtr->level)
     {
       send_to_user(from, "\002You cannot delete users of equal or higher level as yourself.\002");
       return;
     }
     DelChanListUser(ChanPtr, OpPtr);
     send_to_user(from, "\002All hostmasks attached to handle %s deleted from %s userlist.\002",
                                      handle, ChanPtr);
     SaveDatFile(ChanPtr);
  }
  else
  {
     send_to_user(from, "\002I can't find handle %s in the %s userlist.  User NOT deleted.\002",
                                  handle, ChanPtr->name);
     return;
  }
}

void c_screwban(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  char *nuh;
  char *temp;
  int num;

  nuh = strtok(rest, " ");
  if (!nuh || !*nuh)
  {
    bot_reply(from, 4);
    return;
  }
  nuh = find_person(from, nuh);
  if (!*nuh)
    return;

  temp = strtok(NULL, "\n");
  if (temp)
    num = atoi(temp);
  else
    num = 1;

  if ((protlevel(nuh, ChanPtr) || (userlevel(nuh, ChanPtr) >= PROTLEVEL)) &&
      (userlevel(from, ChanPtr) != OWNERLEVEL))
  {
    send_to_user(from, "\x2Sorry, that user is protected\x2");
    return;
  }

  screw_ban(channel, nuh, num);
  send_to_user(from, "\x2%s screwbanned on %s\x2", nuh, channel);
}

void c_ban(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  char *nuh;

  nuh = strtok(rest, " ");
  if (!nuh || !*nuh)
  {
    bot_reply(from, 4);
    return;
  }
  nuh = find_person(from, nuh);
  if (!*nuh)
    return;

  if ((protlevel(nuh, ChanPtr) || (userlevel(nuh, ChanPtr) >= PROTLEVEL)) &&
      (userlevel(from, ChanPtr) != OWNERLEVEL))
  {
    send_to_user(from, "\x2Sorry, that user is protected\x2");
    return;
  }

  ban_user(channel, nuh);
  send_to_user(from, "\x2%s banned on %s\x2", nuh, channel);
}

void c_siteban(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  char *nuh;

  nuh = strtok(rest, " ");
  if (!nuh || !*nuh)
  {
    bot_reply(from, 4);
    return;
  }
  nuh = find_person(from, nuh);
  if (!*nuh)
    return;

  if ((protlevel(nuh, ChanPtr) || (userlevel(nuh, ChanPtr) >= PROTLEVEL)) &&
      (userlevel(from, ChanPtr) != OWNERLEVEL))
  {
    send_to_user(from, "\x2Sorry, that user is protected\x2");
    return;
  }

  site_ban(channel, nuh);
  send_to_user(from, "\x2%s site-banned on %s\x2", nuh, channel);
}

void c_sitekb(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  char *nuh, *msg;

  nuh = strtok(rest, " ");
  if (!nuh || !*nuh)
  {
    bot_reply(from, 4);
    return;
  }
  nuh = find_person(from, nuh);
  if (!*nuh)
    return;

  if ((protlevel(nuh, ChanPtr) || (userlevel(nuh, ChanPtr) >= PROTLEVEL)) &&
      (userlevel(from, ChanPtr) != OWNERLEVEL))
  {
    send_to_user(from, "\x2Sorry, that user is protected\x2");
    return;
  }
  msg = strtok(NULL, "\n\0");
  if (!msg)
    msg = "";
  site_ban(channel, nuh);
  sendkick(channel, getnick(nuh), msg);
  send_to_user(from, "\x2%s site-kickbanned on %s\x2", nuh, channel);
}

void c_fuckban(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  char *nuh, *msg;

  nuh = strtok(rest, " ");
  if (!nuh || !*nuh)
  {
    bot_reply(from, 4);
    return;
  }
  nuh = find_person(from, nuh);
  if (!*nuh)
    return;

  if ((protlevel(nuh, ChanPtr) || (userlevel(nuh, ChanPtr) >= PROTLEVEL)) &&
      (userlevel(from, ChanPtr) != OWNERLEVEL))
  {
    send_to_user(from, "\x2Sorry, that user is protected\x2");
    return;
  }
  msg = strtok(NULL, "\n\0");
  if (!msg)
    msg = "";
  ban_user(channel, nuh);
  screw_ban(channel, nuh, 1);
  sendkick(channel, getnick(nuh), msg);
}

void c_kickban(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  char *nuh, *msg;

  nuh = strtok(rest, " ");
  if (!nuh || !*nuh)
  {
    bot_reply(from, 4);
    return;
  }
  nuh = find_person(from, nuh);
  if (!*nuh)
    return;

  if ((protlevel(nuh, ChanPtr) || (userlevel(nuh, ChanPtr) >= PROTLEVEL)) &&
      (userlevel(from, ChanPtr) != OWNERLEVEL))
  {
    send_to_user(from, "\x2Sorry, that user is protected\x2");
    return;
  }
  msg = strtok(NULL, "\n\0");
  if (!msg)
    msg = "";
  ban_user(channel, nuh);
  sendkick(channel, getnick(nuh), msg);
}

void c_kick(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  char *nuh, *msg;

  nuh = strtok(rest, " ");
  if (!nuh || !*nuh)
  {
    bot_reply(from, 4);
    return;
  }
  nuh = find_person(from, nuh);
  if (!*nuh)
    return;

  if ((protlevel(nuh, ChanPtr) || (userlevel(nuh, ChanPtr) >= PROTLEVEL)) &&
      (userlevel(from, ChanPtr) != OWNERLEVEL))
  {
    send_to_user(from, "\x2Sorry, that user is protected\x2");
    return;
  }
  msg = strtok(NULL, "\n\0");
  if (!msg)
    msg = "";
  sendkick(channel, getnick(nuh), msg);
}

/* Shows the list of people in the channel spesified */
void c_who(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  show_users_on_channel(from, channel);
}

void c_massop(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  char pattern[100];
  char *op_pattern;

  if (*rest)
  {
    op_pattern = strtok(rest, " ");
    if (op_pattern)
      strcpy(pattern, op_pattern);
    else
      strcpy(pattern, "*!*@*");
  }
  else
    strcpy(pattern, "*!*@*");
  channel_massop(channel, pattern);
  return;
}

void c_massdeopnu(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  channel_massdeop(channel, "");
  return;
}

void c_masskicknu(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  channel_masskick(channel, "");
  return;
}

void c_massopu(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  channel_massop(channel, "");
  return;
}

void c_massdeop(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  char pattern[100];
  char *op_pattern;

  if (*rest)
  {
    op_pattern = strtok(rest, " ");
    if (op_pattern)
      strcpy(pattern, op_pattern);
    else
      strcpy(pattern, "*!*@*");
  }
  else
    strcpy(pattern, "*!*@*");
  channel_massdeop(channel, pattern);
  return;
}

void c_masskick(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  char pattern[100];
  char *op_pattern;

  if (*rest)
  {
    op_pattern = strtok(rest, " ");
    if (op_pattern)
      strcpy(pattern, op_pattern);
    else
      strcpy(pattern, "*!*@*");
  }
  else
    strcpy(pattern, "*!*@*");
  channel_masskick(channel, pattern);
  return;
}

void c_masskickban(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  char pattern[100];
  char *op_pattern;

  if (*rest)
  {
    op_pattern = strtok(rest, " ");
    if (op_pattern)
      strcpy(pattern, op_pattern);
    else
      strcpy(pattern, "*!*@*");
  }
  else
    strcpy(pattern, "*!*@*");
  channel_masskickban(channel, pattern);
  return;
}

void c_shit(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  char *who, *levelstr, *reason, *buf;
  char reasonbuff[MAXLEN];
  int level=0;
  USER_list *UserPtr;
  char hostbuf[MAXLEN];

  who = strtok(rest, " ");
  levelstr = strtok(NULL, " ");
  reason = strtok(NULL, "\0");

  if(!who)
  {
     send_to_user(from, "\002Syntax: SHIT <nick/mask> <level 1-5> [reason]\002");
     return;
  }
  if(!levelstr || (level = atoi(levelstr)) > 5 || (level < 1))
  {  
     send_to_user(from, "\002Incorrect level paramiter.\002");
     send_to_user(from, "\002Syntax: SHIT <nick/mask> <level 1-5> [reason]\002");
     return;
  }  
  if(!reason)
  {
     reason = reasonbuff;
     sprintf(reason, "Banned by %s", gethandle(from, ChanPtr));
  }
  else
  {
     sprintf(reasonbuff, "<%s> %s", (buf=gethandle(from, ChanPtr))?buf:"UNKNOWN", reason);
     reason = reasonbuff;
  }
  if(!ismask(who))
  {
    if (!(UserPtr = finduserbynick(who)))
    {
      send_to_user(from, "\002I cant find any users with the nick %s right now.\002", who);
      return;
    }
    sprintf(hostbuf, "%s!%s@%s", UserPtr->nick, UserPtr->user, UserPtr->host);
    who = cluster(hostbuf);
  }
  if(shitlevel(who, ChanPtr))
  {
     send_to_user(from, "\002A ban already exists for %s.\002", who);
     return;
  }
  if(userlevel(from, ChanPtr) <= userlevel(who, ChanPtr))
  {
     send_to_user(from, "\002Sorry Sir, %s has access higher than or equal to yours.\002");
     return;
  }
  AddChanListBan(ChanPtr, who, level, from, time(NULL), time(NULL)+3600, reason);
  send_to_user(from, "\002%s is now shitlisted in %s, level %d", who, ChanPtr->name, level);
  SaveDatFile(ChanPtr);
}

void c_rshit(char *from, char *channel, char *rest, char *to, CHAN_list *ChanPtr)
{
  ShitStruct *BanPtr, *PrevBPtr;
  char *host, hostbuf[MAXLEN];
  USER_list *UserPtr;

  host = strtok(rest, " ");

  if(!host)
  {
     send_to_user(from, "\002Syntax: RSHIT <nick/host/mask>\002");
     return;
  }
  if(!ismask(host))
  {
    if (!(UserPtr = finduserbynick(host)))
    {
      send_to_user(from, "\002I cant find any users with the nick %s right now.\002", host);
      return;
    }
    sprintf(hostbuf, "%s!%s@%s", UserPtr->nick, UserPtr->user, UserPtr->host);
    host = hostbuf;
     
  }
  PrevBPtr = NULL;
  for(BanPtr=ChanPtr->ShitList;BanPtr;BanPtr=BanPtr->next)
  {
     if(!strcasecmp(BanPtr->userhost, host))
       break;
     if(!matches(BanPtr->userhost, host) || !matches(host, BanPtr->userhost))
       break;
     PrevBPtr = BanPtr;
  }
  if(!BanPtr)
  {
     send_to_user(from, "\002I dont see a ban for %s in the %s shitlist.", host, ChanPtr->name);
     return;
  }
  send_to_user(from, "\002Deleted %s from shitlist.\002", BanPtr->userhost);
  if(!PrevBPtr)
  {
     free(BanPtr);
     ChanPtr->ShitList = NULL;
  }
  else
  {
     PrevBPtr->next = BanPtr->next;
     free(BanPtr);
  }
  SaveDatFile(ChanPtr);
}

void c_prot(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  char *newlevel, *nuh;
  int level = 0;

  nuh = strtok(rest, " ");
  if (!nuh || !*nuh)
  {
    send_to_user(from, "\002Syntax: PROT <Userhost/Nick> <level>\002");
    return;
  }
  nuh = find_person(from, nuh);
  if (!*nuh)
    return;

  if (!isuser(nuh, ChanPtr))
  {
    send_to_user(from, "\x2%s is not in my users list\x2", nuh);
    return;
  }

  if (!(newlevel = strtok(NULL, " ")))
  {
    bot_reply(from, 9);
    return;
  }

  level = protlevel(nuh, ChanPtr);

  if ((atoi(newlevel) <= 0) || (atoi(newlevel)) > MAXPROTLEVEL)
  {
    send_to_user(from, "\x2Valid protect levels are 1 through %i!!!\x2",
		 MAXPROTLEVEL);
    send_to_user(from, "\x2To unprotect, use \"%crprot\"\x2",
		 cmdchar);
    return;
  }

  if (!change_userlist(nuh, -1, -1, atoi(newlevel), ChanPtr))
  {
    send_to_user(from, "\x2%s is not in my users list\x2", nuh);
    return;
  }
  if (level)
    send_to_user(from, "\x2Protect level changed from %i to %i\x2",
		 level, atoi(newlevel));
  else
    send_to_user(from, "\x2User now protected with level %i\x2",
		 atoi(newlevel));
  return;
}

void c_nprot(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  char *nuh;
  int level = 0;

  nuh = strtok(rest, " ");
  if (!nuh || !*nuh)
  {
    bot_reply(from, 4);
    return;
  }
  nuh = find_person(from, nuh);
  if (!*nuh)
    return;

  if (!isuser(nuh, ChanPtr))
  {
    send_to_user(from, "\x2%s is not in my users list\x2", nuh);
    return;
  }
  level = protlevel(nuh, ChanPtr);
  if (!level)
  {
    send_to_user(from, "\x2User is already not being protected\x2");
    return;
  }
  if (!change_userlist(nuh, -1, -1, 0, ChanPtr))
  {
    send_to_user(from, "\x2%s is not in my users list\x2", nuh);
    return;
  }
  send_to_user(from, "\x2User is now not being protected\x2");
  return;
}

void c_setsecurity(char *from, char *channel, char *rest, char *to, CHAN_list *ChanPtr)
{
   HostListStruct *HostPtr;
   UserStruct *UsrPtr, *FromPtr;
  
   char *hostmask;
   char *level;

   hostmask = strtok(rest, " ");
   level = strtok(NULL, " ");

   if(!hostmask)
   {
      send_to_user(from, "No level spesified.");
      send_to_user(from, "\002Usage: SETSECURITY [hostmask] <SECURE/INSECURE>\002");
      return;
   }
   if(!level)
   {
      level = hostmask;
      hostmask = NULL;
   }
   if((strcasecmp(level, "SECURE")) && (strcasecmp(level, "INSECURE")))
   {
      send_to_user(from, "Invalid Level spesified.");
      send_to_user(from, "\002Usage: SETSECURITY [hostmask] <SECURE/INSECURE>\002");
      return;
   }
   if(hostmask)
   {
      UsrPtr = find_user(hostmask, ChanPtr);
      if(!UsrPtr)
      {
         send_to_user(from, "No match for %s", hostmask);
         return;
      }
      FromPtr = find_user(from, ChanPtr);
      if(!FromPtr)
         FromPtr = find_user(from, ChanPtr);
      if(!FromPtr)
      {
          send_to_user(from, "\002Error cannot find your account.\002");
          return;
      }
      if(UsrPtr !=  FromPtr)
      {
        if(UsrPtr->level >= FromPtr->level)
        {
            send_to_user(from, "Sir, you cannot modify a user of greater or equal standing.");
            return;
        }
      }
   }
   else
   {
      UsrPtr = find_user(from, ChanPtr);
      if(!UsrPtr)
      {
         send_to_user(from, "Error cant find you in database!");
         return;
      }
   }
   for(HostPtr = UsrPtr->userhosts;HostPtr;HostPtr = HostPtr->next)
   {
       if(!matches(HostPtr->userhost, hostmask))
       {
          if(!strcasecmp(level, "SECURE"))
            HostPtr->security = 0;
          else
            HostPtr->security = 1;
          send_to_user(from, "%s security setting for %s set to %s", ChanPtr->name, HostPtr->userhost, HostPtr->security?"INSECURE":"SECURE");
          SaveDatFile(ChanPtr);
          return;
       }
   }
}

void c_clvl(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  char *newlevel;
  UserStruct *UsrPtr;
  int level = 0;

  rest = strtok(rest, " ");
  if(!*rest)
  {
      send_to_user(from, "\002Usage: CLVL <Handle> <Newlevel>\002");
      return;
  }
  UsrPtr = FindOpByHandle(ChanPtr, rest);
  if(!UsrPtr)
  {
      send_to_user(from, "\002No account for %s found.\002", rest);
      send_to_user(from, "\002Usage: CLVL <handle> <Newlevel>\002");
      return;
  }
  if (UsrPtr->level >= userlevel(from, ChanPtr))
  {
    send_to_user(from, "\002You cannot modify users of equal or greater access, sir.\002");
    return;
  }

  if (!(newlevel = strtok(NULL, " ")))
  {
    bot_reply(from, 9);
    return;
  }
  level = ConvertLevel(newlevel);
  if(level < 1 || level > 100)
  {
      send_to_user(from, "\002Incorrect <level> paramiter. Must be a number between 1 and 100.\002");
      return;
  }
  if(level >= userlevel(from, ChanPtr))
  {
    send_to_user(from, "\002Sorry, can't change to higher or equal to your level\002");
    return;
  }
  if ((level <= 0) || (level > 100))
  {
    if (!level)
      send_to_user(from, "\002Please use the \"REM\" command to delete a user\002");
    else
      send_to_user(from, "\002Valid levels are from 1 to 100\002");
    return;
  }
  UsrPtr->level = level;
  SaveDatFile(ChanPtr);
  send_to_user(from, "\002Access level for %s now changed to %i\002", rest, level);
  return;
}

void c_addks(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  char ks[MAXLEN];

  if (!*rest)
  {
    send_to_user(from, "\x2Specify a string of text to kick upon when said\x2");
    return;
  }
  strcpy(ks, "*");
  strcat(ks, rest);
  strcat(ks, "*");
  if (is_in_list(KickSays, ks))
  {
    send_to_user(from, "%cAlready kicking on the say of that\x2",
		 '\x2');
    if (KickSayChan == NULL)
      send_to_user(from, "\x2WARNING: No channels selected for auto-kick\x2");
    return;
  }
  if (add_to_list(&KickSays, ks))
  {
    send_to_user(from, "\x2Now kicking on the say of %s\x2", ks);
    if (KickSayChan == NULL)
      send_to_user(from, "\x2WARNING: No channels selected for auto-kick\x2");
  }
  else
    send_to_user(from, "\x2Problem adding the kicksay\x2");
  return;
}

void c_addksc(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  if (!*rest || !ischannel(rest))
  {
    bot_reply(from, 10);
    return;
  }
  if (!mychannel(rest) && (*rest != '*'))
  {
    bot_reply(from, 11);
    return;
  }
  if (is_in_list(KickSayChan, rest))
  {
    send_to_user(from, "%cAlready auto-kicking on that channel\x2",
		 '\x2');
    return;
  }
  if (add_to_list(&KickSayChan, rest))
    send_to_user(from, "\x2Now auto-kicking on %s\x2", rest);
  else
    send_to_user(from, "\x2Problem adding the channel\x2");
  return;
}

void c_clrks(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  if (KickSays == NULL)
  {
    send_to_user(from, "\x2There are no kicksays\x2");
    return;
  }
  remove_all_from_list(&KickSays);
  send_to_user(from, "%cAll kicksays have been removed\x2", '\x2');
  return;
}

void c_clrksc(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  if (KickSayChan == NULL)
  {
    send_to_user(from, "\x2There are no kicksay channels\x2");
    return;
  }
  remove_all_from_list(&KickSayChan);
  send_to_user(from, "%cAll channels have been removed\x2", '\x2');
  return;
}

void c_delks(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  char ks[MAXLEN];

  if (!*rest)
  {
    send_to_user(from, "\x2Specify the string of text to delete\x2");
    return;
  }
  strcpy(ks, rest);
  if (!is_in_list(KickSays, ks))
  {
    send_to_user(from, "\x2I'm not kicking on the say of that\x2");
    return;
  }
  if (remove_from_list(&KickSays, ks))
    send_to_user(from, "\x2The kicksay has been deleted\x2");
  else
    send_to_user(from, "\x2Problem deleting the kicksay\x2");
  return;
}

void c_delksc(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  char ks[MAXLEN];

  if (!*rest)
  {
    bot_reply(from, 10);
    return;
  }
  strcpy(ks, rest);
  if (!is_in_list(KickSayChan, ks))
  {
    send_to_user(from, "\x2I'm not auto-kicking on that channel\x2");
    return;
  }
  if (remove_from_list(&KickSayChan, ks))
    send_to_user(from, "\x2The channel has been deleted\x2");
  else
    send_to_user(from, "\x2Problem deleting the channel\x2");
  return;
}

void c_listks(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  ListStruct *temp;
  int i = 0;

  if (KickSays == NULL)
  {
    send_to_user(from, "\x2There are currently no kicksays\x2");
    return;
  }
  send_to_user(from, "--- List of Kicksays: ---");
  for (temp = KickSays; temp; temp = temp->next)
  {
    i++;
    send_to_user(from, "\x2%2i -- %s\x2", i, temp->name);
  }
  send_to_user(from, "--- End of list ---");
  return;
}

void c_listksc(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  ListStruct *temp;
  int i = 0;

  if (KickSayChan == NULL)
  {
    send_to_user(from, "\x2There are currently no kicksay channels\x2");
    return;
  }
  send_to_user(from, "--- List of Kicksay channels: ---");
  for (temp = KickSayChan; temp; temp = temp->next)
  {
    i++;
    send_to_user(from, "\x2%2i -- %s\x2", i, temp->name);
  }
  send_to_user(from, "--- End of list ---");
  return;
}

void c_massunban(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  channel_massunban(channel);
}

void c_unban(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  char *nuh;

  nuh = strtok(rest, " ");
  if (!nuh || !*nuh)
  {
    bot_reply(from, 4);
    return;
  }
  nuh = find_person(from, nuh);
  if (!*nuh)
    return;
  channel_unban(channel, nuh);
  send_to_user(from, "\x2%s unbanned on %s\x2", nuh, channel);
}

void c_unbanme(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  channel_unban(channel, from);
  send_to_user(from, "\x2You are unbanned on %s\x2", channel);
}

void c_listdcc(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  show_dcclist(from);
}

#ifdef NOTDEF
void c_spawn(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  char buf[MAXLEN];
  char *nick, *args;
  FILE *out;
  int i;

#ifdef WIN95
  send_to_user(from, "\x2Spawn not supported under non-unix OS\x2");
  return;
#endif
  return;			/* not yet re-supported */
  if (!*rest)
  {
    send_to_user(from, "\x2Usage: Spawn Nick Arguments\x2");
    return;
  }
  nick = strtok(rest, " ");
  args = strtok(NULL, "\n");
  if (!args)
    args = "";
  sprintf(buf, "%s%s.servers", bot_dir, nick);
  if (!exist(buf) && ((out = fopen(buf, "w")) != NULL))
  {
    for (i = 0; *(bot.servers[i].servername); i++)
      fprintf(out, "%s\n", bot.servers[i].servername);
    fclose(out);
  }
  sprintf(buf, "%s%s.userlist", bot_dir, nick);
  if (!exist(buf))
    write_userlist(buf);
  sprintf(buf, "%s%s.shitlist", bot_dir, nick);
  if (!exist(buf))
    write_shitlist(buf);
  sprintf(buf, "%s%s.userhost", bot_dir, nick);
  if (!exist(buf))
    write_userhost(buf);

  sprintf(buf, "-e %s -n %s ", settingsfile, nick);
  strcat(buf, args);
  make_process(progname, buf);
}
#endif

void c_topic(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  if (!*rest)
  {
    send_to_user(from, "\x2What's the topic???\x2");
    return;
  }
  send_command(0, "TOPIC %s :%s", channel, rest);
  send_to_user(from, "\x2Topic changed on %s\x2", channel);
  return;
}

void c_test(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
    char *pri;
    int priority = 0;

    if(*rest)
    {
        pri = strtok(rest, " ");
        rest = strtok(NULL, "\0");
        if(!(priority = atoi(pri)))
        {
           send_to_user(from, "incorrect priority");
           return;
        }

        send_command(priority, "PRIVMSG %s :TEST: %s", getnick(from), rest);
    }
    else
    {
        QueueListType qptr;
        for(qptr = Queue;qptr;qptr = qptr->Next)
        {
           Debug(DBGNOTICE, "%d - %s", qptr->priority, qptr->buff);
        }
        Debug(DBGNOTICE, "--end of list--");
    }
}

void c_test1(char *from, char *channel, char *rest, char *to, CHAN_list *ChanPtr)
{
   char *ban1, *ban2;

   ban1 = (char *) strtok(rest, " ");
   ban2 = (char *) strtok(rest, " ");

   if(!ban2)
   {
        send_to_user(from, "USAGE: test1 ban1 ban2");
        return;
   }
   if(bancmp(ban1, ban2))
   {
       send_to_user(from, "%s does not match %s.", ban1, ban2);
   }
   else
   {
       send_to_user(from, "%s matches %s.", ban1, ban2);
   }

}

#ifdef PYTHON
void c_python(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  PyObject *Temp;

  strcpy(PYNICK, from);
  PyRun_SimpleString(rest);
}

void c_pyscript(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  FILE *TheFile;
  char *file;
  char Str[MAXLEN];

  file = strtok(rest, " ");
  rest = strtok(NULL, "\n");
  if (TheFile = fopen(file, "r"))
  {
    sprintf(Str, "rest = \"%s\"", rest);
    PyRun_SimpleString(Str);
    sprintf(Str, "nick = \"%s\"", from);
    PyRun_SimpleString(Str);
    sprintf(Str, "to = \"%s\"", to);
    PyRun_SimpleString(Str);

    PyRun_SimpleFile(TheFile, "file");
  }
  fclose(TheFile);
}
#endif

void c_spymsg(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  if (is_in_list(SpyMsgList, getnick(from)))
  {
    send_to_user(from, "%cAlready redirecting messages to you\x2",
		 '\x2');
    return;
  }
  if (add_to_list(&SpyMsgList, getnick(from)))
    send_to_user(from, "\x2Now redirecting messages to you\x2");
  else
    send_to_user(from, "%cError: Can't redirect you the messages\x2",
		 '\x2');
  return;
}

void c_nospymsg(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  if (!is_in_list(SpyMsgList, getnick(from)))
  {
    send_to_user(from, "%cI'm not redirecting messages to you\x2",
		 '\x2');
    return;
  }
  if (remove_from_list(&SpyMsgList, getnick(from)))
    send_to_user(from, "\x2No longer redirecting messages to you\x2");
  else
    send_to_user(from, "%cError: Can't stop redirecting you the messages\x2",
		 '\x2');
  return;
}

void c_cmdchar(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  char temp;

  if (!*rest)
  {
    send_to_user(from, "\x2What's the new command character?\x2");
    return;
  }
  temp = *rest;
  if (isalnum(temp))
    send_to_user(from, "\x2Sorry, that command character is not allowed\x2");
  else
  {
    cmdchar = temp;
    send_to_user(from, "\x2My command character is now \"%c\"\x2",
		 cmdchar);
  }
  return;
}

void c_holdnick(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  char *themsg;

  if (holdnick)
  {
    if (*rest)
    {
      themsg = strtok(rest, "\n");
      if (themsg)
      {
	strcpy(nhmsg, themsg);
      }
      if (holdingnick)
	send_command(900, "AWAY :%s", nhmsg);
      send_to_user(from, "\x2Now sending new nickhold message\x2");
      return;
    }
    send_to_user(from, "\x2Already holding %s's nick\x2", ownernick);
    return;
  }
  else if (*rest)
  {
    themsg = strtok(rest, "\n");
    if (themsg)
    {
      strcpy(nhmsg, themsg);
    }
  }
  else
    strcpy(nhmsg, "");
  holdnick = 1;
  strcpy(targetnick, ownernick);
  send_command(800, "ISON %s", ownernick);
  strcpy(nohold_nick, current_nick);
  send_to_user(from, "\x2Now holding %s's nick\x2", ownernick);
  return;
}

void c_noholdnick(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  if (!holdnick)
  {
    send_to_user(from, "\x2I'm not holding %s's nick\x2", ownernick);
    return;
  }
  holdnick = 0;
  holdingnick = 0;
  send_command(0, "AWAY");
  c_nick(from, channel, nohold_nick, to, ChanPtr);
  send_to_user(from, "\x2No longer holding %s's nick\x2", ownernick);
  return;
}

void c_seen(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  char buf[MAXLEN]="", tmp[200]="";
  time_t difference;
  UserHostStruct *temp = UserHostList;
  int num;
  char *target;
  int days, hours, minutes, seconds;

  if (!*rest && !pubseen)
  {
    bot_reply(from, 4);
    return;
  }

  rest = strtok(rest, " ");
  if (*(rest + strlen(rest) - 1) == '?')
    *(rest + strlen(rest) - 1) = '\0';
  num = 0;

  if(ChanPtr && (ChanPtr != Globals) && ChanPtr->Settings[PUBSEEN])
  {
     target = ChanPtr->name;
  }
  else
  {
     target = getnick(from);
  }
  Debug(DBGINFO, "From is %s, rest is %s", from, rest);
  if(!strcasecmp(rest, getnick(from)))
  {
     sendprivmsg(target, "\002%s\002, i'm telling you, your NOT invisible! I really think you should talk to someone.", getnick(from));
     return;
  }
  while(temp)
  {
    temp = find_userhostfromnick(temp, rest);
    if(temp)
    {
      num++;

      difference = time(NULL) - temp->time;
      days = difference / 86400;
      difference = difference - (days * 86400);
      hours = difference / 3600;
      difference = difference - (hours * 3600);
      minutes = difference / 60;
      difference = difference - (minutes * 60);
      seconds = difference;

      if(days == 1)
       strcat(buf, "1 day ");
      else if(days > 1)
      {
        sprintf(tmp, "%d days ", days);
        strcat(buf, tmp);
      }
      if(hours == 1)
        strcat(buf, "1 hour ");
      else if(hours > 1)
      {
        sprintf(tmp, "%d hours ", hours);
        strcat(buf, tmp);
      }
      if (!days) /* why show minutes if it's been over a day? */
      {
        if(minutes == 1)
          strcat(buf, "1 minute ");
        else if(minutes > 1)
        {
            sprintf(tmp, "%d minutes ", minutes);
            strcat(buf, tmp);
        }
        if(!hours) /* no reason to display seconds if it has been over an hour */
        {
          if(seconds == 1)
           strcat(buf, "1 second ");
          else if(seconds > 1)
          {
              sprintf(tmp, "%d seconds ", seconds);
              strcat(buf, tmp);
          }
        }
      }

/*
      strcpy(buf, ctime(&temp->time));
      if(strlen(buf))
        *(buf + strlen(buf) - 1) = '\0';
*/

      if(ChanPtr && !pubseen) /* a real command */
      {
        if(num == 1)
          send_to_user(from, "=-------\002UserHost\002----------------------------\002Channel\002------\002Time\002--------=");
        send_to_user(from, "\002%40s\002: %10s \002%s\002ago.", temp->name, temp->channel, temp->time ? buf : "<UNKNOWN>");
      } 
      else  /* Public overheard command*/
      {
        if((temp->time) && (strcasecmp(temp->channel, "<UNKNOWN>")))
        {
          if(num == 1)
          {
            sendprivmsg(target, "\002%s:\002 I've seen %s", getnick(from), temp->name);
            sendprivmsg(target, "I last saw him or her on %s, \002%s\002ago.", temp->channel, buf);
          }
          else if(num == 2)
          {
            sendprivmsg(target, "\002%s\002 There are more userhosts for that nick...To see the whole list do: %cseen <nick>", getnick(from), cmdchar);
            return; /* Gee, if were done lets like uh.. maybe stop looping?? ya think?? */
          }
        }
      }

      temp = temp->next;
    }
  }
  if(num)
  {
    if(!pubseen)
      send_to_user(from, "=--------------------------------------------------------------------=");
    return;
  }
  if(!pubseen)
    send_to_user(from, "\x2No userhost information found\x2");
  else
    sendprivmsg(target, "\x2%s:\x2 I haven't seen %s", getnick(from), rest);
  return;
}

void c_resetuhost(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  remove_all_from_userhostlist();
  send_to_user(from, "\x2Starting fresh userhost list\x2");
  return;
}

void c_getuserhost(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  char nuh[255];
  char nuh2[255];
  int num = 0;

  UserHostStruct *temp = UserHostList;

  if (!*rest)
  {
    bot_reply(from, 4);
    return;
  }
  rest = strtok(rest, " ");

  num = 0;
  while (temp)
  {
    temp = find_userhostfromnick(temp, rest);
    if (temp)
    {
      num++;
      send_to_user(from, "Possible userhost: %s\x2", temp->name);
      temp = temp->next;
    }
  }
  if (num)
    return;
  strcpy(nuh, rest);
  strcat(nuh, "!");
  strcpy(nuh2, getuserhost(rest));
  if (!*nuh2)
  {
    send_to_user(from, "\x2There is no information for that nickname\x2");
    return;
  }
  strcat(nuh, nuh2);
  send_to_user(from, "\x2%s is %s\x2", rest, nuh);
  return;
}

void c_totstats(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  char nuh[255];
  char nuh2[255];
  UserHostStruct *user;

  if (!*rest)
  {
     rest = from;
  }
  if (strchr(rest, ' '))
    *strchr(rest, ' ') = '\0';
  strcpy(nuh, getuserhost_fromlist(rest));
  strcpy(nuh2, rest);
  strcat(nuh2, "!");
  strcat(nuh2, nuh);
  user = find_userhost(nuh2);
  if (!*nuh || !user)
  {
    send_to_user(from, "\x2There is no information for that nickname\x2");
    send_to_user(from, "\x2or there is more than one user@host for that nick\x2");
  }
  else
  {
    send_to_user(from, "\x2Total Stats for %s:\x2", nuh2);
    send_to_user(from, "\x2------------------------------\x2");
    send_to_user(from, "Number of ops given:     %5i", user->totop);
    send_to_user(from, "Number of deops done:    %5i", user->totdeop);
    send_to_user(from, "Number of bans done:     %5i", user->totban);
    strcpy(nuh, ctime(&user->kicktime));
    if (strchr(nuh, '\n'))
      *strchr(nuh, '\n') = '\0';
    if (!user->totkick)
      strcpy(nuh, "NEVER");
    send_to_user(from, "Number of people kicked: %5i", user->totkick);
    send_to_user(from, "     last one at approx:     %s", nuh);
    strcpy(nuh, ctime(&user->killtime));
    if (strchr(nuh, '\n'))
      *strchr(nuh, '\n') = '\0';
    if (!user->totkill)
      strcpy(nuh, "NEVER");
    send_to_user(from, "Number of people killed: %5i", user->totkill);
    send_to_user(from, "     last one at approx:     %s", nuh);
    send_to_user(from, "\x2------ End of Tot Stats ------\x2");
  }
  return;
}

void c_getmsg(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  int num;
  UserHostStruct *user;
  MessagesStruct *mess;

  user = find_userhost(from);
  if (!user)
  {
    update_userhostlist(from, "", getthetime());
    send_to_user(from, "\x2No more messages\x2");
    return;
  }
  if (!*rest)
    num = user->msgnum + 1;
  else
    num = atoi(rest);
  if (num > user->totmsg)
  {
    send_to_user(from, "\x2No more messages\x2");
    return;
  }
  mess = get_message(user->messages, num);
  if (!mess)
  {
    send_to_user(from, "\x2Message #%i doesn't exist\x2", num);
    send_to_user(from, "%cCurrent Msg: #%i,  Last Msg: %i\x2", '\x2',
		 user->msgnum + 1, user->totmsg);
    return;
  }
  if (num > user->msgnum)
    user->msgnum = num;

  send_to_user(from, "\x2Message %i of %i\x2", num, user->totmsg);
  send_to_user(from, "%cFrom: %s\x2", '\x2', mess->from);
  send_to_user(from, "\x2Time: %s\x2", ctime(&mess->time));
  send_to_user(from, "\x2-----------------------------------------------\x2");
  send_to_user(from, "\x2%s\x2", mess->text);
  if (num == user->totmsg)
    send_to_user(from, "\x2-------------- No more messages ---------------\x2");
  else
    send_to_user(from, "\x2---------------- More messages ----------------\x2");

  return;
}

void c_sendmsg(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  char nuh[MAXLEN], nuh2[MAXLEN];
  char *nick;
  UserHostStruct *temp;
  char *ptr;

  nick = strtok(rest, " ");

  rest = strtok(NULL, "\n");
  if (!nick || !rest)
  {
    send_to_user(from, "\x2Usage: sm [nick or nuh] [themessage]\x2");
    return;
  }
  if (!strchr(nick, '@') && (num_userhost(nick) != 1))
  {
    send_to_user(from, "\x2That nick doesn't exist or there is more than\x2");
    send_to_user(from, "\x2one userhost for it...please do sm n!u@h message\x2");
    return;
  }
  if (!strchr(nick, '@'))
  {
    temp = find_userhostfromnick(UserHostList, nick);
    strcpy(nuh, temp->name);
  }
  else
  {
    strcpy(nuh, nick);
    temp = find_userhost(nuh);
    if (!temp)
    {
      update_userhostlist(nuh, "<UNKNOWN>", 0);
      temp = find_userhost(nuh);
      if (!temp)
      {
	send_to_user(from, "\x2No such nick!user@host known\x2");
	return;
      }
    }
  }
  if (add_message(&temp->messages, from, rest, temp->totmsg + 1))
  {
    temp->totmsg = temp->totmsg + 1;
    send_to_user(from, "\x2Message saved to %s\x2", nuh);
    ptr = strchr(nuh, '!');
    ptr++;

    strcpy(nuh2, getuserhost(getnick(nuh)));
    if (*nuh2)
      if (!strcasecmp(nuh2, ptr))
      {
	send_to_user(nuh, "\x2You have new mail\x2");
	send_to_user(nuh, "\x2Use \"%cgetmsg\" to read messages\x2", cmdchar);
      }
  }
  else
    send_to_user(from, "\x2Problem saving the message\x2");
  return;
}

void c_scanmsg(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  UserHostStruct *user;
  MessagesStruct *mess;
  char fr[255];

  user = find_userhost(from);
  if (!user)
  {
    send_to_user(from, "\x2No more messages\x2");
    return;
  }
  send_to_user(from, "\x2 Num         From                    Message\x2");
  for (mess = user->messages; mess; mess = mess->next)
  {
    strcpy(fr, left(mess->from, 20));
    send_to_user(from, "\x2[%3i] %20s %37s\x2", mess->num, fr,
		 left(mess->text, 37));
  }
  send_to_user(from, "\x2---------- end of scan -------------\x2");
}

void c_roll(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  int my_rand;
  int size;
  char *target;

  rest = strtok(rest, " ");
  if (rest && *rest)
  {
    size = atoi(rest);
  }
  else
    size = 6;
  if(size <= 2)
  {
    send_to_user(from, "\002Die must have at least 2 sides.\002");
    return;
  }
  my_rand = (rand() % size);
  my_rand++;
  if(to)
    target = to;
  else
    target = from; 
  sendprivmsg(target, "a \002%d\002 shows on the %d sidded die.", my_rand, size);
}

void c_8ball(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  int my_rand = 0;
  char outcome[MAXLEN];

  my_rand = (rand() % 11);
  switch (my_rand)
  {
  case 0:
    {
      strcpy(outcome, "Your outlook seems bleak.");
      break;
    }
  case 1:
    {
      strcpy(outcome, "Most definately.");
      break;
    }
  case 2:
    {
      strcpy(outcome, "It is decidedly so.");
      break;
    }
  case 3:
    {
      strcpy(outcome, "My sources say no.");
      break;
    }
  case 4:
    {
      strcpy(outcome, "Forcast hazey, try again.");
      break;
    }
  case 5:
    {
      strcpy(outcome, "It wont happen, not a chance, definately no.");
      break;
    }
  case 6:
    {
      strcpy(outcome, "Are you out of your MIND?");
      break;
    }
  case 7:
    {
      strcpy(outcome, "You bet!!");
      break;
    }
  case 8:
    {
      strcpy(outcome, "In your Dreams.");
      break;
    }
  case 9:
    {
      strcpy(outcome, "Its hard to be sure.");
      break;
    }
  case 10:
    {
      strcpy(outcome, "If the prophets wish it...");
      break;
    }
  }
  if (ischannel(to))
    sendprivmsg(to, "%s", outcome);
  else
    send_to_user(from, "%s", outcome);
}

void c_calc(char *from, char *channel, char *rest, char *to, CHAN_list *ChanPtr)
{
    char *target;

    if(!(rest && *rest))
    {
       send_to_user(from, "\002Usage: CALC <math formula>\002");
       return;
    }
    if(to && *to && userlevel(from, ChanPtr) >= ChanOp)
      target = to;
    else
      target = from;
    c_math(from, target, rest);
}

void c_heart(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  char *sendto;
  char *nick;
  int i;

  sendto = strtok(rest, " ");
  if (sendto)
    nick = strtok(NULL, " ");
  if (!sendto || !nick)
  {
    send_to_user(from, "\x2  Usage: heart where nick\x2");
    send_to_user(from, "%cExample: heart #TeenSex JoeBlow\x2", '\x2');
    return;
  }
  *(nick + 10) = '\0';
  for (i = 1; i < (10 - strlen(nick)) / 2; i++)
    strcat(nick, " ");
  if (ischannel(sendto))
  {
    sendprivmsg(sendto, "\x2          ..     ..\x2");
    sendprivmsg(sendto, "\x2       .`    `.\'    \'.\x2");
    sendprivmsg(sendto, "\x2       .%10s   .\x2", nick);
    sendprivmsg(sendto, "\x2         .         .\x2");
    sendprivmsg(sendto, "\x2            .   .\x2");
    sendprivmsg(sendto, "\x2              .\x2");
  }
  else
  {
    sendnotice(sendto, "\x2          ..     ..\x2");
    sendnotice(sendto, "\x2       .`    `.\'    \'.\x2");
    sendnotice(sendto, "\x2       .%10s   .\x2", nick);
    sendnotice(sendto, "\x2         .         .\x2");
    sendnotice(sendto, "\x2            .   .\x2");
    sendnotice(sendto, "\x2              .\x2");
  }
  return;
}

void c_last10(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  int i;

  send_to_user(from, "\x2Last 10 commands:\x2");
  send_to_user(from, "\x2-------------------------\x2");
  for (i = 0; i < 10; i++)
    send_to_user(from, "\x2%2i: %s\x2", i + 1, lastcmds[i]);
  send_to_user(from, "\x2------ end of list ------\x2");
  return;
}

void c_chaccess(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  int i, oldlevel, newlevel;
  char *name, *level;

  name = strtok(rest, " ");
  level = strtok(NULL, " ");
  if (!*rest || !name)
  {
    send_to_user(from, "\x2Usage: chaccess <commandname> [level]\x2");
    return;
  }
  if (level)
  {
    newlevel = atoi(level);
    if ((newlevel < 0) || (newlevel > 100))
    {
      send_to_user(from, "\x2Level must be between 0 and 100\x2");
      return;
    }
  }
  else
    newlevel = -1;
  if (newlevel > userlevel(from, ChanPtr))
  {
    send_to_user(from, "%cCan't change level to one higher than yours\x2",
		 '\x2');
    return;
  }
  for (i = 0; on_msg_commands[i].name != NULL; i++)
  {
    if (!strcasecmp(on_msg_commands[i].name, name))
    {
      oldlevel = on_msg_commands[i].userlevel;
      if (newlevel == -1)
      {
	send_to_user(from, "\x2The access level needed for that command is %i\x2",
		     oldlevel);
	send_to_user(from, "\x2To change it, specify a level\x2");
	return;
      }
      if (oldlevel > userlevel(from, ChanPtr))
      {
	send_to_user(from, "%cCan't change a level that is higher than yours\x2",
		     '\x2');
	return;
      }
      if (oldlevel == newlevel)
	send_to_user(from, "\x2The level was not changed\x2");
      else
	send_to_user(from, "\x2Level changed from %i to %i\x2",
		     oldlevel, newlevel);
      on_msg_commands[i].userlevel = newlevel;
      return;
    }
  }
  send_to_user(from, "\x2That command is not known\x2");
  return;
}

void c_server(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  char *login, *ircname;
  char *newport;
  char buf[100];

  int i = 1;

  rest = strtok(rest, " ");
  if (!rest || !*rest)
  {
    send_to_user(from, "\x2Usage: server <servername> [port] [login] [ircname]\x2");
    return;
  }
  newport = strtok(NULL, " ");
  if (!newport)
    newport = "6667";
  login = strtok(NULL, " ");
  ircname = strtok(NULL, " ");
  if (login)
    strcpy(bot.login, login);
  if (ircname)
    strcpy(bot.ircname, ircname);
  send_to_user(from, "\x2Trying new server: %s:%s\x2", rest, newport);
  sprintf(buf, "Changing servers\n");
  sendquit(buf);
  close(bot.server_sock);
  sleep(4);
  if ((bot.server_sock = connect_by_number(atoi(newport), rest)) < 0)
  {
    i = 0;
    reconnect_to_server();
  }
  else
  {
    sendregister(current_nick, bot.login, bot.ircname);
/* current_nick was bot.nick */
/* This now gets done on num 001 *      reset_channels(HARDRESET); */
  }
  if (i)
    send_to_user(from, "\x2Server change successful\x2");
  else
    send_to_user(from, "\x2Server change unsuccessful\x2");
  return;
}

void c_nextserver(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  char *login, *ircname;
  SERVER_list *ServPtr;

  login = strtok(rest, " ");
  ircname = strtok(NULL, "\n");
  if (login)
    strcpy(bot.login, login);
  if (ircname)
    strcpy(bot.ircname, ircname);

  ServPtr = bot.CurrentServer->next;
  if(!ServPtr)
    ServPtr = bot.servers;
  send_to_user(from, "\x2Jumping to next server:%s\x2", bot.CurrentServer->name);
  sendquit("Changing servers");
  reconnect_to_server();
  send_to_user(from, "\x2Now on server:%s\x2", bot.CurrentServer->name);
  return;
}

void c_addserver(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
   char *servname, *portstr, *passwd;
   int port;

   servname = strtok(rest, " ");
   portstr = strtok(NULL, " ");
   passwd = strtok(NULL, " ");

   if(!servname)
   {
      send_to_user(from, "\002Syntax: ADDSERVER <Servername> [port] [password]\002");
      return;
   }
   port = 6667;
   if(portstr)
     port = atoi(portstr);
   if(!passwd)
      passwd = "";
   AddServer(servname, port, passwd);
   send_to_user(from, "\002Added server %s:%d to my autoconnect database.\002", servname, port);
   SaveDatFile(Globals);
}

void c_remserver(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
    char *server;
    SERVER_list **last;
    SERVER_list *ServPtr;

    server = strtok(rest, " ");
    if(!server)
    {
       send_to_user(from, "\002Syntax: REMSERVER <servername>\002");
       return;
    }
    last = &bot.servers;
    for(ServPtr = bot.servers;ServPtr;ServPtr=ServPtr->next)
    {
       if(!strcasecmp(ServPtr->name,server))
       {
          *last = ServPtr->next;
          free(ServPtr);
          send_to_user(from, "\002Removed server %s from autoconnect list.\002", server);
          return;
       }
       last = &ServPtr->next;
    }
    send_to_user(from, "\002Server %s not found.\002", server);
    SaveDatFile(Globals);
}

void c_listservers(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  SERVER_list *ServPtr;

  send_to_user(from, "=----------\002Server\002--------------\002Port\002----\002Pass\002---\002Conn\002-\002Att\002-\002Err\002--=");
  for(ServPtr = bot.servers;ServPtr;ServPtr=ServPtr->next)
  {
     send_to_user(from, "\002%30s:%6d %8s %3d %3d %3d", ServPtr->name, ServPtr->port, 
                               ServPtr->password,
                               ServPtr->connects, ServPtr->attempts, ServPtr->errors);
  }
  send_to_user(from, "=---------------- End of Server list --------------------=");
}

void c_top10kills(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  UserHostStruct *user;
  char nicks[10][100] =
  {"", "", "", "", "", "", "", "", "", ""};
  int i = 0, kills[10] =
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  int ii, smallest, smallnum, save;
  int temp[10] =
  {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

  for (user = UserHostList; user; user = user->next)
  {
    if (user->totkill)
    {
      for (i = 0; i < 10; i++)
      {
	if (!*nicks[i])
	{
	  strcpy(nicks[i], getnick(user->name));
	  kills[i] = user->totkill;
	  break;
	}
	if (user->totkill >= kills[i])
	{
	  smallest = kills[0];
	  smallnum = 0;
	  for (ii = 1; ii < 10; ii++)
	    if (kills[ii] < smallest)
	    {
	      smallnum = ii;
	      smallest = kills[ii];
	    }
	  kills[smallnum] = user->totkill;
	  strcpy(nicks[smallnum], getnick(user->name));
	  break;
	}
      }
    }
  }
  for (i = 0; i < 9; i++)
    for (ii = i + 1; ii < 10; ii++)
      if (kills[temp[ii]] > kills[temp[i]])
      {
	save = temp[ii];
	temp[ii] = temp[i];
	temp[i] = save;
      }
  send_to_user(from, "\x2------- Top Ten Kills ------\x2");
  send_to_user(from, "\x2   Oper         # of kills\x2");
  send_to_user(from, "\x2------------+---------------\x2");
  for (i = 0; i < 10; i++)
    send_to_user(from, "\x2 %10s |   %4i\x2", nicks[temp[i]], kills[temp[i]]);
  send_to_user(from, "\x2------------+---------------\x2");
  send_to_user(from, "\x2------- end of list --------\x2");
  return;
}

void c_logchat(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  rest = strtok(rest, " ");

  if (!rest || !*rest)
  {
    bot_reply(from, 10);
    return;
  }
  if (!mychannel(rest))
  {
    bot_reply(from, 11);
    return;
  }
  if (is_in_list(LogChatList, rest))
  {
    send_to_user(from, "%cAlready logging chat on that channel\x2",
		 '\x2');
    return;
  }
  if (add_to_list(&LogChatList, rest))
    send_to_user(from, "\x2Now logging chat on %s\x2", rest);
  else
    send_to_user(from, "\x2Problem logging chat\x2");
  return;
}

void c_rlogchat(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  rest = strtok(rest, " ");
  if (!rest || !*rest)
  {
    bot_reply(from, 10);
    return;
  }
  if (!is_in_list(LogChatList, rest))
  {
    send_to_user(from, "%cAlready not logging chat on that channel\x2",
		 '\x2');
    return;
  }
  if (remove_from_list(&LogChatList, rest))
    send_to_user(from, "\x2No longer logging chat on %s\x2", rest);
  else
    send_to_user(from, "\x2Problem removing the log chat\x2");
  return;
}

void c_removeuh(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{

  if (!*rest || !strchr(rest, '!') || !strchr(rest, '@'))
  {
    send_to_user(from, "\x2Specify the full nick!user@host to delete\x2");
    return;
  }

  if (!remove_userhost(rest))
  {
    send_to_user(from, "\x2%s not found in memory\x2", rest);
    return;
  }
  send_to_user(from, "\x2%s removed from memory\x2", rest);
  return;
}

void c_saveuserhost(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  if (!write_userhost(userhostfile))
    send_to_user(from, "\x2Userhost info could not be save to file: %s\x2",
		 userhostfile);
  else
    send_to_user(from, "\x2Userhost info saved to file: %s\x2",
		 userhostfile);
}

void c_ping(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  if (ischannel(to))
    sendprivmsg(to, "\x2Pong\x2");
  else
    send_to_user(from, "\x2Pong\x2");
  return;
}

void c_steal(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  ListStruct *temp;

  if (!*rest || !ischannel(rest))
  {
    send_to_user(from, "\x2Trying to steal the following channels:\x2");
    send_to_user(from, "\x2---------------------------------------\x2");
    for (temp = StealList; temp; temp = temp->next)
      send_to_user(from, "\x2%s\x2", temp->name);
    send_to_user(from, "\x2-------------- end of list ------------\x2");
    return;
  }
  if (is_in_list(StealList, rest))
  {
    send_to_user(from, "%cAlready stealing that channel\x2", '\x2');
    return;
  }
  if (add_to_list(&StealList, rest))
  {
    send_to_user(from, "\x2Now trying to steal %s\x2", rest);
    if (!mychannel(rest))
      join_channel(rest, 1);
  }
  else
    send_to_user(from, "\x2Problem adding the channel\x2");
  return;
}

void c_nosteal(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  if (!*rest || !ischannel(rest))
  {
    bot_reply(from, 10);
    return;
  }
  if (!is_in_list(StealList, rest))
  {
    send_to_user(from, "\x2I'm not stealing that channel\x2");
    return;
  }
  if (remove_from_list(&StealList, rest))
    send_to_user(from, "\x2No longer stealing %s\x2", rest);
  else
    send_to_user(from, "\x2Problem deleting the channel\x2");
  leave_channel(rest);
  return;
}

void c_cycle(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  send_to_user(from, "%cCycling channel %s\x2", '\x2', channel);
  cycle_channel(channel);
}

/* TODO : Someone with kerbos knoledge or access, figure out if this stuff
 * still works as it should. */
void c_kinit(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
#ifndef KINIT
  send_to_user(from, "\x2KINIT not defined in config.h\x2");
  return;
#endif
  make_process(kinitfile, "");
  send_to_user(from, "\x2Running %s\x2", kinitfile);
}

/* TODO : the whole file server situation needs to be redone... */
#ifdef NOTDEF
void c_display(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  FILE *in;
  char buffer[255];
  char filename[255];
  int level, ulevel;

  strcpy(filename, download_dir);
  if (!*rest)
  {
    send_to_user(from,
		 "\x2Use \"%cdisplay <filename>\" to view one of these files:\x2",
		 cmdchar);
    if ((in = fopen(filedispfile, "r")) == NULL)
    {
      send_to_user(from, "\x2No files available\x2");
      return;
    }
    send_to_user(from, "\x2-------- Filename --------+- Level Needed -\x2");
    while (!feof(in))
    {
      if (fscanf(in, "%s %i\n", buffer, &level))
	send_to_user(from, "%25s |    %3i", buffer, level);
    }
    send_to_user(from, "\x2--------------------------+----------------\x2");
    fclose(in);
    return;
  }
  ulevel = userlevel(from, ChanPtr);
  if ((in = fopen(filedispfile, "r")) == NULL)
  {
    send_to_user(from, "\x2Problem opening %s\x2", filedispfile);
    return;
  }
  while (!feof(in))
  {
    fscanf(in, "%s %i\n", buffer, &level);
    if (!strcmp(rest, buffer))
    {
      if (level > ulevel)
      {
	send_to_user(from, "\x2Sorry, you need access level %i to get this file\x2", level);
	return;
      }
      fclose(in);
      strcat(filename, rest);
      if ((in = fopen(filename, "r")) == NULL)
      {
	send_to_user(from, "\x2Problem opening the file\x2");
	return;
      }
      while (!feof(in))
	if (readln_from_a_file(in, buffer))
	  send_to_user(from, buffer);
      fclose(in);
      return;
    }
  }
  send_to_user(from, "\x2That file does not exist...use \"%cdisplay\"\x2",
	       cmdchar);
  fclose(in);
}
#endif

void c_spy(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  if (!mychannel(channel))
  {
    send_to_user(from, "\x2I'm not on that channel\x2");
    return;
  }
  if (add_spy_chan(channel, from))
    send_to_user(from, "\x2Now spying on %s for you\x2", channel);
  else
    send_to_user(from, "\x2You are already spying on %s, dilweed\x2", channel);
  return;
}

void c_rspy(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  if (!mychannel(channel))
  {
    send_to_user(from, "\x2I'm not on that channel\x2");
    return;
  }
  if (del_spy_chan(channel, from))
    send_to_user(from, "\x2No longer spying on %s for you\x2", channel);
  else
    send_to_user(from, "\x2You are not spying on %s, dilweed\x2", channel);
  return;
}

void c_repeat(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  int numoftimes = 0, invalid = 0, I;
  char str[MAXLEN], cmdstr[MAXLEN];
  char *cmd;

  strcpy(str, rest);
  cmd = strchr(str, ' ');
  cmd[0] = '\0';
  cmd++;
  for (I = 0; I < strlen(str); I++)
    if (!isdigit(str[I]))
      invalid++;
  if (invalid != 0)
  {
    send_to_user(from, "\x2Syntax: repeat <xx> <command>\x2");
    send_to_user(from, "=x2  Where xx = Times to repeat\x2");
    return;
  }
  numoftimes = atoi(str);
  sprintf(str, "%c%s", cmdchar, cmd);
  strcpy(cmdstr, str);
  for (I = 0; I < numoftimes; I++)
  {
    strcpy(str, cmdstr);
/*
 * TODO : figure out an elegant way to do this... 
//        on_command(from, to, channel, command, str);
*/
  }
}

void c_spylist(char *from, char *channel, char *rest, char *to, CHAN_list * ChanPtr)
{
  send_to_user(from, "\x2--- Spylist for %s\x2", channel);
  show_spy_list(from, channel);
  send_to_user(from, "\x2--- end of list ---\x2");
}
