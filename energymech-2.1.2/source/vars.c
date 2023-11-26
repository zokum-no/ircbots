/*
 * vars.c - keeps track of all the bots variables
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include "global.h"
#include "config.h"
#include "defines.h"
#include "structs.h"
#include "h.h"

int get_int_varc(channel, which)
aChannel *channel;
int which;
{
	if (!channel || IsGlobalType(&DefaultVars[which]))
		return DefaultVars[which].value;
	return channel->Vars[which].value;
}

char *get_str_varc(channel, which)
aChannel *channel;
int which;
{
	if (!channel || IsGlobalType(&DefaultVars[which]))
	        return channel->Vars[which].strvalue;
	return 0;
}

int get_int_var(name, which)
char *name;
int which;
{
	aChannel *Chan;

	if (!name ||
		(!(Chan=find_channel(name)) && !(Chan=find_oldchannel(name))))
		return get_int_varc(NULL, which);
	return IsGlobalType(&Chan->Vars[which]) ? DefaultVars[which].value :
		Chan->Vars[which].value; 
}

char *get_str_var(name, which)
char *name;
int which;
{
	aChannel *Chan;

	if (!name ||
		(!(Chan=find_channel(name)) && !(Chan=find_oldchannel(name))))
		return get_str_varc(NULL, which);
	return IsGlobalType(&Chan->Vars[which]) ? DefaultVars[which].strvalue :
		Chan->Vars[which].strvalue;
}

void set_int_varc(channel, which, value)
aChannel *channel;
int which;
int value;
{
	channel->Vars[which].value = value;
}

void set_str_varc(channel, which, value)
aChannel *channel;
int which;
char *value;
{
	char *temp;

	mstrcpy(&temp, value);
	if (channel->Vars[which].strvalue)
		MyFree((char **)&channel->Vars[which].strvalue);
	channel->Vars[which].strvalue = temp;
}

int set_int_var(name, which, value)
char *name;
int which;
int value;
{
	aChannel *Chan;
	if (!(Chan = find_channel(name)) && !(Chan = find_oldchannel(name)))
		return FALSE;
	Chan->Vars[which].value = value;
	return TRUE;
}

int set_str_var(name, which, value)
char *name;
int which;
char *value;
{
	aChannel *Chan;
	if (!(Chan = find_channel(name)) && !(Chan = find_oldchannel(name)))
		return FALSE;
	set_str_varc(Chan, which, value);
	return TRUE;
}

int find_var_name(name, type)
char *name;
int type;
{
	int i;

	for (i=0;*DefaultVars[i].name;i++)
		if (!my_stricmp(name, DefaultVars[i].name))
			if (DefaultVars[i].type&type)
				return i;
	return -1;
}

void copy_vars(dst, src)
VarStruct *dst, *src;
{
	int i;
	
	for (i=0;*src[i].name;i++)
	{
		strcpy(dst[i].name, src[i].name);
		strcpy(dst[i].longname, src[i].longname);
		dst[i].type = src[i].type;
		dst[i].value = src[i].value;
		MyFree(&dst[i].strvalue);
		if (dst[i].strvalue)
			mstrcpy(&dst[i].strvalue, src[i].strvalue);
		else
			dst[i].strvalue = (char *) 0;
		dst[i].min = src[i].min;
		dst[i].max = src[i].max;
	}	
}

void dup_defaultvars(void)
{
	int i;

	for (i=0; *DefaultVars[i].name; i++)
		if (DefaultVars[i].strvalue)
			mstrcpy(&DefaultVars[i].strvalue, DefaultVars[i].strvalue);
}

void free_strvars(vars)
VarStruct *vars;
{
	int i;

	for (i=0;*vars[i].name;i++)
		if (vars[i].strvalue)
			MyFree(&vars[i].strvalue);
}

VarStruct *find_var(name, which)
char *name;
int which;
{
	aChannel *Chan;

	if (!(Chan = find_channel(name)) && !(Chan = find_oldchannel(name)))
		return NULL;
	return (VarStruct *) &Chan->Vars[which];
}	

void do_set(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	char *chan, *setname, *value;
	int which;
	VarStruct *var;
	register aChannel *temp;
	int num;

	chan = get_channel2(to, &rest);
	if (!(setname = get_token(&rest, " ")))
	{
		send_to_user(from, "\002No variable name specified to set\002");
		return;
	}
	if ((which = find_var_name(setname, INT_VAR+STR_VAR)) == -1)
	{
		send_to_user(from, "\002No such variable name\002");
		return;
	}
	value = get_token(&rest , " ");
	var = &DefaultVars[which];
	num = 0;
	if (value && IsIntType(var))
        {
                if (check_for_number(from, value))
                        return;
                num = atoi(value);
                if (num < var->min || num > var->max)
                {
                        send_to_user(from, "\002Possible values are %i "
                                        "through %i\002", var->min, var->max);
                        return;
                }
        }
	if (!IsGlobalType(&DefaultVars[which]))
	{
		if (*chan == '*')
		{
			int times = 1;

			if (!value)
			{
				send_to_user(from,
					"\002Please specify a value\002");
				return;
			}
			temp = current->Channel_list;
			if (!temp)
				temp = current->OldChannel_list;
			while(temp)
			{
				var = (VarStruct *) &temp->Vars[which];
				if (IsIntType(var))
					var->value = num;
				else
				{
					if (var->strvalue)
						MyFree(&var->strvalue);
					mstrcpy(&var->strvalue, value);
				}
				temp = temp->next;
				if (!temp)
				{
					if (times==1)
					{
						temp = current->OldChannel_list;
						times++;
					}
				}
			}
			send_to_user(from, "\002Var: %s\002", var->longname);
      			send_to_user(from, "\002Value changed on all channels\002"); 
			return;
		}
		var = find_var(chan, which);
	}
	if (!var)
	{
		send_to_user(from, "\002Problem finding the variable (Am I on %s???)\002", chan);
		return;
	}		
	send_to_user(from, "\002Var: %s\002", var->longname);
	if (!value)
	{
 		if (IsIntType(var))
			send_to_user(from, "\002Value for %s: %i\002", chan,
				var->value);
		if (IsStrType(var))
			send_to_user(from, "\002Value for %s: %s\002", chan,
				var->strvalue);
		return;
	}
	if (IsIntType(var))
	{
		var->value = num;
		send_to_user(from, "\002Value for %s: Now changed to %i\002",
			chan, num);
		return;
	}
	mstrcpy(&var->strvalue, value);
	send_to_user(from, "\002Value: Now changed to %s\002",
		value);		
}

void do_toggle(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
        char *chan, *setname;
        int which;
        VarStruct *var;
	register aChannel *temp;

        chan = get_channel2(to, &rest);
        if (!(setname = get_token(&rest, " ")))
        {
                send_to_user(from, "\002No variable name specified to toggle\002");
                return;
        }
        if ((which = find_var_name(setname, TOG_VAR)) == -1)
        {
                send_to_user(from, "\002No such variable name\002");
                return;

        }
	var = &DefaultVars[which];
	if (!IsGlobalType(&DefaultVars[which]))
	{
		if (*chan == '*')
		{
			int times = 1;
			int hrm = -1;
                        temp = current->Channel_list;
                        if (!temp)
                                temp = current->OldChannel_list;
                        while(temp)
                        {
                                var = (VarStruct *) &temp->Vars[which];
				if (hrm == -1)
				{
					if (var->value == 0)
						var->value = 1;
					else
						var->value = 0;
					hrm = var->value;
				}
				else
					var->value = hrm;
                                temp = temp->next;
                                if (!temp)
                                {
                                        if (times==1)
                                        {
                                                temp = current->OldChannel_list;
                                                times++;
                                        }
                                }
                        }
                        send_to_user(from, "\002Var: %s\002", var->longname);
                        send_to_user(from, "\002Value on all channels toggled to %s\002",
				hrm == 0 ? "Off" : "On");
			return;
		}
		var = find_var(chan, which);
	}
	if (!var)
	{
		send_to_user(from, "\002Problem finding the toggle "
				"(Am I on %s???)\002", chan);
		return;
	}
	send_to_user(from, "\002Var: %s\002", var->longname);
	if (var->value)
	{
		var->value = 0;
		send_to_user(from, "\002Value on %s: Now toggled to Off\002",
			IsGlobalType(var) ? "(global)" : chan);
	}
	else
	{
		var->value = 1;
		send_to_user(from, "\002Value on %s: Now toggled to On\002",
			IsGlobalType(var) ? "(global)" : chan);
	}
}

void do_report(from, to, rest, cmdlevel)
char *from;
char *to;
char *rest;
int cmdlevel;
{
	int i, tempval;
	char *chan, *tempstr;
	VarStruct *Vars;

	chan = get_channel(to, &rest);

 	if (!(Vars = find_var(chan, 0)))
	{
		send_to_user(from, "\002Am I on %s?\002", chan);
		return;
	}

	send_to_user(from, "\002Report for %s:\002", chan);
	for (i=0;*Vars[i].name;i++)
        {
		tempstr = Vars[i].strvalue;
		tempval = Vars[i].value;		
		if (IsGlobalType(&Vars[i]))
		{
			tempstr = DefaultVars[i].strvalue;
			tempval = DefaultVars[i].value;
		}
		if (IsIntType(&Vars[i]))
			send_to_user(from, "(SET) %10s is set to %4i",
				Vars[i].name, tempval);
		if (IsStrType(&Vars[i]))
			send_to_user(from, "(SET) %10s is set to %s",
				Vars[i].name, tempstr ?
				tempstr : "<null>");
		if (IsTogType(&Vars[i]))
			send_to_user(from, "(TOG) %10s is set to %s",
				Vars[i].name, (tempval) ?
					"On" : "Off");
	}
}

