void bot_reply(char *to, int num);
int numchar(char *string, char c);
int  exist(char *filename);
void steal_channels(void);
void save_files(char *from);
void shitkick(char *who, char *channel);
void showspymsg(char *person, char *msg);
char *left(char *string, int i);
void mode3(char *channel, char *mode, char *data);
char *cluster(char *userhost);
void make_process(char *name, char *args);
char *fix_channel(char *channel);
char *find_channel ( char *to, char **rest );
char *find_person( char *from, char *person );
void takeop( char *channel, char *nicks );
void sep_userhost( char *userhost, char *nick, char *user, char *host, char *machine);
void mode3(char *channel, char *mode, char *data);
void screw_ban( char *channel, char *who, int num);
void alt_ques( char *string);
char *rand_ques(char *string, int num);
void rand_quesfunc(char *string);
void massprot_action(char *from, char *channel, char *buffer);
void prot_action(char *from, char *protnuh, char *channel);
void giveop( char *channel, char *nicks );
void ban_user( char *channel, char *who );
void signoff( char *from, char *reason );
char *getuserhost2(char *nick);
