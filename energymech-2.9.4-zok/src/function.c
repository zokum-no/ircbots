/*

    EnergyMech, IRC bot software
    Parts Copyright (c) 1997-2001 proton, 2002-2004 emech-dev

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
#include "config.h"

#define TOC " =\t\r\n"

static	char uhbuf[NUHLEN];		/* for find_userhost() */
static	char timebuf[MSGLEN];
static	char idlestr[MSGLEN];

static char *months[] =
{
	"Jan","Feb","Mar","Apr","May","Jun",
	"Jul","Aug","Sep","Oct","Nov","Dec"
};

static char *days[] =
{
	"Sun","Mon","Tue","Wed","Thu","Fri","Sat"
};

#ifndef STRICTAUTH
int is_netsplit(char *quitmsg)
{
	char	*pt;
	int	dot1,dot2,spc;

	if (!strcmp("*.net *.split",quitmsg)) /* New Undernet is LAME! */
		return(TRUE);

	if (!strncmp(quitmsg,"Quit: ",6)) /* DALnet et-al */
		quitmsg += 6;

	dot1 = dot2 = spc = 0;

	for(pt=quitmsg;*pt;pt++)
	{
		if (*pt == '.')
			dot2++;
		if (*pt == ' ')
		{
			if (!spc)
			{
				dot1 = dot2;
				dot2 = 0;
				spc++;
			}
			else
				return(FALSE);	/* more than one space */
		}
	}
	if (spc && dot1 && dot2)
		return(TRUE);
	return(FALSE);
}
#endif /* STRICTAUTH */

char *formatgreet(char *format, char *insert)
{
	static	char mem[MSGLEN];
	char	*dst,*src;

	if (!format || !insert)
		return(NULL);

	dst = mem;
	while(*format)
	{
		if (*format == '%')
		{
			format++;
			switch(*format)
			{
			case 'n':
				src = insert;
				while(*src)
					*(dst++) = *(src++);
				format++;
				break;
			case '%':
				*dst = '%';
				break;
			default:
				*(dst++) = '%';
				*(dst++) = *(format++);
			}
		}
		else
			*(dst++) = *(format++);
	}
	*dst = 0;
	return(mem);
}

void update_auths(void)
{
	aTime	*cptr,*nxt;

	cptr = current->Auths;
	while(cptr)
	{
		nxt = cptr->next;
		if ((now - cptr->time) > AUTH_TIMEOUT)
		{
			if (is_present(getnick(cptr->name)))
				cptr->time = now;
			else
			{
				remove_time(&current->Auths,cptr);
			}
		}
		cptr = nxt;
	}
}

#ifdef SUPERDEBUG

void *MyMalloc(int size)
{
	aME	*mmep;
	aMEA	*mp;
	int	i;

	mmep = NULL;
	mp = mrrec;
	while(!mmep)
	{
		for(i=0;i<MRSIZE;i++)
		{
			if (mp->mme[i].area == NULL)
			{
				mmep = &mp->mme[i];
				break;
			}
		}
		if (!mmep)
		{
			if (mp->next == NULL)
			{
				mp->next = calloc(sizeof(aMEA),1);
				mmep = &mp->next->mme[0];
			}
			else
				mp = mp->next;
		}
	}

	if ((mmep->area = (void*)calloc(size,1)) == NULL)
	{
		do_debug(NULL,NULL,NULL,0);
		exit(1);
	}
	mmep->size = size;
	mmep->when = now;
	mmep->doer = mallocdoer;
	mallocdoer = NULL;
	return((void*)mmep->area);
}

void MyFree(char **mem)
{
	aME	*mmep;
	aMEA	*mp;
	int	i;

	if (!mem || !*mem)
		return;

	mmep = NULL;
	mp = mrrec;
	while(!mmep)
	{
		for(i=0;i<MRSIZE;i++)
		{
			if (mp->mme[i].area == *mem)
			{
				mmep = &mp->mme[i];
				break;
			}
		}
		if (!mmep)
		{
			if (mp->next == NULL)
			{
				debug("(MyFree) PANIC: MyFree(0x"mx_pfmt"); Unregistered memory block\n",(mx_ptr)*mem);
				exit(1);
			}
			mp = mp->next;
		}
	}

	mmep->area = NULL;
	mmep->size = 0;
	mmep->when = (time_t)0;
	free(*mem);
	*mem = NULL;
}

#else /* SUPERDEBUG */

void *MyMalloc(int size)
{
	void	*tmp;

	if ((tmp = (void*)calloc(size,1)) == NULL)
		exit(1);
	return((void*)tmp);
}

void MyFree(char **mem)
{
	if (mem && *mem)
	{
		free(*mem);
		*mem = NULL;
	}
}

#endif /* SUPERDEBUG */

char *mstrcpy(char **dest, char *src)
{
	if (src)
	{
		*dest = (char*)MyMalloc(strlen(src)+1);
		strcpy(*dest,src);
	}
	else
		*dest = NULL;
	return(*dest);
}

char *terminate(char *string, char *chars)
{
	char	*p;

	if (!string || !chars)
		return("");
	p = strpbrk(string,chars);
	if (p)
		*p = 0;
	return(string);
}

char *get_token(char **src, char *token_sep)
{
	char    *tok;

	if (!src || !*src || !**src)
		return(NULL);

	while(**src && strchr(token_sep,**src))
		(*src)++;

	if (**src)
		tok = *src;
	else
		return(NULL);

	*src = strpbrk(*src,token_sep);
	if (*src)
	{
		**src = 0;
		(*src)++;
		while(**src && strchr(token_sep,**src))
			(*src)++;
	}
	else
		*src = "";
	return(tok);
}

char *chop(char **src)
{
	char	*tok;

	if (!src || !*src || !**src)
		return(NULL);

	while(**src == ' ')
		(*src)++;

	if (**src)
		tok = *src;
	else
		return(NULL);

	while(**src && **src != ' ')
		(*src)++;

	if (**src)
	{
		**src = 0;
		(*src)++;
		while(**src == ' ')
			(*src)++;
	}
	return(tok);
}

int Strcasecmp(const char *s1, const char *s2)
{
	Uchar	*p1 = (Uchar*)s1;
	Uchar	*p2 = (Uchar*)s2;
	int	ret;
	Uchar	c1;

	if (p1 == p2)
		return(0);
	if (!p1 || !p2)
		return(1);

	for(; !(ret = (c1 = tolowertab[*p1]) - tolowertab[*p2]); p1++, p2++)
		if (c1 == 0)
			break;

	return(ret);
}

/*
 *  This code might look odd but its optimized for size,
 *  so dont change it!
 */
char *Strcat(char *dst, char *src)
{
	while(*(dst++))
		;
	--dst;
	while((*(dst++) = *(src++)) != 0)
		;
	return(dst-1);
} 

char *getnick(char *nuh)
{
	static	char nick[80];
	char	*tp = nick;

#ifdef LINKING
	if (*nuh == '$')
	{
		while(*nuh != '@')
			nuh++;
		nuh++;
	}
#endif /* LINKING */
	while(*nuh && (*nuh != '!'))
	{
		*tp = *nuh;
		nuh++;
		tp++;
	}
	*tp = 0;
	return(nick);
}

char *gethost(char *nuh)
{
	static	char host[80];
	char	*tp;

	tp = nuh;
	while(*tp)
	{
		if (*tp == '@')
			nuh = tp + 1;
		tp++;
	}
	tp = host;
	while(*nuh)
	{
		*tp = *nuh;
		nuh++;
		tp++;
	}
	*tp = 0;
	return(host);
}

char *getuh(char *nuh)
{
	static	char uh[80];
	char	*tp;

	tp = nuh;
	while(*tp)
	{
		if (*tp == '!')
		{
			nuh = tp + 1;
			/*
			 *  We have to grab everything from the first '!' since some
			 *  braindamaged ircds allow '!' in the "user" part of the nuh
			 */
			break;
		}
		tp++;
	}
	tp = uh;
	while(*nuh)
	{
		*tp = *nuh;
		nuh++;
		tp++;
	}
	*tp = 0;
	return(uh);
}

char *time2str(time_t when)
{
	struct	tm *btime;

	if (!when)
		return(NULL);

	btime = localtime(&when);
	if (sprintf(timebuf,"%-2.2d:%-2.2d:%-2.2d %s %-2.2d %d",
	    btime->tm_hour,btime->tm_min,btime->tm_sec,
	    months[btime->tm_mon],btime->tm_mday,btime->tm_year+1900))
		return(timebuf);
	return(NULL);
}

char *time2away(time_t when)
{
	struct	tm *btime;
	char	ampm;

	if (!when)
		return(NULL);

	btime = localtime(&when);
	if (btime->tm_hour < 12)
	{
		if (btime->tm_hour == 0)
			btime->tm_hour = 12;
		ampm = 'a';
	}
	else
	{
		if (btime->tm_hour != 12)
			btime->tm_hour -= 12;
		ampm = 'p';
	}

	if (sprintf(timebuf,"%i:%-2.2d %cm %s %s %d",
		btime->tm_hour,btime->tm_min,ampm,days[btime->tm_wday],
		months[btime->tm_mon],btime->tm_mday))
	{
		return(timebuf);
	}
	return(NULL);
}

char *time2medium(time_t when)
{
	struct	tm *btime;

	btime = localtime(&when);
	if (when && (sprintf(timebuf,"%-2.2d:%-2.2d",btime->tm_hour,btime->tm_min)))
		return(timebuf);
	return(NULL);
}

char *time2small(time_t when)
{
	struct	tm *btime;

	btime = localtime(&when);
	if (when && (sprintf(timebuf,"%s %-2.2d",months[btime->tm_mon],btime->tm_mday)))
		return(timebuf);
	return(NULL);
}

char *idle2str(time_t when, int small)
{
	int	d,h,m,s;

	d = when / 86400;
	h = (when -= d * 86400) / 3600;
	m = (when -= h * 3600) / 60;
	s = when % 60;

	if (small)
		sprintf(idlestr,"%d d, %d h, %d m, %d s",d,h,m,s);
	else
		sprintf(idlestr,"%d day%s %-2.2d:%-2.2d:%-2.2d",d,EXTRA_CHAR(d),h,m,s);
	return(idlestr);
}

char *my_stristr(char *s1, char *s2)
{
	char	n1[HUGE],n2[HUGE];
	char	*temp,*ptr1,*ptr2;
	char	*save = s1;

	if (!s1 || !s2)
		return(NULL);
	ptr1 = n1;
	ptr2 = n2;
	while(*s1)
		*(ptr1++) = toupper(*(s1++));
	while(*s2)
		*(ptr2++) = toupper(*(s2++));
	*ptr1 = 0;
	*ptr2 = 0;
	temp = strstr(n1,n2);
	if (temp)
		return(save + (temp-n1));
	return(NULL);
}

char *find_userhost(char *from, char *nick)
{
	char	*uh;

	if (!nick)
		return(NULL);

	strcpy(uhbuf,nick);

	if (strchr(nick,'!'))
		return(nick);

	if (strchr(nick,'@'))
	{
		strcpy(uhbuf,"*!");
		if (nick[0] != '*')
			Strcat(uhbuf,"*");
		Strcat(uhbuf,nick);
		return(uhbuf);
	}
	uh = find_nuh(nick);
	if (!uh)
	{
		if (from)
			no_info(from,nick);
		return(NULL);
	}
	return(strcpy(uhbuf,uh));
}

void no_info(char *who, char *nick)
{
	send_to_user(who,TEXT_NOINFOFOR,nick);
}

void mass_action(char *who, char *channel)
{
	aChanUser *fromuser;
	aChan	*Chan;
	char	*temp;
	int	mpl;

	if ((Chan = find_channel(channel,CH_ACTIVE)) == NULL)
		return;
	if (!get_int_varc(Chan,TOGMASS_VAR))
		return;

	fromuser = Chan->users;
	while(fromuser)
	{
		if (!Strcasecmp(who,get_nuh(fromuser)))
			break;
		fromuser = fromuser->next;
	}
	if (!fromuser)
		return;

	temp = format_uh(who,1);
	if (((mpl = get_int_varc(Chan,SETMPL_VAR)) >= 3) && get_int_varc(Chan,TOGAS_VAR))
	{
		if (!is_user(who,channel) && !(usermode(channel,getnick(who)) & CU_CHANOP))
		{
			add_to_shitlist(temp,2,channel,"Auto-Shit",TEXT_MASSMODES,
				now,now + 86400);
		}
	}
	if (mpl >= 2)
	{
		if ((!(fromuser->flags & CU_DEOPPED)) || (!(fromuser->flags & CU_BANNED)))
		{
			deop_ban(channel,getnick(who),temp);
			fromuser->flags |= (CU_DEOPPED|CU_BANNED);
		}
	}
	if (mpl >= 1)
	{
		if (!(fromuser->flags & CU_KICKED))
		{
			sendkick(channel,getnick(who),TEXT_GETOUTMASSMODES);
			fromuser->flags |= CU_KICKED;
		}
	}
}

int shit_action(char *who, aChan *Chan)
{
	aUser	*Shit;
	char	*reason,*channel;

	if (!get_int_varc(Chan,TOGSHIT_VAR) || !Chan->bot_is_op)
		return(FALSE);
	channel = Chan->name;
	if (is_user(who,channel))
		return(FALSE);
	if ((Shit = find_shit(who,channel)) == NULL)
		return(FALSE);
	if (!Shit->parm.shitmask || !*Shit->parm.shitmask)
		return(FALSE);
	if (Shit->access > 1)
	{
		reason = get_shitreason(Shit);
		deop_ban(channel,getnick(who),Shit->parm.shitmask);
		sendkick(channel,getnick(who),"%s",reason);
		return(TRUE);
	}
	else
	if ((Shit->access == 1) && is_opped(getnick(who),channel))
	{
		sendmode(channel,"-o %s",getnick(who));
	}
	return(FALSE);
}

void prot_action(char *from, aChan *Chan, char *protnuh)
{
	aChanUser *ChanUser,*ChanUser2;
	aUser	*User;
	char	saveprotnuh[MSGLEN];
	char	*banuh;
	int	level;

	if (!get_int_varc(Chan,TOGPROT_VAR) || !Chan->bot_is_op)
		return;

	banuh = getnick(from);
	ChanUser2 = Chan->users;
	while(ChanUser2)
	{
		if (!Strcasecmp(banuh,ChanUser2->nick))
			break;
		ChanUser2 = ChanUser2->next;
	}
	if (!ChanUser2)
		return;

	if (strchr(protnuh,'!') == NULL)
	{
		ChanUser = Chan->users;
		while(ChanUser)
		{
			if (!Strcasecmp(protnuh,ChanUser->nick))
			{
				strcpy(saveprotnuh,get_nuh(ChanUser));
				break;
			}
			ChanUser = ChanUser->next;
		}
	}
	else
		strcpy(saveprotnuh,protnuh);

	User = find_user(saveprotnuh,Chan->name);
	level = get_protuseraccess(saveprotnuh,Chan->name);
	if (User && (User->prot > level))
	{
		level = User->prot;
		strcpy(saveprotnuh,User->name);
	}
	if (!level)
		return;

	User = find_user(from,Chan->name);
	if (!User || !(User->prot || (User->access >= ASSTLEVEL)))
	{
		banuh = format_uh(from,1);
		if ((level >= 4) && (!(ChanUser2->flags & CU_BANNED)))
		{
			ChanUser2->flags |= CU_BANNED;
			deop_ban(Chan->name,getnick(from),banuh);
		}
		if ((level >= 3) && (!(ChanUser2->flags & CU_KICKED)))
		{
			ChanUser2->flags |= CU_KICKED;
			sendkick(Chan->name,getnick(from),TEXT_ISPROTECTED,saveprotnuh);
		}
		if ((level == 2) && (!(ChanUser2->flags & CU_DEOPPED)))
		{
			ChanUser2->flags |= CU_DEOPPED;
			sendmode(Chan->name,"-o %s",getnick(from));
		}
	}
}

#ifdef MULTI

int is_localbot(char *nuh)
{
	aBot	*bot;
	char	*nick;

#ifdef DEBUG
	if (!nuh)
	{
		debug("(is_localbot) PANIC: called with a NULL arg!\n");
		*nuh = 0;	/* cause SEGV */
	}
#endif /* DEBUG */

	nick = getnick(nuh);
	for(bot=botlist;bot;bot=bot->next)
	{
		if (!Strcasecmp(nick,bot->nick))
			return(TRUE);
	}
	return(FALSE);
}

#endif /* MULTI */

char *get_channel(char *to, char **rest)
{
	char	*channel;

	if (*rest && ischannel(*rest))
	{
		channel = chop(rest);
	}
	else
	{
		if (!ischannel(to) && current->CurrentChan)
			channel = current->CurrentChan->name;
		else
			channel = to;
	}
	return(channel);
}

char *get_channel2(char *to, char **rest)
{
	char	*channel;

	if (*rest && (**rest == '*' || ischannel(*rest)))
	{
		channel = chop(rest);
	}
	else
	{
	if (!ischannel(to) && current->CurrentChan)
			channel = current->CurrentChan->name;
	else
			channel = to;
	}
	return(channel);
}

char *nick2uh(char *from, char *userhost, int type)
{
	static	char buffer[HUGE];
	char	*temp;

	if (!userhost || !*userhost)
		return(NULL);
	if (strchr(userhost,'!') && strchr(userhost,'@'))
		return(userhost);
	strcpy(buffer,userhost);
	if (!strchr(userhost,'!') && !strchr(userhost,'@'))
	{
		temp = find_userhost(from,userhost);
		if (!temp || !*temp)
			return(NULL);
		if (type)
			temp = format_uh(temp,type);
		strcpy(buffer,temp);
	}
	else
	{
		strcpy(buffer,"*!");
		if (!strchr(userhost,'@'))
			Strcat(buffer,"*");
		Strcat(buffer,userhost);
	}
	return(buffer);
}

/*
 *  type   output
 *  ~~~~   ~~~~~~
 *  0,1    *!*user@*.host.com
 *  2      *!*@*.host.com
 */
char *format_uh(char *userhost, int type)
{
	static	char banmask[NUHLEN];
	char	tmpmask[NUHLEN];
	char	*u,*h;

	if (strchr(userhost,'*'))
	{
		strcpy(banmask,userhost);
		return(banmask);
	}

	h = tmpmask;
	strcpy(h,userhost);

	get_token(&h,"!");
	u = get_token(&h,"@");

	if (!*h)
	{
		strcpy(banmask,userhost);
		return(banmask);
	}	

	if (u && (type < 2))
	{
		if ((type = strlen(u)) > 9)
			u += (type - 9);
		else
		if (*u == '~')
			u++;
	}

	sprintf(banmask,"*!*%s@%s",(u) ? u : "", cluster(h));
	return(banmask);
}

void deop_ban(char *channel, char *nick, char *nuh)
{
	if (!channel || !nick || !nuh)
		return;
	sendmode(channel,"-o+b %s %s",nick,format_uh(nuh,1));
}

void deop_siteban(char *channel, char *nick, char *nuh)
{
	if (!channel || !nick || !nuh)
		return;
	if (strchr(nuh,'*'))
		sendmode(channel,"-o+b %s %s",nick,nuh);
	else
		sendmode(channel,"-o+b %s %s",nick,format_uh(nuh,2));
}

int isnick(char *nick) 
{
	Uchar	*p;

	p = (Uchar*)nick;
	if ((attrtab[*p] & FNICK) != FNICK)
		return(FALSE);

	while(*p)
	{
		if ((attrtab[*p] & NICK) != NICK)
			return(FALSE);
		p++;
	}
	return(TRUE);
}

char *cluster(char *hostname)
{
	static	char result[1024];
	char	temphost[255];
	char	*host,*tmp;
	char	num;

	if (!hostname)
		return(NULL);

	host = temphost;
	strcpy(result, "");

	if (strchr(hostname, '@'))
	{
		strcpy(result, hostname);
		*strchr(result, '@') = '\0';
		Strcat(result, "@");
		hostname = strchr(hostname, '@');
		hostname++;
	}
	strcpy(host,hostname);

	if (*host && strchr(host,':')) {
		strcpy(result,host);
		return(result);
	}

	if (*host && isdigit((int)*(host+strlen(host)-1)))
	{
		tmp = host;
		for(num=0;num<2;num++)
			tmp = strchr(tmp,'.') + 1;
		*tmp = 0;
		Strcat(result,host);
		Strcat(result,"*.*");
	}
	else
	{
		num = 1;
		tmp = right(host,3);
		if (Strcasecmp(tmp,"com") && Strcasecmp(tmp,"edu") &&
		    (my_stristr(host,"com") || my_stristr(host,"edu")))
			num = 2;
		while(host && *host && (numchar(host,'.') > num))
		{
			if ((host = strchr(host,'.')) == NULL)
				return(NULL);
			host++;
		}
		Strcat(result,"*");
		if (Strcasecmp(host,temphost))
			Strcat(result,".");
		Strcat(result,host);
	}
	return(result);
}

int numchar(char *string, int ch)
{
	int	num = 0;
	char	c;

	c = (char)ch;
	while(*string)
	{
		if (tolower(*string) == tolower(c))
			num++;
		string++;
	}
	return(num);
}

char *right(char *string, int num)
{
	if (strlen(string) < num)
		return(string);
	return(string+strlen(string)-num);
}

int capslevel(char *string)
{
	int	len,tot;

	tot = 0;
	if (!string || !*string)
		return(0);
	len = strlen(string);
	while(*string)
	{
		if (isalpha((int)*string) && isupper((int)*string))
			tot++;
		string++;
	}
	return((100*tot)/len);
}

/*
 *  energymech password encryption
 */

char	pctab[] =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuwvxuz0123456789+-";

#define CIPHER(a1,a2,a3,a4,b1,b2,b3,b4) \
{					\
	a2 =  a2 ^  a1;			\
	b2 =  b2 ^  b1;			\
	a3 =  a2 ^  a1;			\
	b3 =  b2 ^  b1;			\
	b3 >>= 2;			\
	b3 |= ((a3 & 3) << 30);		\
	a3 >>= 2;			\
	a2 =  a3 ^  a2;			\
	b2 =  b3 ^  b2;			\
	a4 = ~a4 ^  a2;			\
	b4 = -b4 ^  b2;			\
	a2 =  a4 ^ ~a2;			\
	b2 =  b4 ^ -b2;			\
	b1 >>= 1;			\
	b1 |= ((a1 & 1) << 31);		\
	a1 >>= 1;			\
}

char *cipher(char *arg)
{
	static	char res[40];
	Ulong	B1a,B2a,B3a,B4a;
	Ulong	B1b,B2b,B3b,B4b;
	Uchar	*ptr;
	Ulong	R1;
	int	i;

	if (!arg || !*arg)
		return(NULL);
	
	B1a = B2a = B3a = B4a = 0;
	B1b = B2b = B3b = B4b = 0;
	ptr = arg;

	while(*ptr)
	{
		R1 = *ptr;
		for(i=0;i<8;i++)
		{
			if (R1 & 1)
			{
				B1a |= 0x80008000;
				B1b |= 0x80008000;
			}
			R1  >>= 1;
			CIPHER(B1a,B2a,B3a,B4a,B1b,B2b,B3b,B4b);
		}
		ptr++;
	}
	while((B1a) || (B1b))
	{
		CIPHER(B1a,B2a,B3a,B4a,B1b,B2b,B3b,B4b);
	}

	memset(res,0,sizeof(res));
	i = 0;
	for(i=0;i<10;i++)
	{
		res[i] = pctab[(B4b & 0x3f)];
		B4b >>= 6;
		B4b |= ((B4a & 0x3f) << 26);
		B4a >>= 6;
	}
	return(res);
}

void makepass(char *encoded, char *plain)
{
	strcpy(encoded,cipher(plain));
}

int passmatch(char *plain, char *encoded)
{
	return(!Strcasecmp(cipher(plain),encoded));
}

int check_for_number(char *from, char *string)
{
	if (!string || !isdigit((int)*string))
	{
		send_to_user(from, TEXT_EXPECTNUM);
		return 1;
	}
	return 0;
}

char *findstruct(char *base, int num, int sz, int ident)
{
	MLStub	*s;
	int	i;

	s = (MLStub*)base;
	for(i=0;i<num;i++)
	{
		if ((s->used) && (s->ident == ident))
			return((char*)s);
		s += sz;
	}
	return(NULL);
}

char *addstruct(char **base, char *new, int *num, int sz)
{
	char	*pt;
	int	i;


	if (*base == NULL)
	{
		set_mallocdoer(addstruct);
		pt = (char*)MyMalloc(sz);
		memcpy(pt,new,sz);
		*base = pt;
		*num = 1;
		return(pt);
	}
loop:
	pt = *base;
	for(i=0;i<*num;i++)
	{
		if (*pt == 0)
		{
			memcpy(pt,new,sz);
			return(pt);
		}
		pt += sz;
	}
	set_mallocdoer(addstruct);
	pt = (char*)MyMalloc(sz * (*num+1));
	if (!pt)
		mechexit(1);
	memcpy(pt,*base,((*num) * sz));
	MyFree((char**)base);
	*base = pt;
	(*num)++;
	goto loop;
}

int *Atoi(char *arg)
{
	static	int res;
	int	neg;

	if (!arg || !*arg)
		return(NULL);

	neg = FALSE;
	if (*arg == '-')
		neg = TRUE, arg++;

	res = 0;
	while(*arg)
	{
		res = (res << 1) + (res << 3);
		if (attrtab[(Uchar)*arg] & NUM)
			res += *(arg++) - '0';
		else
			return(NULL);
	}
	if (neg)
		res = -res;
	return(&res);
}

char	__escape_string__[HUGE];

char *escape_string(char *arg)
{
	char	*pt,*pp;

	pp = __escape_string__;
	pt = arg;
	while(*pt)
	{
		switch(*pt)
		{
		case '\r':
		case '\n':
		case '\\':
		case ' ':
			*pp = '\\';
			pp++;
		}
		switch(*pt)
		{
		case '\r':
			*pp = 'r';
			pp++;
			break;
		case '\n':
			*pp = 'n';
			pp++;
			break;
		case ' ':
			*pp = 's';
			pp++;
			break;
		case '\\':
			*pp = '\\';
			pp++;
			break;
		default:
			*pp = *pt;
			pp++;
		}
		pt++;
	}
	*pp = 0;
	return(__escape_string__);
}

char *unescape_string(char *arg)
{
	char	*pt,*pp;

	pp = pt = arg;
	while(*pt)
	{
		if (*pt == '\\')
		{
			pt++;
			switch(*pt)
			{
			case 'n':
				*pp = '\n';
				break;
			case 'r':
				*pp = '\r';
				break;
			case 's':
				*pp = ' ';
				break;
			default:
				*pp = *pt;
			}
			pt++;
			pp++;
		}
		else
		{
			*pp = *pt;
			pt++;
			pp++;
		}
	}
	*pp = 0;
	return(arg);
}

#ifdef ALIASES
void add_alias(char *from, char *data, int type)
{
	char	*org,*new,*pt;
	int	i;

	org = get_token(&data,TOC);
	if ((new = get_token(&data,TOC)) == NULL) 
		return;
 
	pt = new;
	while(*pt)
	{
		*pt = toupper(*pt);
		pt++;
	}

	for(i=0;mcmd[i].name;i++)
	{
		if (!Strcasecmp(mcmd[i].name,new))
		{
			if (type)
				send_to_user(from,"Alias replacing original command (ignored)");
			else
				printf("init: Alias replacing original command (ignored)\n");
			return;
		}
		if (!Strcasecmp(mcmd[i].alias,new))
		{
			if (type)
				send_to_user(from,"Alias %s already exists (ignored)\n",new);
			else
				printf("init: Alias %s already exists (ignored)\n",new);
			return;
		}
	}

	i = 0;
	while(mcmd[i].name)
	{
		if (!Strcasecmp(mcmd[i].name,org))
		{       
			if (mcmd[i].alias)
				MyFree(&mcmd[i].alias);
			set_mallocdoer(add_alias);
			mstrcpy(&mcmd[i].alias,new);
			if (type) send_to_user(from,"Alias %s added.\n",new);
			return;
		}
		i++;
	}
	if (type) send_to_user(from,"Could not find function %s.\n",org);
}
#endif

