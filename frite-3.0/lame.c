/**********
 **********  The Lame Filter            	Version 2.7.lame.lame.lame
 **********  by zorgo <zorgo@gnu.ai.mit.edu>    07/11/93 
 **********
 **********  This code is too lame to be Commercial,
 **********  too lame to be Shareware,
 **********  too lame to be Public Domain,
 **********  too lame to be GNU copylefted, 
 **********  it's Lameware!
 **********
**********/

#define MAXLAME 1000
/* #define LAME_SRAND 1 */
/* Some lamers have complained that 'time_t t' definition for srand
   barfed on compile, so now we use lame getpid() in srand 
   UNLESS you define LAME_SRAND */

#include <stdio.h>
#include <ctype.h>

#ifdef LAME_SRAND
#include <time.h>
#endif

void main(argc, argv)
int argc;
char *argv[];
{
	register int i;
	char buf[MAXLAME];

	if(argc > 1) {
		if( *argv[1]=='-' ) {
			usage();
			exit();
		}
		for(i=1; i<argc; i++) {
			sprintf(buf, "%s ", argv[i]);
			lamerize(buf);
		}
		printf("\n");  /* for lame unix */
		exit();
	}

	lame();
	exit();
}

/********************************************************************/
/********************************************************************/
lame()
{
	char ch, ch_buf[MAXLAME];
	register int i;

	ch_buf[0] = NULL;

	while(1) {
		ch = fgetc(stdin);
		if( ch == EOF ) return(-1);
		strncat(ch_buf, &ch, 1);

		if(ch=='\n') {
			lamerize(ch_buf);
			for (i=0; i<strlen(ch_buf); i++) {
				ch_buf[i] = NULL;
			}
		}
	}
}


/********************************************************************/
/********************************************************************/
lamerize(buf)
char buf[];
{
	register int j,  count=0;
	char out[MAXLAME];

#ifdef LAME_SRAND
	time_t t;
		srand((unsigned) time(&t));
#else 
		srand((unsigned) getpid());
#endif

		for(j=0; j<strlen(buf); j++) {
		    if( buf[j]=='y' && buf[j+1]=='o' && buf[j+2]=='u') {
				out[count++]='U'; j+=2;
		    }
		    else if( buf[j]=='o' || buf[j]=='O') {
		    	if( rand()%3==1 ) {
		    		out[count++]='('; out[count++]=')';
			} else out[count++]='0';
		    }
		    else if( buf[j]=='a' && buf[j+1]=='n' && buf[j+2]=='d'){
				out[count++]='&'; j+=2;
		    }
		    else if((buf[j]=='i' || buf[j]=='I')&&(rand()%5>=2)) {
				out[count++]='1';
		    }
		    else if( buf[j]=='l' && (rand()%3==1)) {
				out[count++]='|';
		    }
		    else if(buf[j]=='s'&&(buf[j+1]=='\0'||ispunct(buf[j+1]))){
				out[count++]='z'; out[count++]='z';
		    }
		    else if( (buf[j]=='e' || buf[j]=='E') &&(rand()%5 >= 3) ){
				out[count++]='3';
		    }
		    else if( rand()%5==1 ) out[count++]=toupper(buf[j]);
		    else out[count++]=buf[j];
		}

	out[count]=NULL;
	printf("%s", out);
	return(1);
}

/********************************************************************/
/********************************************************************/
usage()
{

	printf("\n***\n*** The Lamer Filter, Version 2.7\n");
	printf("***\n*** by zorgo <zorgo@gnu.ai.mit.edu>\n***\n***\n");
	printf("*** Usage:\n");
	printf("***   Starting LAME with no arguments will put it in continuous lamerize mode.\n");
	printf("***   Use Control-C to quit.\n");
	printf("***\n");
	printf("***   Starting LAME with arguments will lamerize everything on\n");
	printf("***   the command line, and then quit.\n");
	printf("***\n");
	printf("***   To lamerize a text file:\n");
	printf("***  Unix:   cat in_file | lame > out_file\n");
	printf("*** MSDOS:   type in_file | lame > out_file\n");
	printf("***\n");
	printf("***   To be lame in real time on IRC (ircII only):\n");
	printf("***          /alias l /exec -out lame \"$0-\"\n");
	printf("***          /l text_to_be_lamerized\n");
	return(1);
}



