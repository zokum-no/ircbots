/*

    EnergyMech, IRC bot software
    Copyright (c) 2001 proton, 2002-2004 emech-dev 

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
#define TRIVIA_C
#include "config.h"

#ifdef TRIVIA

#define STRCHR                  strchr                                                                                       
#define STREND(x)               STRCHR(x,0)

#define NEWFILEMODE		0644

#define SIZEOF_STRP   		(sizeof(void*) + sizeof(char))

#define triv_qchar		'*'
#define triv_qdelay		30
#define MATCH_ALL		"*"

#define TRIV_WAIT_QUESTION	0
#define TRIV_HINT_TWO		1
#define TRIV_HINT_THREE		2
#define TRIV_NO_ANSWER		3

#define TRIV_HINT_DELAY		15

#define TRIV_HINT_DELAY_STR1	"15"
#define TRIV_HINT_DELAY_STR2	"30"

#define TRIV_METACHARS		" .,-'%&/?!:;\""

#define DAY_IN_SECONDS		(24*60*60)
#define WEEK_IN_SECONDS		(7*24*60*60)

aTrivScore *triv_scorelist;
aTrivScore *triv_lastwinner;
aChan *triv_chan = NULL;
aStrp *triv_answers = NULL;
time_t triv_ask_time;
time_t triv_next_time = 0;
time_t triv_weektop10;
int triv_mode;
int triv_score;
int triv_streak;
int triv_halt_flag;

/*
 *  be wary, this is not a normal upper-to-lower table...
 */
const Uchar nickcmptab[256] =
{
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
	0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
	' ',  0x00, '"',  '#',  '$',  '%',  '&',  0x27,		/* <-- observe! the '!' changed to 0x00 */
	'(',  ')',  '*',  '+',  ',',  '-',  '.',  '/',
	'0',  '1',  '2',  '3',  '4',  '5',  '6',  '7',
	'8',  '9',  ':',  ';',  '<',  '=',  '>',  '?',
	'@',  'a',  'b',  'c',  'd',  'e',  'f',  'g',
	'h',  'i',  'j',  'k',  'l',  'm',  'n',  'o',
	'p',  'q',  'r',  's',  't',  'u',  'v',  'w',
	'x',  'y',  'z',  '{',  '|',  '}',  '~',  '_',
	'`',  'a',  'b',  'c',  'd',  'e',  'f',  'g',
	'h',  'i',  'j',  'k',  'l',  'm',  'n',  'o',
	'p',  'q',  'r',  's',  't',  'u',  'v',  'w',
	'x',  'y',  'z',  '{',  '|',  '}',  '~',  0x7f,
	0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
	0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
	0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97,
	0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
	0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
	0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,
	0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7,
	0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
	0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7,
	0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,
	0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7,
	0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf,
	0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7,
	0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
	0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7,
	0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff
};

int a2i(char *anum)
{
	int	res,neg;

	errno = EINVAL;

	if (!anum || !*anum)
		return(-1);

	neg = (*anum == '-') ? 1 : 0;
	anum += neg;

	res = 0;
	while(*anum)
	{
		res *= 10;
		if (attrtab[(Uchar)*anum] & NUM)
			res += *(anum++) - '0';
		else
			return(-1);
	}
	errno = 0;
	return((neg) ? -res : res);
}

int nickcmp(const Uchar *p1, const Uchar *p2)
{
	int	ret;
	int	c;

	if (p1 != p2)
	{
		while(!(ret = nickcmptab[*(p1++)] - (c = nickcmptab[*(p2++)])) && c)
			;
		return(ret);
	}
	return(0);
}

/*
 *
 *  scorings
 *
 */

void trivia_week_toppers(void)
{
	char	triv_str[MSGLEN];
	aTrivScore *chosen[10];
	aTrivScore *su;
	char	*p;
	int	week;
	int	i,x;

	chosen[0] = NULL;
	week = (now + (3 * DAY_IN_SECONDS)) / WEEK_IN_SECONDS;

	for(su=triv_scorelist;su;su=su->next)
	{
		if (su->week_nr != week)
			continue;
		for(i=0;i<10;i++)
		{
			if (!chosen[i] || (chosen[i]->score_wk < su->score_wk))
			{
				for(x=8;x>=i;x--)
					chosen[x+1] = chosen[x];
				chosen[i] = su;
				break;
			}
		}
	}

	if (chosen[0])
	{
		p = triv_str;
		for(i=0;i<10;i++)
		{
			if (!chosen[i])
				break;
			if (i)
			{
				*(p++) = ' ';
				*(p++) = ' ';
			}
			sprintf(p,"#%i: %s (%ipts)",i+1,chosen[i]->nick,chosen[i]->score_wk);
			p = STREND(p);
		}
		to_server("PRIVMSG %s :" TEXT_TRV_WKTOP10 " %s\n",triv_chan->name,triv_str);
	}
}

/*
 *
 *
 *
 */

void hint_one(void)
{
	char	triv_str[MSGLEN];
	char	*src,*dst;

	sprintf(triv_str,"PRIVMSG %s :" TEXT_TRV_HINT1,triv_chan->name);
	dst = STREND(triv_str);
	src = triv_answers->p;
	while(*src)
	{
		if (STRCHR(TRIV_METACHARS,*src))
			*(dst++) = *src;
		else
			*(dst++) = triv_qchar;
		src++;
	}
	*dst = 0;
	to_server(TEXT_TRV_QSCORE,triv_str,triv_score);
}

void hint_two(void)
{
	char	triv_str[MSGLEN];
	char	*src,*dst;
	int	n;

	sprintf(triv_str,"PRIVMSG %s :" TEXT_TRV_HINT2,triv_chan->name);
	dst = STREND(triv_str);
	src = triv_answers->p;

	n = a2i(src);
	if (!errno)
	{
		if (n >   99 && *src) *(dst++) = *(src++);
		if (n >  999 && *src) *(dst++) = *(src++);
		if (n > 9999 && *src) *(dst++) = *(src++);
	}
	else
	{
		n = strlen(src);
		if (n > 2 && *src) *(dst++) = *(src++);
		if (n > 4 && *src) *(dst++) = *(src++);
		if (n > 6 && *src) *(dst++) = *(src++);
	}

	while(*src)
	{
		if (STRCHR(TRIV_METACHARS,*src))
			*(dst++) = *src;
		else
			*(dst++) = triv_qchar;
		src++;
	}
	*dst = 0;
	to_server("%s   " TRIV_HINT_DELAY_STR2 TEXT_TRV_SECONDSLEFT,triv_str);
}

void hint_three(void)
{
	char	triv_str[MSGLEN];
	char	*src,*dst;
	int	n;

	sprintf(triv_str,"PRIVMSG %s :" TEXT_TRV_HINT3,triv_chan->name);
	dst = STREND(triv_str);
	src = triv_answers->p;

	n = a2i(src);
	if (!errno)
	{
		if (n >   9 && *src) *(dst++) = *(src++);
		if (n >  99 && *src) *(dst++) = *(src++);
		if (n > 999 && *src) *(dst++) = *(src++);
	}
	else
	{
		n = strlen(src);
		if (n > 1 && *src) *(dst++) = *(src++);
		if (n > 3 && *src) *(dst++) = *(src++);
		if (n > 4 && *src) *(dst++) = *(src++);
	}

	while(*src)
	{
		if (STRCHR(TRIV_METACHARS "aeiouyAEIOUY",*src))
			*(dst++) = *src;
		else
			*(dst++) = triv_qchar;
		src++;
	}
	*dst = 0;
	to_server("%s   " TRIV_HINT_DELAY_STR1 TEXT_TRV_SECONDSLEFT,triv_str);
}

/*
 *
 *
 *
 */

void trivia_cleanup(void)
{
	aStrp	*ans;

	triv_mode = TRIV_WAIT_QUESTION;
	triv_next_time = now + triv_qdelay;
	while((ans = triv_answers))
	{
		triv_answers = ans->next;
		MyFree((char**)&ans);
	}
}

void trivia_check(char *channame, char *rest)
{
	aTrivScore 	*su;
	aChan	   	*Chan;
	aStrp		*ans;
	int		week;

	if ((Chan = find_channel(channame,CH_ACTIVE)) == NULL)
		return;
	
	if (Chan != triv_chan)
		return;

	for(ans=triv_answers;ans;ans=ans->next)
	{
		if (!Strcasecmp(ans->p,rest))
			goto have_answer;
	}
	return;

have_answer:
	week = (now + (3 * DAY_IN_SECONDS)) / WEEK_IN_SECONDS;

	for(su=triv_scorelist;su;su=su->next)
	{
		if (!nickcmp(su->nick,CurrentNick))
		{
			su->score_mo += triv_score;

			if (su->week_nr == week)
				su->score_wk += triv_score;
			else
			{
				if (su->week_nr == (week - 1))
					su->score_last_wk = su->score_wk;
				else
					su->score_last_wk = 0;
				su->week_nr = week;
				su->score_wk = triv_score;
			}
			break;
		}
	}
	if (!su)
	{
		set_mallocdoer(trivia_check);
		su = (aTrivScore*)MyMalloc(sizeof(aTrivScore) + strlen(CurrentNick));
		su->next = triv_scorelist;
		triv_scorelist = su;
		su->score_wk = su->score_mo = triv_score;
		su->score_last_wk = su->score_last_mo = 0;
		su->week_nr = week;
		su->month_nr = 0; /* fix this */
		strcpy(su->nick,CurrentNick);
	}

	to_server("PRIVMSG %s :" TEXT_TRV_ANSWERED,triv_chan->name,CurrentNick,triv_answers->p,
		(int)(now - triv_ask_time),triv_score);

	if (su == triv_lastwinner)
	{
		triv_streak++;
		if (su->score_wk == su->score_mo)
		{
			to_server("PRIVMSG %s :" TEXT_TRV_INAROWWEEK,triv_chan->name,CurrentNick,triv_streak,su->score_wk);
		}
		else
		{
			to_server("PRIVMSG %s :" TEXT_TRV_INAROWMONTH,triv_chan->name,CurrentNick,triv_streak,su->score_wk,su->score_mo);
		}
	}
	else
	{
		triv_lastwinner = su;
		triv_streak = 1;
		if (su->score_wk == su->score_mo)
		{
			to_server("PRIVMSG %s :" TEXT_TRV_WONQUES,triv_chan->name,CurrentNick,su->score_wk);
		}
		else
		{
			to_server("PRIVMSG %s :" TEXT_TRV_WONQUESMONTH,triv_chan->name,CurrentNick,su->score_wk,su->score_mo);
		}
	}

	trivia_cleanup();
}

void trivia_no_answer(void)
{
	to_server("PRIVMSG %s :" TEXT_TRV_TIMESUP,triv_chan->name,triv_answers->p);

	trivia_cleanup();
}

char *random_question(char *triv_rand)
{
	char	*p;
	off_t	sz;
	int	fd,ifd;
	int	n;
	struct
	{
		int	off;
        	int	sz;
	} entry;

	if ((fd = open(RANDTRIVIAFILE,O_RDONLY)) < 0)
		return(NULL);

	strcpy(triv_rand,RANDTRIVIAFILE);
	p = rindex(triv_rand,'e');
	*p = 'i';

	if ((ifd = open(triv_rand,O_RDONLY)) < 0)
		return(NULL);

	sz = lseek(ifd,0,SEEK_END);
	sz = sz / sizeof(entry);
	n = RANDOM(1,sz);
	n--;

	lseek(ifd,(n * sizeof(entry)),SEEK_SET);
	read(ifd,&entry,sizeof(entry));

	lseek(fd,entry.off,SEEK_SET);
	read(fd,triv_rand,entry.sz);
	triv_rand[entry.sz] = 0;

	close(fd);
	close(ifd);

	return(triv_rand);
}

void trivia_question(void)
{
	char	buffer[MSGLEN];
	aStrp	*ans,**pp;
	char	*question,*answer,*rest;

	if (triv_halt_flag)
	{
		to_server("PRIVMSG %s :" TEXT_TRV_STOPPED "\n",triv_chan->name);
		goto stop_trivia;
	}

	if ((rest = random_question(buffer)) == NULL)
	{
bad_question:
		to_server("PRIVMSG %s :" TEXT_TRV_BADQFILE "\n",triv_chan->name);
stop_trivia:
		trivia_cleanup();
		triv_chan = NULL;
		triv_next_time = 0;
		triv_halt_flag = FALSE;
		short_tv &= ~TV_TRIVIA;
		return;
	}

	question = get_token(&rest,MATCH_ALL);

	pp = &triv_answers;
	while((answer = get_token(&rest,MATCH_ALL)))
	{
		set_mallocdoer(trivia_question);
		*pp = ans = (aStrp*)MyMalloc(SIZEOF_STRP + strlen(answer));
		pp = &ans->next;
		strcpy(ans->p,answer);
	}
	*pp = NULL;

	if (triv_answers == NULL)
		goto bad_question;

	triv_score = (RANDOM(2,9) + RANDOM(2,10) + RANDOM(2,10)) / 3;
	triv_ask_time = now;

	if (now > (triv_weektop10 + 1200))
	{
		trivia_week_toppers();
		triv_weektop10 = now;
	}

	to_server("PRIVMSG %s :%s\n",triv_chan->name,question);
	hint_one();
}

void trivia_tick(void)
{
	aChan	*Chan;
	aBot	*bot;

	if (triv_next_time && (now >= triv_next_time))
	{
		for(bot=botlist;bot;bot=bot->next)
		{
			for(Chan=bot->Channels;Chan;Chan=Chan->next)
			{
				if (triv_chan == Chan)
				{
					current = bot;
					triv_next_time = now + TRIV_HINT_DELAY;
					switch(triv_mode)
					{
					case TRIV_WAIT_QUESTION:
						trivia_question();
						break;
					case TRIV_HINT_TWO:
						hint_two();
						break;
					case TRIV_HINT_THREE:
						hint_three();
						break;
					case TRIV_NO_ANSWER:
						trivia_no_answer();
						return; /* dont increment with triv_mode */
					}
					triv_mode++;
#ifdef DEBUG
					current = NULL;
#endif /* DEBUG */
					return;
				}
			}
		}
	}
}

/*
 *
 *  File operations
 *
 */

void write_triviascore(void)
{
	aTrivScore *su;
	int	fd;

	if (triv_scorelist)
	{
		if ((fd = open(TRIVIASCOREFILE,O_WRONLY|O_TRUNC|O_CREAT,NEWFILEMODE)) < 0)
			return;
		tofile(fd,"# nick score_wk score_mo score_last_wk score_last_mo\n");
		for(su=triv_scorelist;su;su=su->next)
		{
			tofile(fd,"%s %i %i %i %i %i %i\n",su->nick,su->score_wk,su->score_mo,
				su->score_last_wk,su->score_last_mo,su->week_nr,su->month_nr);
		}
		close(fd);
	}
}

int trivia_score_callback(char *rest)
{
	aTrivScore *su;
	char	*nick,*wk,*mo,*lwk,*lmo,*wnr,*mnr;
	int	score_wk,score_mo,score_last_wk,score_last_mo;
	int	week_nr,month_nr;
	int	err;

	if (*rest != '#')
	{
		nick = chop(&rest);
		wk   = chop(&rest);
		mo   = chop(&rest);
		lwk  = chop(&rest);
		lmo  = chop(&rest);
		wnr  = chop(&rest);
		mnr  = chop(&rest);

		if (mnr)
		{
			score_wk = a2i(wk);
			err  = errno;
			score_mo = a2i(mo);
			err += errno;
			score_last_wk = a2i(lwk);
			err += errno;
			score_last_mo = a2i(lmo);
			err += errno;
			week_nr = a2i(wnr);
			err += errno;
			month_nr = a2i(mnr);
			err += errno;

			if (!err)
			{
				set_mallocdoer(trivia_score_callback);
				su = (aTrivScore*)MyMalloc(sizeof(aTrivScore) + strlen(nick));

				su->next = triv_scorelist;
				triv_scorelist = su;

				su->score_wk = score_wk;
				su->score_mo = score_mo;
				su->score_last_wk = score_last_wk;
				su->score_last_mo = score_last_mo;
				su->week_nr = week_nr;
				su->month_nr = month_nr;

				strcpy(su->nick,nick);
			}
		}
	}
	return(FALSE);
}

void read_triviascore(void)
{
	aTrivScore *su;
	int	fd;

	if ((fd = open(TRIVIASCOREFILE,O_RDONLY)) < 0)
		return;

	while(triv_scorelist)
	{
		su = triv_scorelist;
		triv_scorelist = su->next;
		MyFree((char**)&su);
	}

	readline(fd,&trivia_score_callback);		/* readline closes fd */
}

/*
 *
 *
 *
 */

void do_trivia(char *from, char *to, char *rest, int cmdlevel)
{
	/*
	 *  on_msg checks CAXS + CARGS
	 */
	aChan	*Chan;
        char	*channel;
	int	uaccess;

	uaccess = max_userlevel(from);

	channel = get_channel2(to,&rest);
	if ((Chan = find_channel(channel,CH_ACTIVE)) == NULL)
	{
		if (uaccess) send_to_user(from,ERR_CHAN,to);
		return;
	}

	if (!Strcasecmp(rest,TEXT_TRV_CMDSTART))
	{
		if (triv_chan)
		{
			if (triv_chan == Chan)
			{
				triv_halt_flag = FALSE;
				return;
			}
			if (uaccess) send_to_user(from,TEXT_TRV_ACTIVE " %s!",
				(get_userlevel(from,triv_chan->name)) ? triv_chan->name : TEXT_TRV_ANOTHERCHAN);
			return;
		}
		to_server("PRIVMSG %s :" TEXT_TRV_STARTING "\n",Chan->name);
		triv_chan = Chan;
		triv_mode = TRIV_WAIT_QUESTION;
		triv_next_time = now + triv_qdelay;
		triv_weektop10 = now;
		triv_lastwinner = NULL;
		short_tv |= TV_TRIVIA;
		if (!triv_scorelist)
		{
			read_triviascore();
		}
	}
	else
	if (!Strcasecmp(rest,TEXT_TRV_CMDSTOP))
	{
		if (Chan == triv_chan)
		{
			if (Chan == triv_chan)
				to_server("PRIVMSG %s :" TEXT_TRV_SHUTDOWN "\n",Chan->name);
			triv_halt_flag = TRUE;
		}
		else
		{
			if (uaccess) send_to_user(from,TEXT_TRV_NOTRUNNING " %s!",
				(get_userlevel(from,channel)) ? channel : TEXT_TRV_ANOTHERCHAN);
			return;
		}
	}
}

void do_triv_wk10(char *from, char *to, char *rest, int cmdlevel)
{
	int	n,uaccess;

	if (triv_chan)
	{
		uaccess = get_userlevel(from,triv_chan->name);
		if (now > (triv_weektop10 + 300))
			n =  1;
		else
			n = uaccess;

		if (n)
		{
			trivia_week_toppers();
			if (!uaccess) triv_weektop10 = now;
		}
	}
}

#endif /* TRIVIA */
