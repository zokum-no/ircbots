#include "config.h"
/* misc.c */
void Debug(int Level, char *fmt, ...);
void safecopy(char *target, char *source, int num);
int BannedWordOk(int kslevel, int userlevel);
int AddyOk(char *Address);
int ismask(char *mask);
int IsAllDigits(char *string);
int KeyOk(char *Key);
char *UpCase(char *Str);
int GetModeNum(char *Mode);
void ConvertModeString(char *ModeString, unsigned int *PlusModes, unsigned int *MinModes, char **Key, char **limit);
char *strlwr(char *string);
char *getuserhost(char *nick);
int ischannel(char *channel);
char *strcasestr(char *s1, char *s2);
char *getnick(char *nick_userhost);
char *gethost(char *nick_userhost);
char *time2str(time_t time);
char *idle2str(long time);
char *strip(char *temp, char c);
char *fixuser(char *temp);
USER_list *finduserbynick(char *nick);
