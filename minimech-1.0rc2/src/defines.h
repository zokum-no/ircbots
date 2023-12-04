/*

    Starglider Class EnergyMech, IRC bot software
    Parts Copyright (c) 1997-2000  proton

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
#define USERLIST_USER		1
#define USERLIST_SHIT		2
#define USERLIST_KICKSAY	3

#define IsUser(x)		((x)->type == USERLIST_USER)
#define IsShit(x)		((x)->type == USERLIST_SHIT)
#define IsKickSay(x)		((x)->type == USERLIST_KICKSAY)

/*
 *  Channel Crap
 */

#define COLLECT_TYPE		1
#define REJOIN_TYPE		2
#define CYCLE_TYPE		4

#define ADD_MODE		1
#define SUB_MODE		2
#define MODE_FORCE		3

#define MODE_PRIVATE		0x0004
#define MODE_SECRET		0x0008
#define MODE_MODERATED		0x0010
#define MODE_TOPICLIMIT		0x0020
#define MODE_INVITEONLY		0x0040
#define MODE_NOPRIVMSGS		0x0080
#define MODE_KEY		0x0100
#define MODE_BAN		0x0200
#define MODE_LIMIT		0x0400
#define MODE_FLAGS		0x07ff

#define CU_VOICE		0x0001
#define CU_CHANOP		0x0002
#define CU_VOICETMP		0x0004
#define CU_MODES		0x00ff

#define CU_DEOPPED		0x0100
#define CU_KICKED		0x0200
#define CU_BANNED		0x0400
#define CU_MASSTMP		0x0800

/*
 *  Bitfield for short_tv being set to 1 or 30 seconds
 */

#define TV_AVOICE		0x0001
#define TV_TELNET_NICK		0x0002
#define TV_SERVCONNECT		0x0004

/* Misc Crap: */

#define EXTRA_CHAR(q)		q==1?"":"s"
#define SPLIT			1

/* Parse Stuff */

#define PA_WHOIS		5
#define PA_LUSERS		15
#define PA_STATS		20
#define PA_USERHOST		25

/* DCC Crap: */

#define DCC_CHAT		0x0001
#define DCC_TELNET		0x0002

#define DCC_WAIT		0x0010
#define DCC_ACTIVE		0x0020
#define DCC_OFFER		0x0040
#define DCC_DELETE		0x0080
#define DCC_ASYNC		0x0200
#define DCC_STATES		0xfff0

/* Type of Variable: */

#define INT_VAR			0x01
#define STR_VAR			0x02
#define TOG_VAR			0x04
#define GLOBAL_VAR		0x08

#define IsInt(x)		((x)->type & INT_VAR)
#define IsStr(x)		((x)->type & STR_VAR)
#define IsTog(x)		((x)->type & TOG_VAR)
#define IsGlobal(x)		((x)->type & GLOBAL_VAR)

/* For Variables: */

enum {
	SETBT_VAR,
	SETAUB_VAR,
	SETCKL_VAR,
	SETMDL_VAR,
	SETMBL_VAR,
	SETMKL_VAR,
	SETMPL_VAR,
	SETFL_VAR,
	SETFPL_VAR,
	SETNCL_VAR,
	SETIKT_VAR,
	SETMAL_VAR,
	SETBANMODES_VAR,
	SETOPMODES_VAR,
	TOGRK_VAR,
	TOGTOP_VAR,
	TOGCK_VAR,
	TOGAOP_VAR,
	TOGSHIT_VAR,
	TOGPUB_VAR,
	TOGPROT_VAR,
	TOGAS_VAR,
	TOGABK_VAR,
	TOGIK_VAR,
	TOGSD_VAR,
	TOGSO_VAR,
	TOGENFM_VAR,
	TOGMASS_VAR,
	SETENFM_VAR,
	TOGCC_VAR,
	TOGKS_VAR,
	TOGDCC_VAR,
	TOGCTL_VAR,
	TOGSPY_VAR,
	TOGIWM_VAR,
	TOGCLOAK_VAR,
	TOGRV_VAR,
	TOGRF_VAR,
	SETAAWAY_VAR,
	TOGNOIDLE_VAR,
	TOGCHANLOG_VAR,
	SETAVOICE_VAR,
	NULL_VAR,
	SIZE_VARS
};

/*
 *  For botlinks
 */

#ifdef LINKING

/* Link->linktype */

#define LN_UNKNOWN		0
#define	LN_TRYING		1
#define LN_CONNECTED		2
#define LN_UPLINK		3
#define LN_USER			4
#define LN_REMOTE		5
#define LN_LINKDEAD		6
#define LN_PASSWAIT		7

/* Link->nodetype */

#define LN_PIPE_HASNICK		50
#define LN_SAYNICK_SEND		99
#define LN_ENTITY		100
#define LN_BOT			101
#define LN_DCCUSER		102

/* LinkConf->autoidx */

#define LC_NOTRY		0
#define LC_HASTRY		1
#define LC_LINKED		2
#define LC_NOPORT		3

#endif /* LINKING */

/* for connect status */

#define CN_NOSOCK		0
#define CN_TRYING		2
#define CN_CONNECTED		3
#define CN_ONLINE		4
#define CN_DISCONNECT		5
#define CN_BOTDIE		6
#define CN_NEXTSERV		7
#define CN_WINGATEWAIT		8

/* DCC Kill flags (BYE command) */

#define DCC_NULL		0
#define DCC_COMMAND		1
#define DCC_KILL		2

/* VHOST types */

#define VH_IPALIAS		0
#define VH_IPALIAS_FAIL		100
#define VH_WINGATE		1
#define VH_WINGATE_FAIL		101

/* server error types */

#define SP_NULL			0
#define SP_NOAUTH		1
#define SP_KLINED		2
#define SP_FULLCLASS		3
#define SP_TIMEOUT		4
#define SP_ERRCONN		5
#define SP_DIFFPORT		6

/* find_channel() */

#define CH_ACTIVE		0x1
#define CH_OLD			0x2
#define CH_ANY			0x3

/* check_mass() */

#define CHK_CAPS		0x01
#define CHK_PUB			0x02
#define CHK_DEOP		0x04
#define CHK_BAN			0x08
#define CHK_KICK		0x10
