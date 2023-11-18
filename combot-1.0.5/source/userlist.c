#include <string.h>
#include <stddef.h>
#include <stdio.h>
#include <time.h>
#include "userlist.h"
#include "config.h"
#include "misc.h"
#include "function.h"

extern UserStruct *UserList;
extern ShitStruct *ShitList;
extern FloodStruct *FloodList;

extern char lastcmd[1024];
extern char current_nick[MAXNICKLEN];
extern UserHostStruct *UserHostList;
extern int prottoggle, shittoggle;

time_t getthetime( void )
{
	time_t t;

	t = time( ( time_t *) NULL);
	return t;
}

int getseconds( time_t seconds)
{
	time_t t;

	t = getthetime();
	return (t-seconds);
}

UserHostStruct *init_userhostlist( void )
{
	UserHostStruct *l_list;
	l_list = (UserHostStruct *) malloc (sizeof(l_list));
	l_list = NULL;
	return (l_list);
}

UserHostStruct *find_userhostfromnick(UserHostStruct *begin, char *nick)
{
	UserHostStruct *Temp;

	if (!nick)
		return NULL;

	for (Temp = begin; Temp; Temp = Temp->next )
		if (!strcasecmp( getnick(Temp->name), nick))
			return Temp;
	return NULL;
}

UserHostStruct *find_userhost(char *fullname)
{
     UserHostStruct *Temp;

     char buffer[255]; 
     if (!fullname)
	 return NULL;
     if (!strchr(fullname, '*'))
     {
	char n[100], u[100], h[100], m[100];
	sep_userhost(fullname, n, u, h, m);
	sprintf(buffer, "%s!%s@*%s", n, u, h);
     }
     else
       strcpy(buffer, fullname);
	
     for (Temp = UserHostList; Temp; Temp = Temp->next )
	if (!matches( buffer, Temp->name))
	   return Temp;
     return NULL; 
}

int num_userhost(char *nick)
{
	int num = 0;
	UserHostStruct *Temp;
	Temp = UserHostList;
	while (Temp != NULL)
	{
		Temp = find_userhostfromnick(Temp, nick);
		if (Temp)
		{
			num++;
			Temp = Temp->next;
		}
	}
	return num;
}

char *getuserhost_fromlist(char *nick)
{
	int num;
	UserHostStruct *Temp;
	char *ptr;
	num = num_userhost(nick);
	if (num == 1)
	{
		Temp = find_userhostfromnick(UserHostList, nick);
		ptr = strchr(Temp->name, '!');
		ptr++;
		if (ptr)
		   return ptr;
	}
	return (getuserhost(nick));
}

char *getlastchan(char *fullname)
{
	UserHostStruct *Temp;
	Temp = find_userhost(fullname);
	if (Temp)
		return Temp->channel;
	return NULL;
}

time_t getlasttime(char *fullname)
{
	UserHostStruct *Temp;
	Temp = find_userhost(fullname);
	if (Temp)
		return Temp->time;
	return 0;
}

int remove_userhost( char *thename)
{
  UserHostStruct *old;
  UserHostStruct *dummy;

  if( (dummy = find_userhost( thename )) == NULL )
    return(FALSE);
  if (dummy == UserHostList)
    {
      UserHostList = dummy->next;
      free(dummy);
      return TRUE;
    }
  for( old = UserHostList; old; old = old->next )
    if( old->next == dummy )
      {
	old->next = dummy->next;
	free( dummy );
	return(TRUE);
      }
  return(FALSE);
}

int update_userhostlist( char *thename2, char *thechannel, time_t thetime)
{
	char n[100], m[100], u[100], h[100];
	char buffer[255];
	char buf2[255], *thename = buf2;
	UserHostStruct *New_name;

	strcpy(thename, thename2);
	sep_userhost( thename, n, u, h, m);
	strcpy(u, strip(u, '#'));
	strcpy(u, strip(u, '~'));
	sprintf(buffer, "%s!*%s@*%s",n, u, h);
	strcat(lastcmd, "uuok1");
	New_name = find_userhost(buffer);
	strcat(lastcmd, "uuok2");
	if (!New_name)
	{

	  if( (New_name = (UserHostStruct *) malloc (sizeof(*New_name))) == NULL)
	    {           
	      strcat(lastcmd, "MEM");
	      return FALSE;
	    }
	  
	  strcpy(New_name->name, thename);
	  New_name->next = UserHostList;
	  New_name->kicktime = thetime;
	  New_name->killtime = 0;
	  New_name->bantime = thetime;
	  New_name->deoptime = thetime;
	  New_name->bannum = 0;
	  New_name->deopnum = 0;
	  New_name->kicknum = 0;
	  New_name->totkick = 0;
	  New_name->totkill = 0;
	  New_name->totop = 0;
	  New_name->totban = 0;
	  New_name->totdeop = 0;
	  New_name->totmsg = 0;
	  New_name->msgnum = 0;
	  New_name->messages = init_messages();
	  UserHostList = New_name;
	  strcpy(New_name->channel, "<UNKNOWN>");
	}
	strcat(lastcmd, "uuok3");
	if (*thechannel)
	  strcpy(New_name->channel, thechannel);
	New_name->time = thetime;
	return TRUE;
}

int remove_all_from_userhostlist( void )
{
	UserHostStruct *dummy;

	for( dummy = UserHostList; dummy; dummy = dummy->next )
		free(dummy);
	UserHostList = init_userhostlist();
}

extern char writeruhost[255];
extern char owneruhost[255];

ListStruct *find_list(ListStruct *WhichList, char *thename)
{
	ListStruct *Temp;

	if (!thename)
		return NULL;

	for (Temp = WhichList; Temp; Temp = Temp->next )
		if (!matches( Temp->name, thename))
			return Temp;
	return NULL;
}

int is_in_list( ListStruct *WhichList, char *thename)
{
   return (find_list( WhichList, thename) != NULL);
}

ListStruct *init_list( void )
{
	ListStruct *l_list;
	l_list = (ListStruct *) malloc (sizeof(l_list));
	l_list = NULL;
	return (l_list);
}

int add_to_list( ListStruct **WhichList, char *thename)
{
     ListStruct *New_name;

     if( (New_name = find_list(*WhichList, thename)) != NULL )
	return FALSE;
     if( (New_name = (ListStruct *) malloc (sizeof(*New_name))) == NULL)
	return FALSE;
     
     strcpy(New_name->name, thename );
     New_name->next = *WhichList;
     *WhichList = New_name;
     return TRUE;
}

int remove_from_list( ListStruct **WhichList, char *thename )
{
	ListStruct *old;
	ListStruct *dummy;

	if( (dummy = find_list( *WhichList, thename )) == NULL )
		return(FALSE);
	if (dummy == *WhichList)
	{
		*WhichList = dummy->next;
		free(dummy);
		return TRUE;
	}
	for( old = *WhichList; old; old = old->next )
		if( old->next == dummy )
		{
			old->next = dummy->next;
			free( dummy );
			return(TRUE);
		}
	return(FALSE);
}

int remove_all_from_list( ListStruct **WhichList )
{
	ListStruct *dummy;

	for( dummy = *WhichList; dummy; dummy = dummy->next )
		free(dummy);
	*WhichList = init_list();
}

MessagesStruct *init_messages( void)
{
  MessagesStruct *l_list;
  l_list = (MessagesStruct *) malloc (sizeof(l_list));
  l_list = NULL;
  return (l_list);
}

int add_message( MessagesStruct **mess, char *from, char *message, int num)
{
  MessagesStruct *newmsg;

  if ((newmsg = (MessagesStruct *) malloc (sizeof(*newmsg))) == NULL)
    return FALSE;
  strcpy(newmsg->from, from);
  strcpy(newmsg->text, message);
  newmsg->time = getthetime();
  newmsg->num = num;
  newmsg->next = *mess;
  *mess = newmsg;
  return TRUE;
}

MessagesStruct *get_message(MessagesStruct *mess, int num)
{
  MessagesStruct *temp;

  for (temp=mess; temp; temp = temp->next)
      if (temp->num == num)
	return temp;
  return NULL;
}

FloodStruct *init_flood( void )
{
  FloodStruct *l_list;
  l_list = (FloodStruct *) malloc (sizeof(l_list));
  l_list = NULL;
  return l_list;
}

FloodStruct *find_flood(char *hostname)
{
  FloodStruct *temp;

  if (!hostname)
    return NULL;
  for (temp=FloodList;temp;temp=temp->next)
    if (!strcasecmp(temp->host, hostname))
      return temp;
  return NULL;
}

int add_flood(char *userhost)
{
  FloodStruct *New_user;
  char *hostname, *temp;

  temp = cluster(userhost);
  hostname = gethost(temp);
  if( (New_user = find_flood(hostname)) != NULL )
    return FALSE;
  if( (New_user = (FloodStruct *) malloc (sizeof(*New_user))) == NULL)
    return FALSE;
  strcpy(New_user->host, hostname);
  New_user->num = 0;
  New_user->thetime = getthetime();
  New_user->next = FloodList;
  FloodList = New_user;
  return TRUE;
}

int is_flooding(char *userhost)
{
  FloodStruct *dummy;
  char *hostname, *temp;
  time_t currenttime;

  if (!strchr(userhost, '@'))
    return 0;

  if (!strcasecmp(getnick(userhost), current_nick))
    return 0;
  temp = cluster(userhost);
  hostname = gethost(temp);
  
  if (!(dummy=find_flood(hostname)))
    {
      add_flood(userhost);
      if (!(dummy=find_flood(hostname)))
	return 0;
    }

  currenttime = getthetime();
  if ((currenttime-dummy->thetime) > 18)
    {
      dummy->num = 0;
      dummy->thetime = currenttime;
    }
  dummy->num++;
  if (dummy->num >= 9)
    {
      if (dummy->num == 10)
	{
	  send_to_user(userhost, "\x2You are now being ignored for a while...\x2");
	  dummy->thetime = currenttime; 
	}
      return TRUE;
    }
  return FALSE;
}

int remove_floodstruct(FloodStruct *dummy)
{
  FloodStruct *old;

  if (!dummy)
    return;
  if (dummy == FloodList)
    {
      FloodList = dummy->next;
      free(dummy);
      return TRUE;
    }
  for( old = FloodList; old; old = old->next )
    if( old->next == dummy )
      {
	old->next = dummy->next;
	free( dummy );
	return TRUE;
      }
  return FALSE;
}

void remove_oldfloods(void)
{
  FloodStruct *temp, *temp2;

  while (FloodList && ((getthetime()-FloodList->thetime) > 30))
    {
      temp = FloodList;
      FloodList = FloodList->next;
      free(temp);
    }
  temp = FloodList;
  while (temp && temp->next)
    {
      if ((getthetime()-(temp->next)->thetime) > 30)
	{
	  temp2 = temp->next;
	  temp->next=(temp->next)->next;
	  free(temp2);
	}
      else
	temp=temp->next;
    }
  return;
}

UserStruct *init_userlist( void )
{
	UserStruct *l_list;
	l_list = (UserStruct *) malloc (sizeof(l_list));
	l_list = NULL;
	return (l_list);
}

UserStruct *find_user(char *userhost)
{
	UserStruct *User;

	if (!userhost)
		return NULL;

	for( User = UserList; User; User = User->next )
	    if( !matches( User->userhost, userhost ) )
	       return(User);
	return(NULL);
}

int isuser( char *userhost )
{
    return (find_user(userhost) != NULL);
}

int add_to_userlist( char *userhost, int level, int aop, int prot)
{
	UserStruct *New_user;

	if( (New_user = find_user(userhost)) != NULL )
		return FALSE;
	if( (New_user = (UserStruct *) malloc (sizeof(*New_user))) == NULL)
		return FALSE;
	strcpy(New_user->userhost, userhost );
	New_user->level = level;
	New_user->isaop = aop;
	New_user->protlevel = prot;
	New_user->next = UserList;
	UserList = New_user;
	return TRUE;
}

int  remove_from_userlist( char *userhost )
{
	UserStruct *old;
	UserStruct *dummy;

	if( (dummy = find_user(userhost)) == NULL )
		return(FALSE);
	if (dummy == UserList)
	{
		UserList = dummy->next;
		free(dummy);
		return TRUE;
	}
	for( old = UserList; old; old = old->next )
		if( old->next == dummy )
		{
			old->next = dummy->next;
			free( dummy );
			return(TRUE);
		}
	return(FALSE);
}

ShitStruct *init_shitlist( void )
{
	ShitStruct *l_list;
	l_list = (ShitStruct *) malloc (sizeof(l_list));
	l_list = NULL;
	return (l_list);
}

ShitStruct *find_shit(char *userhost)
{
	ShitStruct *User;

	if (!userhost)
	  return NULL;

	for( User = ShitList; User; User = User->next )
	    if( !matches( User->userhost, userhost ) )
	       return(User);
	return(NULL);
}

ShitStruct *find_shitbm(char *userhost)
{
  ShitStruct *User;
  if (!userhost)
    return NULL;
  for (User=ShitList; User; User=User->next)
    if (!bancmp( User->userhost, userhost))
      return (User);
  return (NULL);
}

int add_to_shitlist( char *userhost, int level, char *channels)
{
	ShitStruct *New_user;
	char *channame;
	char buffer[MAXLEN];

	if( (New_user = find_shit(userhost)) != NULL )
		return FALSE;
	if( (New_user = (ShitStruct *) malloc (sizeof(*New_user))) == NULL)
		return FALSE;
	
	strcpy(New_user->userhost, userhost );
	New_user->level = level;
	New_user->next = ShitList;
	New_user->channels = init_list();
	if (channels)
	  channame = strtok(channels, SEPERATORS);
	if (channels && channame)
	  do
	  {
	    if (!ischannel(channame) && strcmp(channame, "*"))
	    {
	       strcpy(buffer, "#");
	       strcat(buffer, channame);
	       add_to_list(&(New_user->channels), buffer);
	    }
	    else
		add_to_list(&(New_user->channels), channame);
	} while (channame = strtok(NULL, SEPERATORS));
	ShitList = New_user;
	return TRUE;
}

int  remove_from_shitlist( char *userhost )
{
	ShitStruct *old;
	ShitStruct *dummy;

	if( (dummy = find_shit(userhost)) == NULL )
		return(FALSE);
	if (dummy == ShitList)
	{
		ShitList = dummy->next;
		remove_all_from_list(&(dummy->channels));
		free(dummy);
		return TRUE;
	}
	for( old = ShitList; old; old = old->next )
		if( old->next == dummy )
		{
			old->next = dummy->next;
			remove_all_from_list(&(dummy->channels));
			free( dummy );
			return(TRUE);
		}
	return(FALSE);
}

int change_userlist( char *userhost, int level, int aop, int prot)
{
	UserStruct *dummy;

	if( (dummy = find_user(userhost)) != NULL )
	{
		if (level != -1)
			dummy->level = level;
		if (aop != -1)
			dummy->isaop = aop;
		if (prot != -1)
			dummy->protlevel = prot;
		return TRUE;
	}
	return FALSE;
}

int userlevel( char *userhost )
{
	UserStruct *dummy;
	if (!userhost)
	  return 0;
	if( (dummy = find_user(userhost)) != NULL )
		return (dummy->level);
	if (!matches(owneruhost, userhost))
	    return 100;
	return 0;
}

int isaop( char *userhost )
{
	UserStruct   *dummy;

	if( (dummy = find_user(userhost)) != NULL )
		return (dummy->isaop);
	return 0;
}

int protlevel( char *userhost )
{
	UserStruct   *dummy;

	if (!prottoggle)
		return 0;
	
	if( (dummy = find_user(userhost)) != NULL )
		return (dummy->protlevel);
	return 0;
}

int protlevelbm( char *userhost)
{
	UserStruct *User;

	if (!prottoggle)
		return 0;

	for( User = UserList; User; User = User->next )
	   if( !bancmp( User->userhost, userhost ) )
		return User->protlevel;
	return 0;
}

char *shitchan( char *userhost )
{
  ShitStruct *dummy;
  ListStruct *temp;
  char chan[MAXLEN]="";

  if ((dummy=find_shit(userhost)) != NULL)
    {
      for (temp=dummy->channels;temp;temp=temp->next)
	{
	  strcat(chan, temp->name);
	  strcat(chan, " ");
	  if (!strcmp(temp->name, "*"))
	    return temp->name;
	}
      return chan;
    }
  return "";
}

int shitlevel( char *userhost )
{
	ShitStruct   *dummy;

	if( (dummy = find_shit(userhost)) != NULL )
		return (dummy->level);
	return 0;
}

int shitlevelbm( char *userhost)
{
	ShitStruct *User;

	if (!shittoggle)
	  return 0;
      
	for ( User = ShitList; User; User = User->next)
	  if ( !bancmp( User->userhost, userhost))
	    return User->level;
	return 0;
}

void show_shitlist( char *from, char *userhost )
{
	ShitStruct   *dummy;
	ListStruct *chan;
	char *temp;

	for( dummy = ShitList; dummy; dummy = dummy->next )
	   if(!matches(dummy->userhost, userhost) || !*userhost)
	      if (dummy->level)
		{
		  temp = shitchan(dummy->userhost);
		  send_to_user(from, " %40s | %3d  | %s",
			  dummy->userhost, dummy->level, temp);
		}
}

void show_userlist( char *from, char *userhost )
{
	UserStruct   *dummy;

	for( dummy = UserList; dummy; dummy = dummy->next )
	   if(!matches(dummy->userhost, userhost) || !*userhost)
		if (dummy->level)
		    send_to_user( from, " %40s | %d",
			dummy->userhost, dummy->level);
}

void show_protlist( char *from, char *userhost )
{
	UserStruct   *dummy;

	for( dummy = UserList; dummy; dummy = dummy->next )
		if(!matches(dummy->userhost, userhost) || !*userhost)
		  if (dummy->protlevel)
			send_to_user( from, " %40s | %d",
			dummy->userhost, dummy->protlevel);
}


int read_userhost( char *filename)
{
  UserHostStruct *dummy;
  FILE *in;
  if ((in = fopen( filename, "r")) == NULL)
    return(FALSE);
  while (!feof(in))
  {
    if ((dummy = (UserHostStruct *) malloc (sizeof(*dummy))) == NULL)
    {
       fclose(in);
       return (TRUE);
    }
    fscanf(in, "%s %s %li %li %li %li %li %i %i %i %i %i %i %i %i %i %i\n",
	     &dummy->name, &dummy->channel, &dummy->time, &dummy->kicktime,
	     &dummy->bantime, &dummy->deoptime, &dummy->killtime, &dummy->bannum,&dummy->deopnum,
	     &dummy->kicknum, &dummy->totmsg, &dummy->msgnum, &dummy->totop,
	     &dummy->totban, &dummy->totdeop, &dummy->totkick,&dummy->totkill);
      dummy->messages = init_messages();
      dummy->next = UserHostList;
      UserHostList = dummy;
  }
  fclose(in);
  return (TRUE);
}

int write_userhost( char *filename )
{
     UserHostStruct *dummy;
     FILE *out;

     if ((out = fopen( filename, "w" )) == NULL)
       return (FALSE);

     for (dummy=UserHostList;dummy;dummy=dummy->next)
       fprintf(out, "%s %s %li %li %li %li %li %i %i %i %i %i %i %i %i %i %i\n",
	       dummy->name, dummy->channel, dummy->time, dummy->kicktime,
	       dummy->bantime, dummy->deoptime, dummy->killtime, dummy->bannum, dummy->deopnum,
	       dummy->kicknum, dummy->totmsg, dummy->msgnum, dummy->totop,
	       dummy->totban, dummy->totdeop, dummy->totkick, dummy->totkill);
     fclose(out);
     return (TRUE);
}

int write_userlist( char *filename )
{
	UserStruct *dummy;
	time_t      T;
	FILE    *list_file;

	if( ( list_file = fopen( filename, "w" ) ) == NULL )
		return( FALSE );
 
	T = time( ( time_t *) NULL);

	fprintf( list_file, "#############################################\n" );
	fprintf( list_file, "## %s -- Userlist\n", filename );
	fprintf( list_file, "## Created: %s", ctime( &T ) );
	fprintf( list_file, "#############################################\n" );

	for( dummy = UserList; dummy; dummy = dummy->next )
		fprintf( list_file, " %40s %3d %1d %1d\n",
			dummy->userhost, dummy->level, dummy->isaop, dummy->protlevel);
	fprintf( list_file, "# End of %s\n", filename );
	fclose( list_file );
	return( TRUE );
}

int write_shitlist( char *filename )
{
	ShitStruct *dummy;
	time_t      T;
	FILE *list_file;
	ListStruct *channel;

	if( ( list_file = fopen( filename, "w" ) ) == NULL )
		return( FALSE );

	T = time( ( time_t *) NULL);

	fprintf( list_file, "#############################################\n" );
	fprintf( list_file, "## %s -- Shitlist\n", filename );
	fprintf( list_file, "## Created: %s", ctime( &T ) );
	fprintf( list_file, "#############################################\n" );

	for( dummy = ShitList; dummy; dummy = dummy->next )
	{
	  fprintf( list_file, " %40s %3d ", dummy->userhost, dummy->level);
	  for (channel=dummy->channels;channel;channel=channel->next)
	     if (channel->next)
		fprintf(list_file, "%s ", channel->name);
	     else        
		fprintf(list_file, "%s", channel->name);
	  fprintf(list_file, "\n");
	}
	fprintf( list_file, "# End of %s\n", filename );
	fclose( list_file );
	return( TRUE );
}

int  readln_from_a_file( FILE *stream, char *lin)
{
	char *p;

	do
		p = fgets( lin, MAXLEN, stream );
	while( ( p != NULL ) && ( *lin == '#') );

	if( p == NULL )
		return( FALSE );
	if (strchr(lin, '\n'))
	  *strchr(lin, '\n') = '\0';
        if (strchr(lin, '\r'))
	  *strchr(lin, '\r') = '\0';
	return( TRUE );
}

int  read_shitlist( char *filename )
{
	FILE  *fp;
	char  lin[MAXLEN];
	char  usrhost[MAXLEN];
	int   level;
	char *channels;        
	
	ShitStruct *dummy;


	if( ( fp = fopen( filename, "r" ) ) == NULL )
		return FALSE;

	for( dummy = ShitList; dummy; dummy = dummy->next )
	{
	     remove_all_from_list(&(dummy->channels));                
	     free(dummy);
	}
	ShitList = init_shitlist();

	while( readln_from_a_file( fp, lin) )
	{
		sscanf(lin, "%s %d\n", usrhost, &level);
		channels = strstr(lin, usrhost);
		if (channels)
		{
		  channels = strtok(channels, " ");
		  channels = strtok(NULL, " ");
		  channels = strtok(NULL, " ");
		}
		if (channels)
		   add_to_shitlist( usrhost, level, channels);
		else
		   add_to_shitlist( usrhost, level, "*");
	}
	fclose( fp );
	return( TRUE );
}

int  read_userlist( char *filename )
{
	FILE  *fp;
	char  lin[MAXLEN];
	char  usrhost[MAXLEN];
	int   level, aop, prot;
	UserStruct *dummy;


	if ((fp = fopen( filename, "r")) == NULL)
	  return FALSE;

	for( dummy = UserList; dummy; dummy = dummy->next )
		free(dummy);
	UserList = init_userlist();

	while( readln_from_a_file( fp, lin) )
	{
		sscanf(lin, "%s %d %d %d\n", usrhost, &level, &aop, &prot);
		add_to_userlist( usrhost, level, aop, prot);
	}
	fclose( fp );
	return( TRUE );
}



