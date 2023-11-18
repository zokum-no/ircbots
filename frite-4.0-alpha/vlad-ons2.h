void	do_nickflood(char *from, char *to, char *rest);
void	do_keybomb(char *from, char *to, char *rest);
void	do_bulletin(char *from, char *to, char *rest);
void	do_away(char *from, char *to, char *rest);
void	do_reboot(char *from, char *to, char *rest);
void	do_tsunami(char *from, char *to, char *rest);
void    do_tsuban(char *from, char *to, char *rest);
void	do_tsusignoff(char *from, char *to, char *rest);
void	do_dccbomb(char *from, char *to, char *rest);
void do_showidle(char *from, char *to, char *rest);
int toggle(char *from, int *bleah, char *what, ...);
int stats(char *from, int *bleah, char *what, ...);
void	do_status(char *from, char *to, char *rest);
void do_cool(char *from, char *to, char *rest);
void    do_ping(char *from, char *to, char *rest);
void    do_getch(char *from, char *to, char *rest);
void    do_cmdchar(char *from, char *to, char *rest);
void	do_userhost(char *from, char *to, char *rest);

