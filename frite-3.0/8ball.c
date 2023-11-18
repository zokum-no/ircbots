#include "config.h"
#include "vlad-ons.h"
#include "send.h"
#include "8ball.h"

void	do_8ball(char *from, char *to, char *rest)
{
	char buffer[MAXLEN];

	if(!rest)
	{
		send_to_user(from, "You need to ask me a yes or no question.");
		return;
	}
	switch (random() % 5) {
	case 0:
		sprintf(buffer, "Of Course!");
		break;
	case 1:
		sprintf(buffer, "Ahh .. No ..");
		break;
	case 2:
		sprintf(buffer, "Ask again later ..");
		break;	
	case 3:
		sprintf(buffer, "It is certain ..");
		break;
	case 4:
		sprintf(buffer, "Not a chance.");
		break;
	}
	if(ischannel(to))
		sendprivmsg(to, buffer);
	else	
		send_to_user(from, buffer);
}
void	do_dice(char *from, char *to, char *rest)
{
	int i,ii;
	switch (random() % 6) {
	case 0:
	  i=1;
	  break;
	case 1:
	  i=2;
	  break;
	case 2:
	  i=3;
	  break;
	case 3:
	  i=4;
	  break;
	case 4:
	  i=5;
	  break;
	case 5:
	  i=6;
	  break;
	}
	send_to_user(from, "I rolled a %i", i);
	switch (random() % 6) {
	case 0:
	  ii=1;
	  break;
	case 1:
	  ii=2;
	  break;
	case 2:
	  ii=3;
	  break;
	case 3:
	  ii=4;
	  break;
	case 4:
	  ii=5;
	  break;
	case 5:
	  ii=6;
	  break;
	}
	send_to_user(from, "You rolled a %i", ii);
	if(i==ii)
		send_to_user(from, "Its a tie!");
	else if(i>ii)
		send_to_user(from, "I Win!");
	else
		send_to_user(from, "You Win!");
}
