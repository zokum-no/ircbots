#ifndef _CTCP_H_


void    on_ctcp( char *from, char *to, char *ctcp_msg );
void    ctcp_version( char *from, char *to, char *rest );
void    ctcp_trueversion( char *from, char *to, char *rest );
void    ctcp_dcc( char *from, char *to, char *rest );
void    ctcp_ping( char *from, char *to, char *rest );

#endif






