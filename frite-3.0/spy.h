int add_spy_chan(char *name, char *nick);
int del_spy_chan(char *name, char *nicl);
void spy_list(char *from, char *name);
void do_spy(char *from, char *to, char *rest);
void do_rspy(char *from, char *to, char *rest);
void do_spylist(char *from, char *to, char *rest);
void do_spymsg(char *from, char *to, char *rest);
void do_nospymsg(char *from, char *to, char *rest);
