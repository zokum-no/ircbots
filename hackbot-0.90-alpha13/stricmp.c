#include <ctype.h>
#include <stdio.h>

int my_stricmp(str1, str2)
char *str1;
char *str2;
{
	if (!str1 && !str2)
		return 0;
	else if (!str1 || !str2)
		return 1;
	while (*str1 && *str2 && (toupper(*str1) == toupper(*str2)))
	{
		str1++;
		str2++;
	}
	if (*str1 || *str2)
		return 1;
	return 0;
}

main (int argc, char **argv)
{
	printf("%i\n", my_stricmp(argv[1], argv[2]));
}




