#include "misc.h"
#include <stdio.h>
#include <strings.h>

main()
{
	char	source[255];
	char	dest[255] = "/";
	char	*s = source;
	char	*p;
	char	*component;
	
	gets(s);

	while(component = get_token(&s, "/"))
	{
		printf("Component is \"%s\"\n", component);
		printf("Path is \"%s\"\n", dest);
		if(STREQUAL("..", component))
		{
			/* try to find start of last component */
			p = strrchr(dest, '/');
			/* If we're left with the root only ... */
			if(p != dest)
			{
				printf("Going lower ONE level\n");
				*p = '\0';
			}	
			else
			{
				*(p+1) = '\0';
				printf("Trying to get under root\n");
			}
		}
		else
		{
			printf("Adding component to path\n");
			if(!STREQUAL(dest, "/"))
				strcat(dest, "/");
			strcat(dest, component);
			p = dest + strlen(dest) - 1;
		}
	}
	printf("result: \"%s\"\n", dest);
}
