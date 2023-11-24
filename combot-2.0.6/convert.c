/*
 * fileio.c - file io functions
 * (c) 1994 CoMSTuD (cbehrens@iastate.edu)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdio.h>
#include <string.h>

#define FALSE 0
#define MAXLEN 255
#define TRUE 1

char *get_token(src, token_sep)
char **src;
char *token_sep;
{
        char    *tok;

        if (!(src && *src && **src))
                return NULL;

        while(**src && strchr(token_sep, **src))
                (*src)++;

        if(**src)
                tok = *src;
        else
                return NULL;

        *src = strpbrk(*src, token_sep);
        if (*src)
        {
                **src = '\0';
                (*src)++;
                while(**src && strchr(token_sep, **src))
                        (*src)++;
        }
        else
                *src = "";
        return tok;
}

char *terminate(string, chars)  /* terminates a string if a char is in it */
char *string;
char *chars;
{
        char *ptr;

        if (!string || !chars)
         return "";
        while (*chars)
        {
        if (ptr = strrchr(string, *chars))
                *ptr = '\0';
        chars++;
        }
        return string;
}


int  freadln(stream, lin)
FILE *stream;
char *lin;
{
	char *p;

		do
			p = fgets(lin, MAXLEN, stream);
		while (p && (*lin == '#'));

		if (!p)
			return FALSE;

		terminate(lin, "\n\r");
/*      p = stripl(lin, " ");
		strcpy(lin, p);        */

	return TRUE;
}

void do_ul_convert(infilename, outfilename)
char *infilename;
char *outfilename;
{
	FILE *ptr, *ptr2;
	char buffer[MAXLEN];
	char *temp=buffer;
	char *uh, *level, *aop, *prot, *channel, *passwd;

	if (!(ptr=fopen(infilename, "rt")))
		return;
	if (!(ptr2=fopen(outfilename, "at")))
	{
		fclose(ptr);
		return;
	}
	while (!feof(ptr) && freadln(ptr, buffer))
	{
		temp=buffer;
		uh = get_token(&temp, " ");
		level = get_token(&temp, " ");
		aop = get_token(&temp, " ");
		prot = get_token(&temp, " ");
		channel = get_token(&temp, " ");
		passwd = get_token(&temp, " ");
		fprintf(ptr2, "User=%s\nLevel=%s\nAop=%s\n",
			uh, level, aop);	
		fprintf(ptr2, "Prot=%s\nChannel=%s\nPassword=%s\n",
			prot, channel, passwd && *passwd ? passwd : "");
	}
	fclose(ptr);
	fclose(ptr2);
}

void do_sl_convert(infilename, outfilename)
char *infilename;
char *outfilename;
{
        FILE *ptr, *ptr2;
        char buffer[MAXLEN];
        char *temp=buffer;
        char *uh, *level, *who, *thetime, *expire, *channel, *passwd;

        if (!(ptr=fopen(infilename, "rt")))
                return;
        if (!(ptr2=fopen(outfilename, "at")))
        {
                fclose(ptr);
                return;
        }
        while (!feof(ptr) && freadln(ptr, buffer))
        {
                temp=buffer;
                uh = get_token(&temp, " ");
                level = get_token(&temp, " ");
                channel = get_token(&temp, " ");
		who = get_token(&temp, " ");
		thetime = get_token(&temp, " ");
		expire = get_token(&temp, " ");
                fprintf(ptr2, "Shit=%s\nLevel=%s\nChannel=%s\n",
                        uh, level, channel);
                fprintf(ptr2, "Who=%s\nTime=%s\nExpire=%s\nReason:%s\n",
                        who, thetime, expire, temp ? temp : "");
        }
        fclose(ptr);
        fclose(ptr2);
}

int main(argc, argv)
int argc;
char **argv;
{
	char filename[MAXLEN];
	if (argc != 3)
	{
		printf("Usage: convert <{botnick}> <outfile>\n");
		exit(0);
	}
	strcpy(filename, argv[1]);
	strcat(filename, ".userlist");
	printf("Converting %s to %s...", filename, argv[2]);
	do_ul_convert(filename, argv[2]);
	printf("Done.\n");
	strcpy(filename, argv[1]);
	strcat(filename, ".shitlist");
	printf("Converting %s to %s...", filename, argv[2]);
	do_sl_convert(filename, argv[2]);
	printf("Done.\n");
	return (0);
}

