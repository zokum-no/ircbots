/*

    EnergyMech, IRC bot software
    Parts Copyright (c) 1997-2001 proton, 2002-2003 emech-dev

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

int get_int_varc(aChan *channel, int which)
{
	if (!channel || IsGlobal(&current->DefaultVars[which]))
		return(current->DefaultVars[which].value);
	return(channel->Vars[which].value);
}

char *get_str_varc(aChan *channel, int which)
{
	if (!channel || IsGlobal(&current->DefaultVars[which]))
		return(current->DefaultVars[which].strvalue);
	return(channel->Vars[which].strvalue);
}

int get_int_var(char *name, int which)
{
	aChan	*Chan;

	if (!name || ((Chan = find_channel(name,CH_ANY)) == NULL))
		return(get_int_varc(NULL,which));
	return(IsGlobal(&Chan->Vars[which]) ? current->DefaultVars[which].value : Chan->Vars[which].value);
}

char *get_str_var(char *name, int which)
{
	aChan	*Chan;

	if (!name || ((Chan = find_channel(name,CH_ANY)) == NULL))
		return(get_str_varc(NULL,which));
	return(IsGlobal(&Chan->Vars[which]) ? current->DefaultVars[which].strvalue : Chan->Vars[which].strvalue);
}

void set_str_varc(aChan *channel, int which, char *value)
{
	char *temp;

	set_mallocdoer(set_str_varc);
	mstrcpy(&temp,value);
	if (channel->Vars[which].strvalue)
		MyFree((char **)&channel->Vars[which].strvalue);
	channel->Vars[which].strvalue = temp;
}

int find_var_name(char *name, int type)
{
	int	i;

	for(i=0;VarName[i].name;i++)
	{
		if (!Strcasecmp(name,VarName[i].name))
			if (current->DefaultVars[i].type & type)
				return(i);
	}
	return(-1);
}

void copy_vars(VarStruct *dst, const VarStruct *src)
{
	int	i;
	
	for(i=0;VarName[i].name;i++)
	{
		dst[i].type = src[i].type;
		dst[i].value = src[i].value;
		dst[i].min = src[i].min;
		dst[i].max = src[i].max;
		if (src[i].strvalue)
		{
			set_mallocdoer(copy_vars);
			mstrcpy(&dst[i].strvalue,src[i].strvalue);
		}
	}	
}

void free_strvars(VarStruct *vars)
{
	int	i;

	for(i=0;VarName[i].name;i++)
	{
		if (vars[i].strvalue)
			MyFree(&vars[i].strvalue);
	}
}

VarStruct *find_var(char *name, int which)
{
	aChan	*Chan;

	if ((Chan = find_channel(name,CH_ANY)) == NULL)
		return(NULL);
	return((VarStruct *)&Chan->Vars[which]);
}	

void do_set(char *from, char *to, char *rest, int cmdlevel)
{
	VarStruct *var;
	aChan	*temp;
	char	*chan,*setname,*value;
	int	which,num;

	chan = get_channel2(to,&rest);
	if (!(setname = chop(&rest)))
	{
		usage(from,C_SET);
		return;
	}
	if ((which = find_var_name(setname,INT_VAR+STR_VAR)) == -1)
	{
		send_to_user(from,TEXT_VARNOSUCH);
		return;
	}
	value = chop(&rest);
	var = &current->DefaultVars[which];
	num = 0;
	if (value && IsInt(var))
	{
		if (check_for_number(from,value))
			return;
		num = atoi(value);
		if (num < var->min || num > var->max)
		{
			send_to_user(from,TEXT_VARPOSSVALS,var->min,var->max);
			return;
		}
	}
	if (!IsGlobal(&current->DefaultVars[which]))
	{
		if (*chan == '*')
		{
			if (!value)
			{
				send_to_user(from,TEXT_VARVALUEREQ);
				return;
			}
			temp = current->Channels;
			while(temp)
			{
				var = (VarStruct *)&temp->Vars[which];
				if (IsInt(var))
					var->value = num;
				else
				{
					if (var->strvalue)
						MyFree(&var->strvalue);
					set_mallocdoer(do_set);
					mstrcpy(&var->strvalue, value);
				}
				temp = temp->next;
			}
			send_to_user(from,"Var: %s",VarName[which].desc);
			send_to_user(from,TEXT_VARVALCHANGED);
			return;
		}
		var = find_var(chan,which);
	}
	if (!var)
	{
		send_to_user(from,TEXT_VARPROBFIND,chan);
		return;
	}		
	send_to_user(from,"Var: %s",VarName[which].desc);
	if (!value)
	{
 		if (IsInt(var))
			send_to_user(from,TEXT_VARVALNUM,chan,var->value);
		if (IsStr(var))
			send_to_user(from,TEXT_VARVANSTR,chan,var->strvalue);
		return;
	}
	if (IsInt(var))
	{
		var->value = num;
		send_to_user(from,TEXT_VARVALFORCHANGED,chan,num);
		return;
	}
	set_mallocdoer(do_set);
	mstrcpy(&var->strvalue,value);
	send_to_user(from,TEXT_VARVALCHANGEDTO,value);		
}

#define MD_TOGGLE	1
#define MD_SET		2
#define MD_UNSET	3

void do_toggle(char *from, char *to, char *rest, int cmdlevel)
{
	VarStruct *var;
	aChan	*Chan;
	char	*channel,*setname,*ms;
	int	which;
	int	mode;

	mode = MD_TOGGLE;
	channel = get_channel2(to,&rest);
	if ((get_userlevel(from,channel)) < cmdlevel)
		return;
	setname = chop(&rest);
	if (!setname || !*setname)
	{
		send_to_user(from,TEXT_VARNONEGIVEN);
		return;
	}
	ms = chop(&rest);
	if (ms && *ms)
	{
		if (!Strcasecmp(ms,"on"))
			mode = MD_SET;
		else
		if (!Strcasecmp(ms,"off"))
			mode = MD_UNSET;
		else
		if (!strcmp(ms,"1"))
			mode = MD_SET;
		else
		if (!strcmp(ms,"0"))
			mode = MD_UNSET;
		else
		{
			usage(from,C_TOG);
			return;
		}
	}
	if ((which = find_var_name(setname,TOG_VAR)) == -1)
	{
		send_to_user(from,TEXT_VARNOSUCH);
		return;
	}
	var = &current->DefaultVars[which];
	if (!IsGlobal(&current->DefaultVars[which]))
	{
		if (*channel == '*')
		{
			for(Chan=current->Channels;Chan;Chan=Chan->next)
			{
				var = &Chan->Vars[which];
				switch(mode)
				{
				case MD_SET:
					var->value = TRUE;
					break;
				case MD_UNSET:
					var->value = FALSE;
					break;
				case MD_TOGGLE:
					if (var->value)
						var->value = FALSE;
					else
						var->value = TRUE;
					break;
				}
			}
			send_to_user(from,"Var: %s",VarName[which].desc);
			if (mode == MD_TOGGLE)
				send_to_user(from,TEXT_VARVALCHANTOG);
			else
				send_to_user(from,TEXT_VARVALCHANTOGTO,
					(var->value) ? "On" : "Off");
			return;
		}
		var = find_var(channel,which);
	}
	if (!var)
	{
		send_to_user(from,TEXT_VARPROBNOTOG,channel);
		return;
	}
	switch(mode)
	{
	case MD_SET:
		var->value = TRUE;
		break;
	case MD_UNSET:
		var->value = FALSE;
		break;
	case MD_TOGGLE:
		if (var->value)
			var->value = FALSE;
		else
			var->value = TRUE;
		break;
	}
	send_to_user(from,"Var: %s",VarName[which].desc);
	send_to_user(from,TEXT_VARVALNOWTOG,
		IsGlobal(var) ? "(global)" : channel,
		(var->value) ? "On" : "Off");

	check_shit();
}

void do_report(char *from, char *to, char *rest, int cmdlevel)
{
#ifndef NEWBIE
	char	tmp[MSGLEN];
	char	*pt,*pp;
	int	l;
#endif
	VarStruct *Vars;
	aChan	*Chan;
	char	*channel,*tempstr;
	int	i,m,tempval;

	Chan = NULL;
	channel = get_channel(to,&rest);
	if (!ischannel(channel))
	{
		if (current->CurrentChan)
		{
			usage(from,C_REPORT);
			return;
		}
		Vars = current->DefaultVars;
	}
	else
	if ((Chan = find_channel(channel,CH_ACTIVE)) == NULL)
	{
		send_to_user(from,ERR_CHAN,channel);
		return;
	}
	else
		Vars = Chan->Vars;
	m = 4;
	while(m)
	{
		for(i=0;VarName[i].name;i++)
		{
			switch(m)
			{
			case 4:
#ifdef NEWBIE
				send_to_user(from,TEXT_REPGLOBALSET);
#else
				send_to_user(from,TEXT_REPGLOBAL1);
				send_to_user(from,TEXT_REPGLOBAL2);
				tmp[0] = 0;
#endif
				m--;
			case 3:
				if (!(IsGlobal(&Vars[i])))
					continue;
				tempstr = current->DefaultVars[i].strvalue;
				tempval = current->DefaultVars[i].value;
				break;
			case 2:
#ifdef NEWBIE
				if (!Chan)
					send_to_user(from,TEXT_REPDEFCHANSET);
				else
					send_to_user(from,TEXT_REPCHAN1,Chan->name);
#else
				send_to_user(from,TEXT_REPCHAN2,(Chan) ? Chan->name : "default");
				tmp[0] = 0;
#endif
				m--;
			default:
				if (IsGlobal(&Vars[i]))
					continue;
				tempstr = Vars[i].strvalue;
				tempval = Vars[i].value;
			}

#ifdef NEWBIE
			if (IsInt(&Vars[i]))
				send_to_user(from,"[SET] %-10s %-5i (%s)",
					VarName[i].name,(tempval),VarName[i].desc);
			if (IsStr(&Vars[i]))
				send_to_user(from,"[SET] %-10s %-5s (%s)",
					VarName[i].name,nullstr(tempstr),VarName[i].desc);
			if (IsTog(&Vars[i]))
				send_to_user(from,"[TOG] %-10s %-5s (%s)",
					VarName[i].name,(tempval) ? "On" : "Off",VarName[i].desc);
#else /* NEWBIE */
			l = strlen(tmp) + strlen(VarName[i].name);
			if (IsInt(&Vars[i]))
			{
				if (l > 58)
				{
					send_to_user(from,"%s",tmp);
					tmp[0] = 0;
				}
#ifdef HAS_OK_STRCHR
				pp = strchr(tmp,0);
#else /* HAS_OK_STRCHR */
				pp = tmp;
				while(*pp)
					pp++;
#endif /* HAS_OK_STRCHR */
				pt = VarName[i].name;
				while(*pt)
					*(pp++) = (char)tolower(*(pt++));
				sprintf(pp,"=%i ",tempval);
			}
			if (IsStr(&Vars[i]))
			{
				if (tempstr && *tempstr)
					l += strlen(tempstr);
				if ((l > 58) && (strlen(tmp) > 2))
				{
					send_to_user(from,"%s",tmp);
					tmp[0] = 0;
				}
#ifdef HAS_OK_STRCHR
				pp = strchr(tmp,0);
#else /* HAS_OK_STRCHR */
				pp = tmp;
				while(*pp)
					pp++;
#endif /* HAS_OK_STRCHR */
				pt = VarName[i].name;
				while(*pt)
					*(pp++) = (char)tolower(*(pt++));
				sprintf(pp,"=\"%s\" ",nullstr(tempstr));
			}
			if (IsTog(&Vars[i]))
			{
				if (l > 58)
				{
					send_to_user(from,"%s",tmp);
					tmp[0] = 0;
				}
#ifdef HAS_OK_STRCHR
				pp = strchr(tmp,0);
#else /* HAS_OK_STRCHR */
				pp = tmp;
				while(*pp)
					pp++;
#endif /* HAS_OK_STRCHR */
				if (tempval)
					*(pp++) = '+';
				else
					*(pp++) = '-';
				pt = VarName[i].name;
				while(*pt)
					*(pp++) = (char)tolower(*(pt++));
				*(pp++) = ' ';
				*pp = 0;
			}
#endif /* NEWBIE */
		}
#ifndef NEWBIE
		if (tmp[0] && tmp[1])
			send_to_user(from,"%s",tmp);
#endif
		m--;
	}
#ifdef NEWBIE
	send_to_user(from,TEXT_REPEND1);
#else
	send_to_user(from,TEXT_REPEND2);
#endif
}
