/*
	  StelBot Version 1.0 By Bryan8
*/

#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <signal.h>

#include "config.h"
#include "userlist.h"
#include "channel.h"
#include "parse.h"


char botdate[18] = "Novermber 15, 1994";


char lastcmd[1024] = "";
char ownernick[MAXNICKLEN] = "";
char owneruhost[FNLEN] = "";
int holdnick = 0;
char cshell[20] = "/bin/sh";
char  current_nick[MAXNICKLEN];
char ircserver[255] = "";
char *portnumber;
char *progname;
char settingsfile[FNLEN] = SETTINGSFILE;

UserStruct *UserList;
ShitStruct *ShitList;
ListStruct *KickSays;
ListStruct *KickSayChan;
ListStruct *SpyMsgList;
ListStruct *LogChatList;
ListStruct *StealList;
UserHostStruct *UserHostList;
FloodStruct *FloodList;
int beeptoggle = 0;
int seentoggle = 1;
int pmlogtoggle = 0;
int kslevel = 80;
int maxsameuser = 2;
int mplevel = 1;
int malevel = 90;
int minbanlevel = 0;
int minunbanlevel = 0;
int sbtype = 1;
int minoplevel = 0;
int mindeoplevel = 0;
int idlelimit = 300;
int weltoggle = 0;
int cctoggle = 1;
int sdtoggle = 1;
int soptoggle = 0;
int prottoggle = 1;
int aoptoggle = 0;
int shittoggle = 0;
int idletoggle = 0;
int enfmtoggle = 1;
int floodtoggle = 1;
int pubtoggle = 1;
int masstoggle = 1;
int massbanlimit = 4;
int massdeoplimit = 4;
int masskicklimit = 3;
int mutoggle = 0;
int floodlimit = 6;
int klogtoggle = 1;
int logtoggle = 0;
char cmdchar = '!';
char nickname[255] = "Default";
char channels[255] = "#Pub";
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

char writeruhost[255] = "*!*bryan@*primenet.com";
char lastcmds[10][100];
char welcomemess[10][50];
int numwelcome=0;
long uptime;             /* time when bot was started   */
#ifdef KINIT
long kinittime;
#endif

void    sig_hup();
void    sig_cntl_c();
void    sig_segv();     
int readsettings(void);
int rereadsettings(int num);

FILE *fp;
char lin[MAXLEN];

void readwelcome(void)
{
  char buffer[MAXLEN], filename[FNLEN];
  FILE *fp;
  int num=0;

  strcpy(filename, bot_dir);
  strcat(filename, "StelBot.welcome");
  if (!(fp = fopen(filename, "r")))
    return;
  
  while ((num < 10) && (!feof(fp)))
    {
      if (readln_from_a_file(fp, buffer))
	{
	  strncpy(welcomemess[num], buffer, 49);
	  num++;
	}
    }
  fclose(fp);
  numwelcome = num;
}

int rereadsettings(int num)
{

	if (num != 0)
	{
		if( ( fp = fopen( settingsfile, "r" ) ) == NULL )
		{
			printf( "File \"%s\" not found!!!\n", settingsfile );
			return FALSE;
		}
		if (readln_from_a_file( fp, lin))
			;
		if (readln_from_a_file( fp, lin))
			;
		if (readln_from_a_file( fp, lin))
			;
		if (readln_from_a_file( fp, lin))
			;
		if (readln_from_a_file( fp, lin))
			;
		if (readln_from_a_file( fp, lin))
			;
		if (readln_from_a_file( fp, lin))
			;
		if (readln_from_a_file( fp, lin))
			;
	}
	if (readln_from_a_file( fp, lin))
	  sscanf(lin, "%i", &logtoggle);
	if (readln_from_a_file( fp, lin))
	  sscanf(lin, "%i", &klogtoggle);
	if (readln_from_a_file( fp, lin))
		sscanf(lin, "%i", &aoptoggle);
	if (readln_from_a_file( fp, lin))
	  sscanf(lin, "%i", &prottoggle);
	if (readln_from_a_file( fp, lin))
		sscanf(lin, "%i", &shittoggle);
	if (readln_from_a_file( fp, lin))
		sscanf(lin, "%i", &enfmtoggle);
	if (readln_from_a_file( fp, lin))
	  sscanf(lin, "%i", &idletoggle);
	if (readln_from_a_file( fp, lin))
	  sscanf(lin, "%i", &mutoggle);
	if (readln_from_a_file( fp, lin))
	  sscanf(lin, "%i", &floodtoggle);
	if (readln_from_a_file( fp, lin))
	  sscanf(lin, "%i", &sdtoggle);
	if (readln_from_a_file( fp, lin))
	  sscanf(lin, "%i", &soptoggle);
	if (readln_from_a_file( fp, lin))
	  sscanf(lin, "%i", &weltoggle);
	if (readln_from_a_file( fp, lin))
	  sscanf(lin, "%i", &masstoggle);
	if (readln_from_a_file( fp, lin))
	  sscanf(lin, "%i", &massbanlimit);
	if (readln_from_a_file( fp, lin))
	  sscanf(lin, "%i", &massdeoplimit);
	if (readln_from_a_file( fp, lin))
	  sscanf(lin, "%i", &masskicklimit);
	if (readln_from_a_file( fp, lin))
	  sscanf(lin, "%i", &sbtype);
	if (sbtype > 2)
	  sbtype = 2;
	if (readln_from_a_file( fp, lin))
	  sscanf(lin, "%i", &kslevel);
	if (readln_from_a_file( fp, lin))
	  sscanf(lin, "%i", &malevel);
	if (readln_from_a_file( fp, lin))
	  sscanf(lin, "%i", &mplevel);
	if (readln_from_a_file( fp, lin))
	  sscanf(lin, "%i", &minoplevel);
	if (readln_from_a_file( fp, lin))
	  sscanf(lin, "%i", &mindeoplevel);
	if (readln_from_a_file( fp, lin))
	  sscanf(lin, "%i", &minbanlevel);
	if (readln_from_a_file( fp, lin))
	  sscanf(lin, "%i", &minunbanlevel);
	if (readln_from_a_file( fp, lin))
	  {
		 sscanf(lin, "%s", bot_dir);
		 strcpy(mail_dir, bot_dir);
	  }
	if (readln_from_a_file( fp, lin))
		sscanf(lin, "%s", upload_dir);
	if (readln_from_a_file( fp, lin))
		sscanf(lin, "%s", download_dir);
	fclose(fp);
	return TRUE;
}

int readsettings(void)
{

	if( ( fp = fopen( settingsfile, "r" ) ) == NULL )
	{
		printf( "File \"%s\" not found!!!\n", settingsfile );
		return FALSE;
	}
	if (readln_from_a_file( fp, lin))
		sscanf(lin, "%s", nickname);
	if (readln_from_a_file( fp, lin))
		sscanf(lin, "%s", ircserver);
	if (readln_from_a_file( fp, lin))
		sscanf(lin, "%s", USERNAME);
	if (readln_from_a_file( fp, lin))
		strcpy(ircname, lin);
	if (*(ircname+strlen(ircname)) == '\n')
		*(ircname+strlen(ircname)) = '\0';
	if (readln_from_a_file( fp, lin))
	  sscanf(lin, "%s", ownernick);
	if (readln_from_a_file( fp, lin))
	  sscanf(lin, "%s", owneruhost);
	strcpy(channels, "#");
	if (readln_from_a_file( fp, lin))
		sscanf(lin, "%s", channels+1);
	if (readln_from_a_file( fp, lin))
		cmdchar = *lin;

	return rereadsettings(0);
}

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

void    readlevelfiles()
{
	if( !read_userlist( usersfile ) )
	  {
	    printf("No usersfile found...please make sure %s is set up correctly\n", SETTINGSFILE);
	    printf("The bot directory is set as: %s\n", bot_dir);
	    printf("And the usersfile name needs to be: %s.usersfile\n", 
		   current_nick);
		exit(0);
	  }
	read_shitlist( shitfile );
}

int  start_a_bot( char *name , char *channel )
{
	struct  timeval  timer;
	fd_set  rd, wd;
	char *chan;

	strcpy( current_nick, name );

	strcpy(usersfile, bot_dir);
	strcpy(kinitfile, bot_dir);
	strcat(kinitfile, "KINIT");
	strcpy(helpfile, bot_dir);
	strcat(usersfile, name);
	strcpy(shitfile, usersfile);
	strcpy(dcclogfile, usersfile);
	strcpy(botlogfile, usersfile);
	strcpy(errorfile, usersfile);
	strcat(helpfile, "StelBot.helpfile");
	strcpy(killfile, usersfile);
	strcpy(pmlogfile, usersfile);
	strcpy(filelistfile, download_dir);
	strcpy(filedispfile, download_dir);
	strcpy(replyfile, bot_dir);
	strcat(filelistfile, "StelBot");
	strcat(filedispfile, "StelBot");
	strcat(replyfile, "StelBot");
	strcpy(userhostfile, usersfile);
	strcpy(messagesfile, usersfile);
	strcpy(serversfile, usersfile);
	strcat(serversfile, ".servers");
	strcat(usersfile, ".userlist");
	strcat(shitfile, ".shitlist");
	strcat(killfile, ".kills");
	strcat(errorfile, ".errors");
	strcat(botlogfile, ".logfile");
	strcat(dcclogfile, ".dcclog");
	strcat(pmlogfile, ".privmsgs");
	strcat(filelistfile, ".filelist");
	strcat(filedispfile, ".displist");
	strcat(replyfile, ".replies");
	strcat(userhostfile, ".userhost");
	strcat(messagesfile, ".messages");
	strcpy(mailfile, mail_dir);
	strcat(mailfile, "draft");
	/* set uptime */
	uptime = time(NULL);
#ifdef KINIT
	kinittime = uptime;
#endif
	readlevelfiles();
	StealList = init_list();
	LogChatList = init_list();
	KickSays = init_list();
	KickSayChan = init_list();
	add_to_list(&KickSayChan, "*");
	UserHostList = init_userhostlist();
	FloodList = init_flood();
	read_userhost(userhostfile);
	read_serverlist(ircserver, atoi(portnumber));
        save_backups();
	chan = strtok(channel, ",");
	startbot(current_nick, ircname, USERNAME, chan);
	while (chan)
	  {
	    join_channel(chan);
	    chan = strtok(NULL, ",");
	  }
	while( 1 )
	{   
		timer.tv_sec = WAIT_SEC;
		timer.tv_usec = WAIT_MSEC;

		FD_ZERO( &rd );
		FD_ZERO( &wd );

		set_dcc_fds( &rd, &wd );
		set_server_fds( &rd, &wd );
		switch(select( NFDBITS, &rd, 0, 0, &timer ))
		{
		case 0:
#ifdef DBUG
			printf("SELECT: timeout\n");
			break;
#endif
		case -1:
#ifdef DBUG
			printf("SELECT: error\n");
#endif
			break;
		default:
			parse_server_input( &rd );
			parse_dcc_input( &rd );
			/* Only join channels we might have been kicked of */
			reset_channels( SOFTRESET );
			break;
		}
	}
}

void showhelp(void)
{
	printf( "-------------- HELP --------------\n" );
	printf( "usage: %s [switches [args]]\n", progname );
	printf( "-h               shows this help\n" );
	printf( "-b               don't run %s in the background\n",
		progname );
	printf( "-u username      use <username> as username\n");
/*	printf( "-i ircname       use <ircname> as ircname\n" );
*/	printf( "-n nick          use <nick> as nick\n" );
	printf( "-s server        use <server> as server\n" );
	printf( "-p port          use port <port>\n" );
	printf( "-c channel       join channel <channel>\n" );
	printf( "-z cmdchar       use this command char\n\n");
	exit( 0 );
}

int main( int argc, char *argv[] )
{
	char    *arg;
	char    *myname;
	int     do_fork = TRUE;
	int i;
	myname = argv[0];
	progname = argv[0];

	printf( "%s Version %s (c) Bryan8 %s\n", myname, VERSION, botdate);
	printf( "For more info: e-mail bryan@primenet.com\n");

	portnumber = IRCPORT;

	for (i=0;i<10;i++)
	  {
	    strcpy(lastcmds[i], "");
	    strcpy(welcomemess[i], "");
	  }
	readwelcome();

	srand( (unsigned)time( NULL ) );

	if (!readsettings())
		showhelp();
	while( argc > 1 && argv[1][0] == '-' )
	{
		argc--;    
		arg = *++argv;
		switch( arg[1] )
		{
		case 'h':
			showhelp();
			break;
		case 'b':
			do_fork = FALSE;
			break;
		case 'n':
			if( argc > 1 )
			{
				strcpy(nickname, *++argv);
				argc--;
			}
			break;
		case 'u':
			if( argc > 1 )
			{
				strcpy(USERNAME, *++argv);
				argc--;
			}
			break;
		case 'i':
			if( argc > 1 )
			{
				strcpy(ircname, *++argv);
				argc--;
			}
			break;
		case 'z':
			if (argc > 1)
			{
			   *++argv;
				cmdchar = **argv;
			argc--;
				}
			break;
		case 's':
			if( argc > 1 )
			{
				strcpy(ircserver, *++argv);
				argc--;
			}
			break;  
		case 'p':
			if( argc > 1 )
			{
				portnumber = *++argv;
				argc--;
			}
			break;  
		case 'c':
		  printf("Channels: %s %i\n", argc, channels);
			if( argc > 1 )
			{
				strcpy(channels, *++argv);
				argc--;
			}
			  printf("Channels2: %s \n", channels);
			break; 
		default:  
			printf( "Unknown option %s\n", arg );   
			exit( 1 );
		}         
	}


	if( do_fork )
	{
		printf( "%s -=[*](StelBot Is Engaged)[*]=- \n", myname );
		if( !fork() )
			start_a_bot( nickname, channels );
		else
			exit( 0 );
	}
	else
		start_a_bot( nickname, channels );
}





