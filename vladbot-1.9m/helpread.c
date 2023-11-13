#include <stdio.h>
#include "ftext.h"
#include "config.h"

char	current_nick[MAXNICKLEN];

void	show_help( topic )
char	*topic;

{
	int 	i, j;
	FILE	*f;
	char	*s;

	f=fopen("help.bot", "r");
	
	if(!find_topic( f, topic ))
		printf("No help available for \"%s\"\n", topic);
	else
		while(s=get_ftext(f))
			printf("%s\n", s);
	fclose(f);
}

int	main(argc, argv)
int	argc;
char	**argv;

{
	int	i;

	strcpy(current_nick, "helpread");
	for(i=1; i<argc; i++)
		show_help(argv[i]);
}
