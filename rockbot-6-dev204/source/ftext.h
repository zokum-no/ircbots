/* ftext.c */
char *read_word(FILE *ffile);
void translate_escape(char *s, int c);
void skipcomment(FILE *ffile);
int find_topic(FILE *ffile, char *topic);
char *get_ftext(FILE *ffile);
