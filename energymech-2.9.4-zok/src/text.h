/*

    EnergyMech, IRC bot software
    Copyright (c) 2000-2001 proton, 2002-2004 emech-dev

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

//#ifndef TEXT_H
//#define TEXT_H 1

/*
 *  These are more or less globally used..
 */

#define TEXT_NOTINSERVLIST		"(not in serverlist)"
#define TEXT_NONE			"(none)"

#define TEXT_HOSTNAMETOOLONG		"Hostname exceeds maximum length"

#define TEXT_LISTSAVED			"Lists saved to file %s"
#define TEXT_LISTREAD			"Lists read from file %s"
#define TEXT_NOSAVE			"Lists could not be saved to file %s"
#define TEXT_NOREAD			"Lists could not be read from file %s"

/*
 * global.h
 */

#define TEXT_GLH_NOTONCHAN		"I'm not on %s"
#define TEXT_GLH_CANTOPEN		"Couldn't open the file %s"
#define TEXT_GLH_INITWARNING		"init: Warning:"
#define TEXT_GLH_INVALIDNICK		"Invalid nickname: %s"
#define TEXT_GLH_NOACTIVECHANS		"I'm not active on any channels"
#define TEXT_GLH_CANTSAVELISTS		"Lists could not be saved to file %s"
#define TEXT_GLH_IMNOTOPPED		"I'm not opped on %s"

#define TEXT_GLH_VAR_BT			"Which bans to unban when ban-limit is reached"
#define TEXT_GLH_VAR_AUB		"Erases old bans automatically, this sets how old they have to be"
#define TEXT_GLH_VAR_CKL		"CAPS kick level (How many lines of CAPS are allowed)"
#define TEXT_GLH_VAR_MDL		"Mass deop level (How many -o's count as a massdeop)"
#define TEXT_GLH_VAR_MBL		"Mass ban level (How many +b's count as a massban)"
#define TEXT_GLH_VAR_MKL		"Mass kick level (How many kicks count as a masskick)"
#define TEXT_GLH_VAR_MPL		"Mass protection level (What to do with massmoders)"
#define TEXT_GLH_VAR_FL			"Flood level (How many lines count as a flood)"	
#define TEXT_GLH_VAR_FPL		"Flood protection level (What to do with flooders)"
#define TEXT_GLH_VAR_NCL		"Nick change level (How many nick changes allowed)"
#define TEXT_GLH_VAR_IKT		"Idle-kick time (How long users can be idle)"	
#define TEXT_GLH_VAR_MAL		"Mass action level (Max userlevel that mass functions won't affect)"
#define TEXT_GLH_VAR_TR			"Topic refresh delay in minutes (0 turns it off)"
#define TEXT_GLH_VAR_BANMODES		"Number of MODE -b that can be done at a time"
#define TEXT_GLH_VAR_OPMODES		"Number of MODE -o that can be done at a time"
#define TEXT_GLH_VAR_CTIMEOUT		"Server connection timeout"
#define TEXT_GLH_VAR_CDELAY		"Delay between server reconnects"
#define TEXT_GLH_VAR_RK			"Toggle for revenge kicking"
#define TEXT_GLH_VAR_TOP		"Toggle for topic enforcement"
#define TEXT_GLH_VAR_CK			"Toggle for CAPS kicking"
#define TEXT_GLH_VAR_AOP		"Toggle for auto-opping"
#define TEXT_GLH_VAR_SHIT		"Toggle for shit-kicking"
#define TEXT_GLH_VAR_PUB		"Toggle for being able to use public commands"
#define TEXT_GLH_VAR_PROT		"Toggle for protection"
#define TEXT_GLH_VAR_AS			"Toggle for auto-shitlisting"
#define TEXT_GLH_VAR_ABK		"Toggle for kicking banned users rejoining after splits"
#define TEXT_GLH_VAR_IK			"Toggle for the kicking of those who are idle"	
#define TEXT_GLH_VAR_SD			"Toggle for deopping those who get opped by a server"
#define TEXT_GLH_VAR_SO			"Toggle for letting only users get opped"
#define TEXT_GLH_VAR_ENFMTOG		"Toggle for mode enforcement"
#define TEXT_GLH_VAR_MASS		"Toggle for checking of massmodes"
#define TEXT_GLH_VAR_ENFMODE		"The modes to enforce"
#define TEXT_GLH_VAR_CC			"Toggle for the necessity of using the command char"
#define TEXT_GLH_VAR_KS			"Toggle for kicking on banned words"
#define TEXT_GLH_VAR_DCC		"Toggle for allowing non-users dcc stuphs"
#define TEXT_GLH_VAR_CTL		"Toggle for kicking users that have control chars in there userid"
#define TEXT_GLH_VAR_SPY		"Toggle seeing who does commands"
#define TEXT_GLH_VAR_IWM		"Toggle for ignoring wallops in spymsg"
#define TEXT_GLH_VAR_CLOAK		"Toggle for ignoring all CTCP requests"
#define TEXT_GLH_VAR_RV			"Toggle for random CTCP VERSION replies"
#define TEXT_GLH_VAR_RF			"Toggle for random CTCP FINGER finger replies"
#define TEXT_GLH_VAR_AAWAY		"Set for setting the bot automatically away"
#define TEXT_GLH_VAR_NOIDLE		"Toggle for making the bots idle time always low"
#define TEXT_GLH_VAR_CHANLOG		"Toggle for channel logging"
#define TEXT_GLH_VAR_AVOICE		"Auto-voicing mode"
#ifdef DYNCHANLIMIT
#define TEXT_GLH_VAR_DYNAMIC		"Toggle for updating the +l user limit"
#endif /* DYNCHANLIMIT */

/*
 * cfgfile.c
 */
 
#define ERR_NOTJOINCHANS	"%s %s will not join any channels\n"
#define ERR_ILLEGALNICK		"init: Error: Illegal nick %s\n"
#define ERR_COULDNTADD		"init: Error: Couldn't add %s\n"
#define ERR_STRAYLOGIN		"%s Stray LOGIN setting (ignored)\n"
#define ERR_STRAYIRCNAME	"%s Stray IRCNAME setting (ignored)\n"
#define ERR_INVALIDCHANNEL	"%s Invalid channel name\n"
#define ERR_MULTISIGMASTER	"%s Multiple SIGMASTER: Only one bot can be sigmaster (ignored)\n" 
#define ERR_STRAYSIGMASTER	"%s Stray SIGMASTER setting (ingored)\n"
#define ERR_STRAYCMDCHAR	"%s Stray CMDCHAR setting (ignored)\n"
#define ERR_STRAYSET		"%s Stray SET setting (ignored)\n"
#define ERR_UNKNOWNSET		"%s Unknown set (%s) in %s\n"
#define	ERR_SETNOACTIVECHAN	"%s Channel SET (%s) with no active channel (ignored)\n"
#define ERR_STRAYTOG		"%s Stray TOG setting (ignored)\n"
#define ERR_UNKNOWNTOG		"%s Unknown toggle (%s) in %s\n"
#define ERR_TOGNOACTIVECHAN	"%s Channel TOG (%s) with no active channel (ignored)\n"
#define ERR_STRAYMODE		"%s Stray MODES setting (ignored)\n"
#define ERR_STRAYVIRTUAL	"%s Stray VIRTUAL setting (ignored)\n"
#define ERR_STRAYWINGATE	"%s Stray WINGATE setting (ignored)\n"
#define ERR_WINGATENOPORT	"init: Syntax error in WINGATE statement, port number expected\n"
#define ERR_ALIASREPLACECMD	"init: Alias replacing original command (ignored)\n"
#define ERR_ALIASEXISTS		"init: Alias %s already exists (ignored)\n"
#define ERR_UNKNOWNCONFIGITEM	"init: Unknown configuration item: \"%s\" (ignored)\n"
#define TEXT_RESTORESESSION	"init: Restoring previously saved session...\n"
#define ERR_OPENFILE		"init: Couldn't open the file %s\n"
#define ERR_NOBOTSINCONFIG	"init: Error: No bots in the configfile\n"
#define ERR_ENTITYNOTSET	"%s Entity name not set, deactivating linkport\n"
#define ERR_SERVICEDUPED	"init: Service %s already exists (ignored)\n"
#define ERR_SERVICEADDFAIL	"init: Failed to add service %s\n"	
#define TEXT_MECHSADDEDMULTI	"init: Mech(s) added [ "
#define TEXT_MECHSADDEDSINGLE	"init: Mech(s) added [ %s ]\n"

/*
 * channel.c
 */
 
#define TEXT_USERSIDLE		"Users on %s that are idle more than %i second%s:"
#define TEXT_ENDOFLIST		"--- end of list ---"
#define TEXT_IDLEFORMIN		"Idle for %i minutes"
#define	TEXT_AUTOSHIT		"Auto-Shit"
#define TEXT_AUTOSHITREASON	"Quit the nick floods you lamer"
#define TEXT_GETOUTNICKFLOOD	"Get the hell out nick-flooding lamer!!!"
#define TEXT_KICKNICKFLOOD	"%s kicked from %s for nick flooding"
#define TEXT_CHECKOLDCHANS	"I'm not on %s, checking old channels..."
#define TEXT_NOTFOUND		"Nope...not found"
#define TEXT_USERSLASTTIME	"Users on %s the last time I was there:"
#define TEXT_NAMESON		"Names on %s:"
#define TEXT_SERVICEUNKCHAN	"I have no knowledge of channel %s"
#define TEXT_SERVICEEXISTS	"Service definition already exists."
#define TEXT_SERVICEADDERR	"Error adding service."
#define TEXT_SERVICEADDOK	"Service added successfully."
#define TEXT_SERVICENOTEXIST 	"Service definition specified doesn't exist!"
#define TEXT_SERVICEDELERR 	"Error deleting service."
#define TEXT_SERVICEDELOK	"Service deleted."
#define TEXT_SERVICELIST	"Services List:"
#define TEXT_SERVICELISTLOGIN   "  LOGIN:"
#define TEXT_SERVICELISTTOMSG	"    TO: %s, MSG: %s"
#define TEXT_SERVICELISTNEEDOP  "  NEEDOP for %s:"

/*
 *  combot.c
 */

#define ERR_NOUSERFILENAME	"No userfile has been set"
#define TEXT_KILLINGMECH	"Killing mech: %s"
#define TEXT_SAVELISTS		"Saving the lists..."
#define ERR_SEENLISTNOSAVE	"SeenList could not be saved to file %s"
#define ERR_LEVELSNOSAVE	"Levels could not be saved to %s"
#define TEXT_SHUTDOWN		"Shutdown complete"
#define TEXT_REMOVEPIPEUSER	"(signoff) Removing pipuser stuff...\n"
#define TEXT_REMOVESEENLIST	"(signoff) Removing seenlist...\n"
#define TEXT_REMOVEUSERLIST	"(signoff) Removing userlist...\n"
#define TEXT_REMOVELASTCMD	"(signoff) Removing lastcmd list...\n"
#define TEXT_UNLINKBOTRECORD	"(signoff) Unlinking bot record from linked list...\n"
#define TEXT_ALLDONE		"(signoff) All done.\n"
#define TEXT_SETREJOINSYNC	"[CtS] Setting rejoin- and synced-status for all channels\n"
#define TEXT_NOTINSVRLIST	"C:%s AC:%i CS:(not in serverlist)"
#define TEXT_SEENNOSAVEFILE	"SeenList could not be saved to file %s"
#define TEXT_SESSIONNOSAVEFILE	"Session could not be saved to file %s"
#define TEXT_LEVELNOSAVE	"Levels could not be saved to %s"
#define TEXT_WINGATECONNECT	"[PSI] Connecting via WinGate proxy...\n"
#define TEXT_WINGATEACTIVE	"[PSI] WinGate proxy active\n"
#define TEXT_CLOSESVRSOCKET	"[PSI] {%i} errno = %i; closing server socket\n"

/*
 * com-ons.c
 */

#define TEXT_WHERELOOK		"Who do you want me look for?"
#define TEXT_ILLEGALNICK	"Illegal nick"
#define TEXT_ITSME		"%s is me you idiot!"
#define TEXT_FINDYOURSELF	"Trying to find yourself %s?"
#define TEXT_NOMEMORY		"I have no memory of %s"
#define TEXT_RIGHTHERE		"%s is right here moron!"
#define TEXT_PARTEDFROM		"%s parted from %s %s ago"
#define TEXT_SIGNEDOFF		"%s signed off %s ago with message [%s]"
#define TEXT_CHANGEDNICK	"%s changed nicks to %s, %s ago"
#define TEXT_KICKEDBY		"%s was kicked by %s with message [%s] %s ago"
#define ERR_NOHELPTOPIC		"No help found on that topic."
#define TEXT_HELPON		"[\037Help on %s\037]\n"
#define TEXT_LEVELNEEDED	 "Level needed: %i"
#define TEXT_CMDSAVAILABLE	"[\037Commands available to you\037]\n"
#define TEXT_CMDLEVEL		"\037Level %3i\037: "
#define TEXT_ENDHELP		"\n\n[\037End of Help\037]"
#define TEXT_CMDSAVAILATLVL	"[\037Commands available at level %i\037]\n"
#define TEXT_CMDSMATCHLVL	"[\037Commands that match query %s\037]\n"
#define TEXT_KICKEDBYIRC	"*** %s was kicked by %s (%s)"
#define TEXT_BANNEDONSERVER	"You are banned on this server"
#define TEXT_CTRLCHARSUSERID	"Lame Control-Chars in UserId"
#define TEXT_NOWKNOWNAS		"*** %s is now known as %s"
#define TEXT_USEVERIFY		"Use \"VERIFY\" to get verified first"
#define TEXT_EXECUTING		":%s[%i]: Executing %s[%i]"
#define TEXT_SQUEEZEME		"Squeeze me?"
#define TEXT_LOTSOCAPS		"STOP YELLING ALREADY!!!"
#define TEXT_FLOODER		"Get out you flooder!"
#define TEXT_KICKFLOODER	"%s kicked from %s for flooding"
#define TEXT_SETSMODE		"*** %s sets mode: %s"
#define TEXT_LEVELBETWEEN	"Immortality level must be between 0 and %i"	
#define TEXT_NOCHANGELVLHIGH	"Can't change level to one higher than yours"
#define TEXT_LEVELNEEDEDCMD	"The level needed for that command is %i"
#define TEXT_SPECIFYLEVEL	"To change it, specify a level"
#define TEXT_NOCHGLVLHIGHYOURS	"Can't change a level that is higher than yours"
#define TEXT_LVLNOCHANGE	"The level was not changed"
#define TEXT_IMORTALLVLCHANGE	"Immortality level changed from %i to %i"
#define TEXT_CMDUNKNOWN		"That command is not known"
#define TEXT_NOWIN2K		"You cant use Windows 2000, dont you know that?"
#define TEXT_NOUSAGE		"Usage: (missing)"
#define TEXT_UNKNOWNCMD		"Unknown command: %s"

/*
 * commands.c
 */
 
#define TEXT_IMORTSTATS		"[Immortality stats for %s on %s]"
#define TEXT_IMORTHANDLE	"          Handle: %s"
#define TEXT_IMORTUSERLVL	"      User level: %i%s"
#define TEXT_REGOGNISEDUH	"  Recognized u@h: %s%s"
#define TEXT_IMORTVERIFIED	"        Verified: %s"
#define TEXT_IMORTPROTLVL	" Protected level: %-3i%s"
#define TEXT_PROTTOGOFF		"   (Protection toggled off)"
#define TEXT_IMORTAOP		"      Auto Opped: %s%s"
#define TEXT_AOPTOGOFF		"   (Auto-op is toggled off)"
#define TEXT_IMORTPARTYECHO	"  Partyline Echo: %s"
#define TEXT_SHITSTATS		"[Shitlist stats for %s on %s]"
#define TEXT_SHITAS		"   Shitlisted as: %s"
#define TEXT_SHITBY		"   Shitlisted by: %s"
#define TEXT_SHITON		"      Shitted on: %s"
#define TEXT_SHITLVL		"Shitlisted level: %i"
#define TEXT_SHITEXPIRE		"Shitlist expires: %s"
#define TEXT_SHITREASON		" Shitlist reason: %s"
#define TEXT_ENDUSTATS		"[End of UStats]"
#define TEXT_IMORTLVLCMD	"The immortality level needed for that command is: %i"
#define TEXT_IMORTLVLNUH	"Immortality Level for %s"
#define TEXT_CHANACCESS		"Channel: %s  Access: %i"
#define TEXT_ALREADYAUTH	"You have already been authorized"
#define TEXT_NOPASSREQ		"You don't need a password to do commands"
#define TEXT_NOPASSGIVEN	"No password given, not verified"
#define TEXT_USERRECCONFLICT	"User record conflict"
#define TEXT_IMMORTAL		"You are now officially immortal"
#define TEXT_BADPASS		"Incorrect password, not authorized"
#define TEXT_MORTAL		"You are now mortal"
#define TEXT_NOTVERIFIED	"You are not verified"
#define TEXT_CURTIME		"Current time is: %s"
#define TEXT_CURCHANSET		"Current channel set to %s"
#define TEXT_CURCHAN		"Current channel: %s"
#define TEXT_INVITEUSER		"User(s) invited to %s"
#define TEXT_ALREADYDCC		"You are already DCC chatting me"
#define TEXT_NICETRY		"Nice try, but I don't think so"
#define TEXT_NODCCNICK		"I have no DCC connection to %s"
#define TEXT_MSGSENTNICK	"Message sent to %s"
#define TEXT_INVALIDCHAN	"Invalid channel name"
#define TEXT_ATTEMPTJOIN	"Attempting the join of %s"
#define TEXT_LEAVECHAN		"Leaving %s"
#define TEXT_NICKTOOLONG	"Nickname too long: %s"
#define TEXT_DEFSIGNOFF		"I'll get you for this!!!"
#define TEXT_HANDLELEN		"Handle must be between 1 and %i chars in length"
#define TEXT_HANDLEINUSE	"Handle %s is already in use"
#define TEXT_NONICKORNUH	"No nick or userhost specified"
#define TEXT_NOLVLGIVEN		"No level specified"
#define TEXT_PROBGLOBALMASK	"Problem adding %s (global mask)"
#define TEXT_PROBINVALIDMASK	"Problem adding %s (invalid mask)"
#define TEXT_VALIDLVL		"Valid levels are from 0 thru %i"
#define TEXT_LVLLESSTHAN	"Level must be less than or equal to yours on %s"
#define TEXT_ALREADYUSER	"%s %s on %s is already a user"
#define TEXT_USERADDED		"%s has been added as %s on %s"
#define TEXT_USERACCAOPPROT	"Access level: %i  Auto-op: %s  Protect level: %i  %s %s"
#define TEXT_USERSPAM1		"NOTICE %s :%s has blessed you with %i levels of immortality"
#define TEXT_USERSPAM2		"NOTICE %s :You are %sbeing auto-opped *smo0tch*"
#define TEXT_USERSPAM3		"NOTICE %s :You have been granted %i terajoules of EnergyMech Ego Shields"
#define TEXT_USERSPAM4		"NOTICE %s :EnergyMech force does not protect you"
#define TEXT_USERSPAM5		"NOTICE %s :My command character is %c"
#define TEXT_USERSPAM6		"NOTICE %s :Use \026%c%s\026 for command help"
#define TEXT_USERSPAM7		"NOTICE %s :Password necessary for doing commands: %s"
#define TEXT_USERSPAM8		"NOTICE %s :If you do not like your password, use \026%c%s\026 to change it"
#define TEXT_PROBADDINGUSER	"Problem adding the user"
#define TEXT_UNKNOWNHANDLE	"Unknown handle"
#define TEXT_DELBOT		"Deleting bot %s"
#define TEXT_NICKLVLHIGHERYOU	"%s has a higher immortality level than you on %s"
#define TEXT_USERPURGE		"User %s has been purged"
#define TEXT_LVLSWRITTENFIE	"Levels were written to %s"
#define TEXT_LVLSNOTREAD	"Levels could not be read from %s"
#define TEXT_LVLSREAD		"Levels were read from %s"
#define TEXT_LISTSSAVEDFILE	"Lists%s saved to file %s"
#define TEXT_LISTSCOULDNOTBE	" could not be"
#define TEXT_LISTSREADFILE	"Lists%s read from file %s"
#define TEXT_NONICK		"No nick specified"
#define TEXT_BANATTEMPTNUH	"Ban attempt of %s"
#define TEXT_BANNEDON		"%s banned on %s"
#define TEXT_SITEBANNUH		"Siteban attempt of %s"
#define TEXT_SITEBANNEDON	"%s sitebanned on %s"
#define TEXT_KICKBANNUH		"Kickban attempt of %s"
#define TEXT_KICKREQ		"Requested Kick"
#define TEXT_KICKBANNEDON	"%s kickbanned on %s"
#define TEXT_SITEKBNUH		"Sitekickban attempt of %s"
#define TEXT_SITEKBON		"%s sitekickbanned on %s"
#define TEXT_KICKNUH		"Kick attempt of %s"
#define TEXT_KICKEDFROM		"%s kicked from %s"
#define TEXT_NOINFOCHAN		"I have no information on %s"
#define TEXT_NOMATCHUSERS	"No matching users found"
#define TEXT_NOUSERS		"No users found"
#define TEXT_ENDUSERLIST	"\n[\037End of Userlist\037]"
#define TEXT_NOREASON		"No reason specified"
#define TEXT_SHITALREADYINCHAN	"%s is in my shitlist already for this channel"
#define TEXT_ISGODCOMPARED	"%s is a God compared to you on %s"
#define TEXT_PROBSHITGLOBAL	"Problem shitting *!*@*, try again"
#define TEXT_VALIDSHITLVL	"Valid levels are from 1 thru %i"
#define TEXT_SHITUSERON		"The user has been shitted as %s on %s"
#define TEXT_SHITUSEREXPIRE	"The shitlist will expire: %s"
#define TEXT_PROBSHITUSER	"Problem shitting the user"
#define TEXT_SHITNOTINLISTUH	"%s is not in my shit list on that channel"
#define TEXT_SHITHIGHERLVLYOU	"The person who did this shitlist has a higher level than you"
#define TEXT_SHITREMOVECHAN	"User %s is no longer being shitted on %s"
#define TEXT_USERUNKNOWN	"Unknown user: %s"
#define TEXT_ACCESSDENIED	"access denied"
#define TEXT_LVLCHANGED		"Level changed from %i to %i"
#define TEXT_UNBANCHAN		"You have been unbanned on %s"
#define TEXT_UNBANNEDON		"%s unbanned on %s"
#define TEXT_SPAWNFAIL		"Couldn't spawn the bot"
#define TEXT_SPAWNSUCCESS	"New bot spawned: %s"
#define	TEXT_TOPICCHANGED	"Topic changed on %s"
#define TEXT_MSGREDIRECT	"Now redirecting messages to you"
#define TEXT_ADDEDTOCORE	"(%s) Added to mech core"
#define TEXT_MSGNOREDIRECT	"No longer redirecting messages to you"
#define TEXT_REMOVEDCORE	"(%s) Removed from mech core"
#define TEXT_HOSTTOOLONG	"Hostname exceeds maximum length"
#define TEXT_PROBSVRSWITCH	"Problem switching servers"
#define TEXT_TRYNEWSVR		"Trying new server: %s on port %i"
#define TEXT_TRYNEWSVRQUITMSG	"QUIT :Trying new server, brb..."
#define TEXT_SWITCHSVR		"Switching servers..."
#define TEXT_SWITCHSVRQUITMSG	"QUIT :Switching servers..."
#define TEXT_CYCLECHAN		"Cycling channel %s"
#define TEXT_OBJECTCHAN		"Object must be a channel"
#define TEXT_ACCESSTOOLOWON	"You don't have enough access on %s"
#define TEXT_ALREADYSPY		"You are already spying on %s"
#define TEXT_NOWSPY		"Now spying on %s"
#define TEXT_SPYNOMOREYOU	"No longer spying on %s for you"
#define TEXT_SPYNOMOREWHO	"No longer spying on %s for %s"
#define TEXT_NOTSPYON		"You are not spying on %s"
#define TEXT_SPYLIST		"[\037Spy List\037]\n"
#define TEXT_ENDSPYLIST		"\n[\037End of List\037]"
#define TEXT_BANLIST		"[\037Banlist on %s\037]\n\n\037Ban pattern\037%30s"
#define	TEXT_BANSETBY		"\037Set by\037"
#define TEXT_ENDBANLIST		"\n[\037End of Banlist\037]"
#define TEXT_NOBANSCHAN		"There are no active bans on %s"
#define TEXT_BADPASSLEN		"Password must be at least %i characters long"
#define TEXT_BADOLDPASS		"Old password is not correct"
#define TEXT_NEWPASSSET		"New password has been set"
#define TEXT_PASSREMOVED	"Password for %s has been removed"
#define TEXT_NEWPASSSETUSER	"New password for %s has been set"
#define TEXT_SHITLVLCHANGE	"The level has been changed"
#define TEXT_IDLENOTONCHANE	"That user is not on any of my channels"
#define TEXT_IDLEFOR		"%s has been idle for %s"
#define TEXT_SEENLISTNOLOAD	"SeenList could not be loaded from file %s"
#define TEXT_CHANNOTINMEM	"Channel %s is not in memory"
#define TEXT_CHANFORGOTTEN	"Channel %s is now forgotten"
#define TEXT_CHANNOFORGET	"Channel %s couldn't be forgotten"
#define TEXT_KICKSAYQUOTES	"Please put quotes (\"'s) around the arguements"
#define TEXT_KICKSAYEXAMPLE	"Example: KS * \"Fuck you\" \"No, fuck you %%n\""
#define TEXT_KICKSAYALREADY	"I'm already kicking on \"%s\""
#define TEXT_KICKSAYON		"Now kicking on \"%s\" on %s"
#define TEXT_RKICKSAYCHAN	"Specify a channel name or '*' for all"
#define TEXT_RKICKSAYNOSTRING	"No kicksay string specified"
#define TEXT_RKICKSAYNOTKICKON	"I'm not kicking on that"
#define TEXT_RKICKSAYNOLONGERON	"No longer kicking on %s"

/*
 * dcc.c
 */
 
#define TEXT_DCCDISCONNECT	"[%s] %s[%i] has disconnected"
#define TEXT_DCCCONNECT		"[%s] %s[%i] has connected"
#define TEXT_CTCPVERREQ		"[CTCP] :%s[%i]: Requested Version Info"
#define TEXT_CTCPFINGERREQ	"[CTCP] :%s[%i]: Requested Finger Info"
#define TEXT_DCCREQ		"[DCC] :%s[%i]: Requested DCC %s [%s]"
#define TEXT_CTCPUNKNOWN	"[CTCP] :%s[%i]: Unknown [%s]"

/*
 * function.c
 */
 
#define TEXT_NOINFOFOR		"No information found for %s"
#define TEXT_MASSMODES		"Quit the damn mass modes"
#define TEXT_GETOUTMASSMODES	"\002Get the fuck out mass moding lamer\002"
#define TEXT_ISPROTECTED	"\002%s is Protected\002"
#define TEXT_EXPECTNUM		"A number is expected instead of a string"

/*
 * link.c
 */
 
#define TEXT_NOENTITYSET	"No entity name set, no linking possible"
#define TEXT_LINKHEADER		"--[Entity]-----[Password]-----[Host]--------------------------[Port]--"
#define TEXT_LINKFOOTER		"-- Total: %i known entities --"
#define TEXT_USAGELINKUP	"Usage: LINK UP <entity>"
#define TEXT_ENTITYISME		"Entity \"%s\" is ME!"
#define TEXT_ENTITYUNKNOWN	"Unknown entity \"%s\""
#define TEXT_ENTITYNOHOST	"Entry for \"%s\" is missing hostname"
#define TEXT_ENTITYNOPORT	"Entity \"%s\" has no linkport!"
#define TEXT_ENTITYALREADYLINK	"Entity \"%s\" is already linked!"
#define TEXT_ENTITYTRYLINK	"Trying to link to entity \"%s\""
#define TEXT_ENTITYADDED	"Entity \"%s\" added"
#define TEXT_ENTITYREMOVED	"Entity \"%s\" removed"
#define TEXT_ENTITYNOTFOUND	"Entity \"%s\" not found"
#define TEXT_ENTITYFORCEDELINK	"$MSG $ Forced delink of \"%s\" by %s[%i] @ %s"
#define TEXT_ENTITYUNLINKED	"Entity \"%s\" unlinked"
#define TEXT_ENTITYINVALIDPORT	"Port number must be in the range 1-65535"
#define TEXT_ENTITYCHANGEPORT	"Changing linkport to %i"
#define TEXT_LINKUP		"Link established to \"%s\""

/*
 * main.c
 */
 
#define TEXT_SIGINT		"Lurking interrupted by luser ... er, owner. (SIGINT)"
#define TEXT_SIGBUS		"Another one drives the bus! (SIGBUS)"
#define TEXT_SIGSEGV		"Mary had a little signal segmentation fault (SIGSEGV)"
#define TEXT_SIGTERM		"What have I done to deserve this?? aaaaaarrghhh! (SIGTERM)"
#define TEXT_SIGUSR1		"QUIT :Switching servers... (SIGUSR1)"
#define ERR_USERLISTMATCH	"init: Error: UserList for %s matches the userlist for %s\n"
#define TEXT_NOSHAREUSERLIST	"             Bots can not share the same userlist, please specify a new one.\n"
#define ERR_NOUSERLIST		"init: Fatal: %s has no userlist, "
#define ERR_BOTSNOUSERLIST	"init: %d bot(s) have no userlist. EnergyMech NOT "
#define ERR_NOSTART		"init: Error: unable to stat executable\n"
#define TEXT_USAGE1NOR		"Usage: %s [switches [args]]\n"
#define TEXT_USAGE2NOR		" -f <file>   read configuration from <file>\n"
#define TEXT_USAGE3NOR		" -c          make core file instead of coredebug/reset\n"
#define TEXT_USAGE4DBG		" -r <num>    run only for <num> seconds\n"
#define TEXT_USAGE5DBG		" -d          start mech in debug mode\n"
#define TEXT_USAGE6DBG		" -o <file>   write debug output to <file>\n"
#define TEXT_USAGE7SDBG		" -X          write a debug file before exit\n"
#define TEXT_USAGE8NOR		" -h          show this help\n"
#define TEXT_USAGE9NOR		" -v          show EnergyMech version\n"
#define TEXT_ARGRMISSING	"init: argument to -r' is missing\n"
#define TEXT_INVALIDRUNTIME	"init: runtime must be longer than 30 seconds\n"
#define TEXT_NODBGFILE		"init: No debugfile specified\n"
#define TEXT_NOCFGFILE		"init: No configfile specified\n"
#define TEXT_USINGCFG		"init: Using config file: %s\n"
#define TEXT_UNKNOWNOPT		"init: Unknown option %s\n"

/*
 * parse.c
 */
 
#define TEXT_LUSERSTATS		"[\037Lusers status\037]"
#define TEXT_LUSERHEADER	"Users: \002%li\002 Invisible: \002%li\002 Servers: \002%li\002"
#define TEXT_LUSERIRCOP		"IRC Operators \002%li\002"
#define TEXT_LUSERUNKNOWNCON	"Unknown Connections: \002%li\002"
#define TEXT_LUSERCHANS		"Channels: \002%li\002"
#define TEXT_LUSERCLIENTSVR	"Clients: \002%li\002 Servers: \002%li\002"
#define TEXT_LUSERFOOTER	"[\037End of Lusers\037]"
#define TEXT_PARSEAWAY		"Away            - %s"
#define TEXT_WHOISHEADER	"Whois Info for  - %s"
#define TEXT_WHOISADDRESS	"Address         - %s@%s"
#define TEXT_WHOISIRCNAME	"IRCName         - %s"
#define TEXT_WHOISUSERHOST	"Userhost: %s=%s@%s"
#define TEXT_WHOISSVR1		"Server          - %s"
#define TEXT_WHOISSVR2		"                  (%s)"
#define TEXT_PARSEIRCOP		"IRCop           - %s %s"
#define TEXT_WHOISIDLE		"Idle            - %i minutes (%li seconds)"
#define TEXT_WHOISSIGNON	"Signed On       - %s"
#define TEXT_WHOISCHANS		"Channels        - %s"

/*
 * userlist.c
 */
 
#define TEXT_KSLISTHEADER	"--- Channel ------- String ---------- Kick Reason ------"
#define TEXT_KSLISTFOOTER	"--- End of kicksay list ---"
#define TEXT_INVALIDHANDLE	"Invalid handle"
#define TEXT_MASKEXISTS		"Mask %s already exists for %s"
#define TEXT_MASKPROBADDGLOBAL	"Problem adding %s (global mask)"
#define TEXT_MASKPROBADDINVALID	"Problem adding %s (invalid mask)"
#define TEXT_MASKADDED		"Added mask %s to user %s"
#define TEXT_MASKDELETED	"Deleted mask %s from user %s"
#define TEXT_ULISTHEADER	"[\037Userlist for %s\037]"
#define TEXT_ULISTTOTAL		"Total of %d entries"
#define TEXT_ULISTFOOTER	"[\037End of Userlist\037]"
#define TEXT_SHITHEADER		"------- Losers' Userhosts ----------- Channel - Lvl - Shitter"
#define TEXT_SHITFOOTER		"--------------------------------------------------------------"

/*
 * vars.c
 */
 
#define TEXT_VARNOSUCH		"No such variable name"
#define TEXT_VARPOSSVALS	"Possible values are %i through %i"
#define TEXT_VARVALUEREQ	"Please specify a value"
#define TEXT_VARVALCHANGED	"Value changed on all channels"
#define TEXT_VARPROBFIND	"Problem finding the variable (Am I on %s ?)"
#define TEXT_VARVALNUM		"Value for %s: %i"
#define TEXT_VARVANSTR		"Value for %s: %s"
#define TEXT_VARVALFORCHANGED	"Value for %s: Now changed to %i"
#define TEXT_VARVALCHANGEDTO	"Value: Now changed to %s"
#define TEXT_VARNONEGIVEN	"No variable name specified to toggle"
#define TEXT_VARVALCHANTOG	"Value on all channels has been toggled"
#define TEXT_VARVALCHANTOGTO	"Value on all channels toggled to %s"
#define TEXT_VARPROBNOTOG	"Problem finding the toggle (Am I on %s ?)"
#define TEXT_VARVALNOWTOG	"Value on %s: Now toggled to %s"
#define TEXT_REPGLOBALSET	"[\037Report: Global settings\037]"
#define TEXT_REPGLOBAL1		"[\037Configuration Report\037]\n"
#define TEXT_REPGLOBAL2		"(\037Global\037)"
#define TEXT_REPDEFCHANSET	"[\037Report: Default channel settings\037]"
#define TEXT_REPCHAN1		"[\037Report: Channel settings for %s\037]"
#define TEXT_REPCHAN2		"(Channel:\037%s\037)"
#define TEXT_REPEND1		"[\037End of Report\037]"
#define TEXT_REPEND2		"\n[\037End of Report\037]"

/*
 * xmech.c
 */
 
#define TEXT_NOLINKS		"No Links"
#define TEXT_CURNICKWANTED	"Current nick        %s (Wanted: %s)"
#define TEXT_CURNICK		"Current nick        %s"
#define TEXT_USERSSUPER		"Users in userlist   %i (%i Superuser%s, %i Bot%s)"
#define TEXT_ACTIVECHANS	"Active channels     %s"
#define TEXT_ACTIVECHANSNONE	"Active channels     (none)"
#define TEXT_VHOSTACTIVE	"Virtual host        %s (IP Alias)"
#define TEXT_VHOSTINACTIVE	"Virtual host        %s (IP Alias - Inactive)"
#define TEXT_VHOSTGATEACTIVE	"Virtual host        %s:%i (WinGate)"
#define TEXT_VHOSTGATEINACTIVE	"Virtual host        %s:%i (WinGate - Inactive)"
#define TEXT_CURSVR		"Current Server      %s:%i"
#define TEXT_CURSVRNOTINLIST	"Current Server      (not in serverlist)"
#define TEXT_SVRONTIME		"Server Ontime       %s"
#define TEXT_SVRMODE		"Mode                +%s"
#define TEXT_LINKPORT		"Linkport            %i %s"
#define TEXT_CURTIME2		"Current Time        %s"
#define TEXT_STARTED		"Started             %s"
#define TEXT_UPTIME		"Uptime              %s"
#define TEXT_VERSION		"Version             %s (%s)"
#define TEXT_FEATURES		"Features            %s"
#define TEXT_CMDCHAR		"Command Char        %c"
#define TEXT_COREEND		"\n[\037End of Core\037]"
#define TEXT_UHINSHIT		"%s is in my shitlist already for this channel"
#define TEXT_NOSHITNOACCESS	"Unable to shit %s, insufficient access"
#define TEXT_USERSHITON		"The user has been shitted as %s on %s"
#define TEXT_SHITWILLEXPIRE	"The shitlist will expire: %s"
#define TEXT_SHITALLCLEAR	"Shitlist has been cleared"
#define TEXT_PARTYLINE		"[\037Partyline\037]\n"
#define TEXT_UNKNOWNSVR		"(unknown server)"
#define TEXT_NOTCON		"(not connected)"
#define TEXT_ENDLIST		"\n[\037End of List\037]"
#define TEXT_PARTYECHOON	"Partyline echo is now On"
#define TEXT_PARTYECHOOFF	"Partyline echo is now Off"
#define TEXT_LSTCMDSHEADER	"[\037Last %i Commands\037]\n"
#define TEXT_OUTYOUGO		"Out you go!"
#define TEXT_SVRATTEMPTADD	"Attempting to add server %s to server list"
#define TEXT_SVRNOSVRS		"No servers in serverlist!"
#define TEXT_NOMATCHSVRPORT	"No matching entries was found for %s:%i"
#define TEXT_NOMATCHSVR		"No matching entries was found for %s:*"
#define TEXT_SVRDELETED		"Server has been deleted: %s:%i"
#define TEXT_SVRSEVERAL		"Several entries for %s exists, please specify port also"
#define TEXT_SVRLISTHEADER	"[\037Serverlist\037]\n\n\037Server Name\037%28s    \037Last Connect\037","\037Port\037"
#define TEXT_SVRNOAUTH		"(no authorization)"
#define TEXT_SVRKLINE		"(K-lined)"
#define TEXT_SVRCLASSFULL	"(connection class full)"
#define TEXT_SVRTIMEOUT		"(connection timed out)"
#define TEXT_SVRNOCON		"(unable to connect)"
#define TEXT_SVRDIFFPORT	"(use a different port)"
#define TEXT_SVRNEVER		"(never)"
#define TEXT_SVRLISTFOOTER	"\n[\037End of Serverlist\037]"
#define TEXT_BACK		"No longer set /away"
#define TEXT_AWAY		"Now set /away"
#define TEXT_DCCKILL		"Hasta la vista!"
#define TEXT_ACCESSDENIEDON	"Access denied (you have no access on %s)"
#define TEXT_ACCESSDENIEDHIGH	"Access denied (%s has higher access than you)"
#define TEXT_USERMOD		"User %s has been modified"
#define TEXT_USERMODNO		"User %s is unmodified"
#define TEXT_CURCMDCHAR		"Current commandchar is '%c'"
#define TEXT_CMDCHARSET		"Commandchar set to '%c'"
#define TEXT_CMDCHARSINGLE	"Please specify a single character to be used as commandchar"
#define TEXT_HOSTNAMEMAX	"Hostnames cannot be longer than %i chars"
#define TEXT_VHOSTSETTO		"Virtual host has been set to %s"
#define TEXT_VHOSTRESTARTBOT	"Bot must reconnect for this change to take effect"
#define TEXT_GATESETTO		"WinGate has been set to %s:%i"
#define TEXT_SHUTDOWNBY		"Shutdown initiated by %s[%i], flatlining ..."

#define TEXT_TRV_WKTOP10	"This Weeks Top 10:"
#define TEXT_TRV_HINT1		"1st hint: "
#define TEXT_TRV_HINT2		"2nd hint: "
#define TEXT_TRV_HINT3		"3rd hint: "
#define TEXT_TRV_QSCORE		"%s   Question score: %i points\n"
#define TEXT_TRV_SECONDSLEFT	" seconds remaining.\n"
#define TEXT_TRV_ANSWERED	"Yes, %s! got the answer -> %s <- in %i seconds, and gets %i points!\n"
#define TEXT_TRV_INAROWWEEK	"%s has won %i in a row! Total score this Week: %i points\n"
#define TEXT_TRV_INAROWMONTH	"%s has won %i in a row! Total score this Week: %i points, and this Month: %i points\n"
#define TEXT_TRV_WONQUES	"%s has won the question! Total score this Week: %i points\n"
#define TEXT_TRV_WONQUESMONTH	"%s has won the question! Total score this Week: %i points, and this Month: %i points\n"
#define TEXT_TRV_TIMESUP	"Time's up! The answer was -> %s <-\n"
#define TEXT_TRV_BADQFILE	"Bad Question File. Read the file README.FEATURES to fix it."
#define TEXT_TRV_ACTIVE		"Trivia is already activated on"
#define TEXT_TRV_NOTRUNNING     "Trivia is not activated on"
#define TEXT_TRV_ANOTHERCHAN	"another channel"
#define TEXT_TRV_STARTING	"Trivia starting! Get ready..."
#define TEXT_TRV_SHUTDOWN	"Trivia shutting down..."
#define TEXT_TRV_STOPPED	"Trivia has been stopped!"
#define TEXT_TRV_CMDSTART	"start"
#define TEXT_TRV_CMDSTOP	"stop"



//#endif /* TEXT_H */