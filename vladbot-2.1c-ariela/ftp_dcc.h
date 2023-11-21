#ifndef _FTP_DCC_H_
#define _FTP_DCC_H_

#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>

void	pwd(char *from);
char	*real_root(char *path, char *buf);
int	rr_stat(char *name, struct stat *buf);
int	rr_access(char *name, int mode);
DIR	*rr_opendir(char *dirname);
char	*make_dir(char *old, char *new, char *dirbuffer);
void	sort_dir(char *list[], int n);
void	output_dir(char *from, char *path, char *list[], int n);
int	read_dir(char *list[], char *path, char *pattern);
void	close_dir(char *list[], int n);
void	do_ls(char *from, char *path);
void	send_file(char *from, char *name);
void	do_chdir(char *from, char *new_dir);
FILE    *openindex(char *from, char *name);

#endif
