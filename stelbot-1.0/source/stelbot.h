#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <strings.h>
#include <string.h>
#include "channel.h"
#include "config.h"

typedef struct	server_struct
{
	char	servername[MAXLEN];
	int	serverport;
} serv_struc;

typedef	struct	stelbot_struct
{
	char		nick[MAXNICKLEN];
	char		ircname[MAXLEN];
	char		login[MAXLEN];
	char		channel[MAXLEN];
	serv_struc	servers[10];
	int		current;
	int		server_sock;
/*
 * To add:
 * - Channellist
 * - DCC list
 */
}	StelBotStruct;	



