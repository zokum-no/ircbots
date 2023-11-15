/* match.c */
int match(char *ma, char *na);
int matches(char *ma, char *na);
char *collapse(char *pattern);
int matches2(char *ma, char *na);
char *char2str(char c);
int bancmp(char *temp, char *temp2);
int bancmp3(char *temp, char *temp2);
int wldcmp(char *wildexp, char *regstr);
int wldwld(char *wild1, char *wild2);
int bancmp2(char *banmask, char *protmask);
int starcmp(char *banitem, char *protitem);
int wild_match_per(register unsigned char *m, register unsigned char *n);
int wild_match(register unsigned char *m, register unsigned char *n);
int wild_match_file(register unsigned char *m, register unsigned char *n);
