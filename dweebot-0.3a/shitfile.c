#include <stdlib.h>
#include <stdio.h>
#include <string.h>

extern char *crypt(char *, char *);

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

char *encode(passwd)
     char *passwd;
{
  static char saltChars[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789./";
  char salt[3];
  
  srandom(time(NULL));
  salt[0] = saltChars[random() % 64];
  salt[1] = saltChars[random() % 64];
  salt[2] = '\0';
  
  return crypt(passwd, salt);
}

int passmatch(plain, encoded)
     char *plain;
     char *encoded;
{
  char salt[3];
  
  salt[0] = encoded[0];
  salt[1] = encoded[1];
  salt[2] = '\0';
  printf("%s\n", crypt(plain, salt));
  printf("%i\n", !my_stricmp(crypt(plain, salt), encoded));
  return (!my_stricmp(crypt(plain, salt), encoded));
}

void main(argc, argv)
int argc;
char **argv;
{
   char shit[1024];
   strcpy(shit, encode(argv[1]));
   printf("%s\n", shit);
   printf("%i\n", passmatch(argv[2], shit));
 
}



