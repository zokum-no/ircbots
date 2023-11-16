#ifndef _CTCP_H_


void    on_ctcp( char *from, char *to, char *ctcp_msg );
void    ctcp_finger( char *from, char *to, char *rest );
void    ctcp_version( char *from, char *to, char *rest );
void    ctcp_clientinfo( char *from, char *to, char *rest );
void    ctcp_dcc( char *from, char *to, char *rest );
void    ctcp_ping( char *from, char *to, char *rest );
void    ctcp_source( char *from, char *to, char *rest );
void    ctcp_action( char *from, char *to, char *rest );
void    ctcp_unknown( char *from, char *to, char *ctcp_command );
void    ctcp_cmdchar(char *from, char *to, char *ctcp_command);
#endif






