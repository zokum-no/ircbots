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

typedef struct aXDCC
{
	struct		aXDCC *next;

	int		sock;
	int		filedes;		/* file descriptor */
	int		mode;			/* listener or active socket? */
	int		rlen;			/* remaining buffer data to send */
	char		*rpos;			/* where in buffer the rest is */
	char		sd[MSGLEN];		/* send buffer */
	off_t		filesize;

} aXDCC;

typedef struct aDCC
{
	struct		aDCC *next;

	int		sock;
	char		sd[MSGLEN];
	int		flags;
	char		user[NAMELEN];
	int		inputcount;		/* used to enforce input limit */
	time_t		lasttime;               /* can be used for idletime */

} aDCC;

typedef struct aPipeUser
{
	int		sock;
	char		sd[MSGLEN];
	char		name[NICKBUF];
	int		has_nick:1;		/* caught the 1st "NICK" ? */
	time_t		lasttime;

} aPipeUser;

typedef struct VarStruct
{
	int		type;
	int		value;
	char		*strvalue;
	int		min;
	int		max;

} VarStruct;

typedef VarStruct VarType[SIZE_VARS];

typedef struct aVarName
{
	char		*name;
	char		*desc;

} aVarName;

typedef struct aTime
{
	struct		aTime *next;
	struct		aTime *prev;
	char		*name;
	time_t		time;
	int		num;

} aTime;

typedef struct aStrp
{
	struct		aStrp *next;
	char		p[1];

} aStrp;

typedef struct aUser
{
	char		used;
	int		ident;
	char		name[NICKBUF];		/* userlist */
	char		pass[PASSBUF];		/* userlist */
	char		*chanp;			/* userlist, shitlist, kicksays */
	char		*whoadd;		/* shitlist, kicksays */
	char		*reason;		/* shitlist, kicksays */
	union {
	aStrp		*umask;			/* userlist */
	char		*kicksay;		/* kicksays */
	char		*shitmask;		/* shitlist */
	} parm;
	Ulong		echo:1,			/* partyline echo of own messages	*/
			aop:1,			/* auto-opping				*/
			avoice:1,		/* auto-voicing				*/
			prot:3,			/* protlevel (0-4) 		[0-7]	*/
			access:8,		/* access level (0-200)		[0-255]	*/
			type:3;			/* type of user record (1-3)	[0-7]	*/
	time_t		time;			/* shitlist */
	time_t		expire;			/* shitlist */

} aUser;

typedef struct aBan
{
	struct		aBan *next;
	struct		aBan *prev;

	char		*banstring;
	char		*bannedby;
	time_t		time;

} aBan;

typedef struct aChanUser
{
	struct		aChanUser *next;

	Ulong		flags;
	int		floodnum,bannum,deopnum,
			kicknum,nicknum,capsnum;
	time_t		floodtime,bantime,deoptime,
			kicktime,nicktime,capstime,
			idletime;

	char		*nick;		/* nick can change without the user reconnecting */

	char		*host;
	char		user[2];

} aChanUser;

typedef struct aChan
{
	struct		aChan *next;

	char		*name;			/* name of the channel			*/
	char		*key;			/* channel key, if any			*/
	char		*topic;			/* channel topic, if any		*/
	int		limit;			/* channel limit			*/
	char		*kickedby;		/* n!u@h of whomever kicks the bot	*/
	aBan		*banned;		/* banlist				*/
	aChanUser	*users;			/* users				*/
	aChanUser	*cacheuser;		/* cache for find_chanuser()		*/
	VarType 	Vars;			/* channel vars				*/
	aStrp		*avoice;		/* autovoice buffer			*/
	time_t		last_voice;		/* last time we send +v modes		*/
	time_t		last_needop;		/* last time we asked for ops		*/

	Ulong		private:1,		/* channel mode: +p			*/
			secret:1,		/* channel mode: +s			*/
			moderated:1,		/* channel mode: +m			*/
			topprot:1,		/* channel mode: +t			*/
			limitmode:1,		/* channel mode: +l			*/
			invite:1,		/* channel mode: +i			*/
			nomsg:1,		/* channel mode: +n			*/
			keymode:1,		/* channel mode: +k			*/
			hiddenkey:1,		/* Undernet screwup			*/
			sync:1,			/* join sync status			*/
			whois:1,		/* first /WHOIS				*/
			bot_is_op:1,		/* set if the bot is opped		*/
			active:2,		/* active or inactive channel	[0-3]	*/
#ifdef PIPEUSER
			pipeuser:1,		/* channel was JOIN'd by pipeuser	*/
#endif /* PIPEUSER */
#ifdef WARRING
			steal:1,		/* trying to steal it?			*/
#endif /* WARRING */
			rejoin:1;		/* trying to rejoin it?			*/

} aChan;

typedef struct aSeen
{
	struct		aSeen *next;

	char		*userhost;
	char		*signoff;	
	char		*kicker;
	time_t		time;
	int		selector;
	/*
	 *  p is used to malloc a variable length Seen record
	 *  so that nick, userhost, signoff and kicker is copied
	 *  into p at different offsets (saves malloc calls and
	 *  malloc memory headers). See make_seen for more info.
	 */
	char		nick[4];

} aSeen;

typedef struct aMsgList
{
	char		used;
	char		type[NAMELEN];
	char		who[NAMELEN];

} aMsgList;

typedef struct aServer
{
	char		used;
	int		ident;
	char		name[NAMELEN];
	char		pass[NAMELEN];
	char		realname[NAMELEN];
	Ulong		resolved;
	int		usenum;
	int		port;
	int		err;
	time_t		lastconnect;
	time_t		lastattempt;
	time_t		maxontime;

} aServer;

typedef struct aBot
{
	int		created;

	int		connect;
	int		sock;
	Ulong		ip;			/* for DCC			*/
	int		server;			/* ident of my current server	*/
	int		nextserver;
	char		sd[MSGLEN];

	/*
	 *  Line buffer for non-essential stuff
	 */
	aStrp		*sendq;
	time_t		last_sendq;

	/*
	 *  Basic bot information
	 */
	char		nick[NICKBUF];		/* current nickname		*/
	char		wantnick[NICKBUF];	/* wanted nickname		*/
	char		login[LOGINBUF];
	char		ircname[NAMELEN];
	char		vhost[MAXHOSTLEN];
	int		vhost_port;
	int		vhost_type;
	char		userhost[NUHLEN];
	char		modes[32];
	char		cmdchar;
	char		modes_to_send[32];

	/*
	 *  Buffers for do_die() command.
	 */
	char		*signoff;
	char		*from;

	Ulong		away:1;

#ifdef PIPEUSER
	aPipeUser	*pipeuser;
	char		*saved_002;
	char		*saved_003;
	char		*saved_004;
#endif /* PIPEUSER */

	VarType		DefaultVars;

	int		userlistSIZE;
	aUser		*userlistBASE;
	char		Userfile[FILENAMELEN];

	int		msglistSIZE;
	aMsgList	*msglistBASE;

	aTime		*Auths;
	aTime		*ParseList;

	aChan		*Channels;
	aChan		*CurrentChan;

	aDCC		*ClientList;

#ifdef SEEN
	char		*seenfile;
	aSeen		*SeenList;
#endif /* SEEN */

	char		*lastcmds[LASTCMDSIZE];

	time_t		lastreset;		/* last time bot was reset		*/
	time_t		lastping;		/* to check activity server		*/
	time_t		lastrejoin;		/* last time channels were reset	*/
	time_t		lastsave;		/* Saving stuff every hour		*/
	time_t		laststatus;		/* 5 mins status line			*/
	time_t		lastctcp;		/* Flood protection			*/

	time_t		conntry;		/* when connect try started		*/
	time_t		activity;		/* Away timer (AAWAY)			*/

	time_t		uptime;			/* how long the bot has been created	*/
	time_t		ontime;			/* how long the bot has been connected	*/

} aBot;

typedef struct aBotLink
{
	char		used;
	int		ident;
	char		linktype;
	char		nodetype;
	char		auth;
	char		autolinked;
	char		nick[ENTITYLEN];
	char		dccnick[NICKBUF];
	char		sd[MSGLEN];
	aStrp		*ushares;
	aBot		*bot;			/* pointer to the bot whoms userlist telnet user login matches */
	int		sock;
	int		hasdata;
	time_t		uptime;
	int		uplink;
	int		sendlink;

} aBotLink;

typedef struct aWillWont
{
	Ulong		seen:1,
			shellcmd:1,
			signals:1,
			onotice:1,
			sessions:1,
			autolink:1;

} aWillWont;

typedef struct aLinkConf
{
	char		used;
	int		ident;
	char		name[ENTITYLEN];
	char		pass[ENTITYLEN];
	char		host[NAMELEN];
	int		port;
	int		autoidx;

} aLinkConf;

typedef struct OnMsg
{
	char		*name;
	char		*alias;
	void		(*function)(char *, char *, char *, int);
	int		flags;

} OnMsg;

typedef struct MLStub
{
	char		used;
	int		ident;

} MLStub;

typedef struct aME
{
	void		*area;
	void		*doer;
	Ulong		size;
	time_t		when;
	char		touch;

} aME;

typedef struct aMEA
{
	struct		aMEA *next;
	aME		mme[MRSIZE];

} aMEA;

typedef struct aKillSock
{
	struct		aKillSock *next;
	time_t		time;
	int		sock;

} aKillSock;
