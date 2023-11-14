/*
 * userhost.c - implementation of USERHOST <nick> by using a queue
 */

UHQ *is_in_userhostq(l_list, nickname)
UHQ **l_list;
char *nickname;
{
	UHQ *user;

	for (user = *l_list; user; user = user->next)
		if (!my_stricmp(nickname, user->nick)
		  return user;
	return NULL;
}

UHQ **init_userhostq(void)
{
	UHQ **l_list;

	l_list=(UHQ **) malloc (sizeof(*l_list));
	*l_list=NULL;
	return(l_list);
}

int add_to_userhostq(l_list, nick, to, from,
ULS **l_list;
char *uh;
int level;
int aop;
int prot;
char *chan;
char *password;
{
	ULS *New_user;

	if (*chan = '*')
	  remove_from_userlist(l_list, uh, "*");
	if (New_user = is_in_userlist(l_list, uh, chan))
	  return FALSE;
	if (!(New_user = (ULS *) malloc (sizeof(*New_user))))
	  return FALSE;
	mstrcpy(&New_user->userhost, uh);
	New_user->access = level;
	New_user->aop    = aop ? -1 : 0;
	New_user->prot   = (prot > MAXPROTLEVEL) ? 5 : (prot < 0) ? 0 : prot;
	mstrcpy(&New_user->channel, chan);
	mstrcpy(&New_user->password, password);
	New_user->next = *l_list;
	*l_list = New_user;
}

int remove_from_userlist(l_list, userhost, chan)
ULS **l_list;
char *userhost;
char *chan;
{
	ULS **old;
	ULS *dummy;

	if (!(dummy = is_in_userlist(l_list, userhost, chan)))
		return FALSE;

	while (dummy = is_in_userlist(l_list, userhost, chan))
	{
		for (old = l_list; *old; old = &(*old)->next)
			if (*old == dummy)
			{
				*old = dummy->next;
				free(dummy->userhost);
				free(dummy->channel);
				free(dummy->password);
				free(dummy);
			}
	}
	return TRUE;
}

void delete_userlist(l_list )
ULS **l_list;
{
	ULS *dummy;
	ULS *next;

	dummy = *l_list;
	while (dummy)
	{
		next = dummy->next;
		remove_from_userlist(l_list, dummy->userhost, dummy->channel);
		dummy = next;
	}
}







