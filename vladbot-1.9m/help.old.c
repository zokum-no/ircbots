#include <stdlib.h>
#include <stddef.h>
#include "dcc.h"
#include "config.h"


char *standard_help[] =
{       "------------ Basic Help for " IRCNAME " version " VERSION " ------------",
        "These are the basic commands (prefixed by a !) you can give me.",
        "!help command for more info on this command and required levels",
        "---------------------------------------------------------------",
	" 	whoami		info		time		",	
	"	say		do		op		",
	"	giveop		spy		diedie		",
	"	useradd		shitadd		protadd		",	
	" 	userdel 	shitdel		protdel		",
	"	userlist	shitlist	protlist	",
	"	userwrite	shitwrite	protwrite	",
	"	channels	leave		join		",
	"	nick		fork		random		",
	"	massop		massdeop	masskick	",
	"	chat		send		files           ",
	"	flist		dodcc		listdcc		",
	"	killdcc						",
	"Further information is available about:",
	"	patterns	getting_access	dcc		",
	"	filetransfer",
	NULL
};

struct
{
	char	*help_item;
	char	*helptext[15];		/* max. 10 lines */
} help[] =
{
	{ "WHOAMI",
	{ "!WHOAMI shows you your current levels (user, shit and protlevel)",
	  "  ",
	  "See also: !USERLIST, !PROTLIST, !SHITLIST, levels, getting_access",
	  NULL } 
	},

	{ "INFO",
	{ "!INFO gives some information about " IRCNAME ".", 
	   NULL }
	},

	{ "TIME",
	{ "!TIME gives you the current time at the place the bot was started",
	   NULL }
	},

	{ "SAY",
	{ "!SAY makes me say something :)",
	  "Requires user/shitlevel 100/0.",
	  " ",
	  "See also: levels",
           NULL }
        },
 
        { "DO",
	{ "!DO makes the me do something. It will be \'quoted\' to the server",
	  "This requires correct user/shitlevel setings (100/0)",
          " ",
          "See also: levels",
           NULL }
        },
 
        { "OP",
	{ "!OP makes me try to give you op if you have the right", 
	  "user/shitlevel settings. (50/0)",
          " ",
	  "See also: !GIVEOP, levels",
	  NULL }
	},

	{ "GIVEOP",
	{ "!GIVEOP wil give a maximum of three people operatorstatus at once.",
	  "This requires correct user/shitlevel setings (50/0)",
	  " ",
	  "See also: !OP, levels",
          NULL }
        },
 
	{ "SPY",
	{ "!SPY should not be used...",
	   NULL }
        },

	{ "DIEDIE",
	{ "!DIEDIE makes the bot signoff. This should be used with care,",
	  "and any changes in the user/shit/protlist should be written to",
	  "disk first. Requires user/shitlevel 150/0.",
	  NULL }
	},

	{ "USERADD",
	{ "!USERADD adds a user with a specified level to the userlist",
	  "For this operation, a user/shitlevel of (100/0) is required",
	  " ",
	  "SEE ALSO: !USERLIST, USERDEL, patterns, levels, getting_access",
	  NULL }
	},

	{ "USERDEL",
	{ "!USERDEL can be used to remove someone from the userlist.",
	  "This requires a user/shitlevel of (100/0)",
	  " ",
	  "SEE ALSO: !USERLIST, !USERADD, levels, getting_access",
          NULL }
        },
 
	{ "USERLIST",
	{ "!USERLIST will show a list of all registred users. For this",
	  "operation, a user/shitlevel of (0/0) is required",
	  " ",
	  "SEE ALSO: !USERDEL, !USERWRITE, levels, getting_access",
	  NULL }
	},

	{ "USERWRITE",
	{ "!USERWRITE will write the userlist to disk. For this operation",
	  "a user/shitlevel of (100/0) is required",
	  " ",
	  "SEE ALSO: !USERLIST, !USERADD, !USERDEL, levels",
	  NULL }
	},

        { "SHITADD",
        { "!SHITADD adds a user with a specified level to the shitlist",
          "For this operation, a user/shitlevel of (100/0) is required",
          " ",
          "SEE ALSO: !SHITLIST, SHITDEL, patterns, levels, getting_access",
          NULL }
        },

        { "SHITDEL",
        { "!SHITDEL can be used to remove someone from the shitlist.",
          "This requires a user/shitlevel of (100/0)",
          " ",
          "SEE ALSO: !SHITLIST, !SHITADD, levels, getting_access",
          NULL }
        },

        { "SHITLIST",
        { "!SHITLIST will show a list of all users in the shitlist. For",
          "this operation, a user/shitlevel of (0/0) is required",
          " ",
          "SEE ALSO: !SHITDEL, !SHITWRITE, levels, getting_access",
          NULL }
        },

        { "SHITWRITE",
        { "!SHITWRITE will write the shitlist to disk. For this operation",
          "a user/shitlevel of (100/0) is required",
          " ",
          "SEE ALSO: !SHITLIST, !SHITADD, !SHITDEL, levels",
          NULL }
        },
	
        { "PROTADD",
        { "!PROTADD adds a user with a specified level to the protectionlist",
          "For this operation, a user/shitlevel of (100/0) is required",
          " ",
          "SEE ALSO: !PROTLIST, PROTDEL, patterns, levels, getting_access",
          NULL }
        },

        { "PROTDEL",
        { "!PROTDEL can be used to remove someone from the protectionlist.",
          "This requires a user/protlevel of (100/0)",
          " ",
          "SEE ALSO: !PROTLIST, !PROTADD, levels, getting_access",
          NULL }
        },

        { "PROTLIST",
        { "!PROTLIST will show a list of all users in the protectionlist.",
          "For this operation, a user/shitlevel of (0/0) is required",
          " ",
          "SEE ALSO: !PROTDEL, !PROTWRITE, levels, getting_access",
          NULL }
        },

        { "PROTWRITE",
        { "!PROTWRITE will write the protectionlist to disk. For this",
          "operation, a user/shitlevel of (100/0) is required",
          " ",
          "SEE ALSO: !PROTLIST, !PROTADD, !PROTDEL, levels",
          NULL }
        },

        { "OPEN",
        { "!OPEN <channel> will cause me (if I am able) to open up the",
          "<channel> so you can join it. Actually, all I do is try to make",
          "the channel -i, -k and -l.",
          "You need to have user/shit levels of 100/0 for this command.",
          " ",
          "SEE ALSO: !INVITE, getting_access",
          NULL }
        },

        { "INVITE",
        { "!INVITE <channel> will cause me (if I am able) to invite you",
          "to the <channel>. This might be usefull when the <channel> is an",
          "invite only channel. You need user/shit levels of 50/0 for this",
          "opperation to succeed.",
          " ",
          "SEE ALSO: !OPEN, getting_access",
          NULL }
        },

	{ "DCCDO",
	{ "!DCCDO will create dcc connections between you and me.",
	  "You must specify SEND file to receive file or CHAT to",
	  "make me start a chatconnection with you.",
	  "I will also respond to dcc connections initiated by you. This way",
	  "you can upload files (i.e. /dcc send " IRCNAME " foo.txt)."
	  " ",
	  "SEE ALSO: !FILES, !CHAT, !SEND, filetransfer",
	  NULL }
	},  

	{ "SEND",
	{ "!SEND <file> will cause me to send <file> to you. In order",
	  "to receive <file>, you will need to type the following line:",
	  "/dcc get " IRCNAME " <file>",
	  "I will ask you to type this after I prepared the connection.",
	  " ",
	  "SEE ALSO: !FILES, dcc, filetransfer",
	  NULL }
	},

	{ "CHAT",
	{ "!CHAT will cause me to request a fast dcc chat connection",
	  "between you and me. In order to establish this connection, you",
	  "can also type the following line instead of !CHAT:",
	  "/dcc chat " IRCNAME " ",
	  " ",
	  "SEE ALSO: dcc",
	  NULL }
	},

	{ "FILES",
	{ "!FILES will show you a short listing of the available files.",
	  " ",
	  "SEE ALSO: !DCCDO, !SEND, filetransfer",
	  NULL }
	},

	{ "KILLDCC",
	{ "KILLDCC may be used to remove old, inactive dcc connections",
	  "from the dcc-list. Three parameters are required, from, type ",
	  "and desc. Example:",
	  "!KILLDCC Richie_B!~richie@tc0.chem.tue.nl CHAT chat",
	  "This requires user/shitlevel of (125/0)",
	  " ",
	  "SEE ALSO: !LISTDCC",
	  NULL }
	},

	{ "dcc",
	{ "Dcc stands for direct client to client, and is used for fast",
	  "connections between two IRC clients. There are various types of",
	  "dcc-connections. Dcc-chat is used for fast (and safe) text",
	  "transfer, dcc-get and dcc-send are used for client-to-client",
	  "data transfer. In order to establish a dcc connection, both",
	  "parties will need to request the connection. If you request any",
	  "dcc connection to me, I will respond to it.",
	  "Dcc connections are requested by typing:",
	  "/dcc <dcc-type> <nick-of-party> <additional-parameters>",
	  "In order to close a dcc connection, type:",
	  "/dcc close <dcc-type> <nick-of-party>",
	  " ",
	  "SEE ALSO: !SEND, !CHAT, filetransfer, /help dcc",
	  NULL }
	},

	{ "levels",
	{ "Levels are used to determine which users can make me do what.",
	  "The most important use of the levels are:",
	  "userlevel >=  50 : user will be made channel operator by me",
	  "userlevel >= 100 : user may modify user/shit/prot lists",
	  "userlevel >= 150 : user may use all my functions",
	  "shitlevel >=  50 : user won\'t be opped, and cannot modify",
	  "                 : the various list, regardless of the userlevel",
	  "shitlevel >= 100 : user will be kicked and banned when the",
	  "                 : channel is joined",
	  "protlevel == 100 : I will not massdeop or masskick these users",
	  "If you will try to give me a command you don't have the right",
	  "level for, I will simply not understand the command.",
	  " ",
	  "SEE ALSO: !WHOAMI, !USERADD, !SHITADD, !PROTADD, getting_access",
	  NULL }
	},

	{ "getting_access",
	{ "getting_access: To get any level > 0, you need to contact",
	  "someone who has a level >= 100. Use !USERLIST to get a list",
	  "of users.",
	  " ",
	  "SEE ALSO: !USERLIST",
	  NULL }
	},

	{ "patterns",
	{ "Pattern strings should have the pattern: nick!user@host.domain",
	  "Please use wildcards so the person is always recognized.",
	  "Beware that people often use different machines, and different",
	  "nicks. Due to the 2.8.x servers, add a \"*\" before the username.",
	  " ",
	  "Example: *!*richie@*.chem.tue.nl",
	  " ",
	  "SEE ALSO: !USERLIST",
	  NULL }
	},

	{ "filetransfer",
	{ "You can make filetransfers with me without any quota.",
	  "The transfers are done by using dcc. All up and downloads are",
	  "logged for statistic purposes. Don't transfer if you don't like",
	  "this policy. To upload files, type the following:",
	  "/dcc send " IRCNAME " <filename>",
	  " ",
	  "SEE ALSO: !FILES, !SEND, dcc",
	  NULL }
	},


	{ NULL, 
	{ NULL }
	}
};


void	show_help( from, to, rest )
char	*from;
char	*to;
char	*rest;

{
	int 	i, j;

	if( rest == NULL )
	{
		for( i = 0; standard_help[i] != NULL; i++ )
			send_to_user( from, standard_help[i] );
		return;
	}

	for( i = 0; help[i].help_item != NULL; i++ )
	{
		if( !strcasecmp( help[i].help_item, rest ) ||
		    !strcasecmp( help[i].help_item, rest+1 ) )
		{
			send_to_user( from, "   " );
			for( j = 0; help[i].helptext[j] != NULL; j++ )
				send_to_user( from, help[i].helptext[j] );
			return;
		}
	}
	/*
	 * The command wasn't found. Perhaps we should dump some 
	 * standardhelp. Or an errormessage.
	 */
	send_to_user( from, "No help available for \"%s\"", rest );
}
