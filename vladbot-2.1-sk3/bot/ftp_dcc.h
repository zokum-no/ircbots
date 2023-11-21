#ifndef _FTP_DCC_H_
#define _FTP_DCC_H_

#ifdef NEXT
#  include <sys/types.h>
#  include <sys/dir.h>
#else
#  include <dirent.h>
#endif /* NEXT */
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>

/* 
 * I hate this.. 
 * Thanx for fixing Mozgy
 */
#ifndef _POSIX_SOURCE
#  ifndef _S_IFMT
#    define _S_IFMT 0170000
#  endif /* _S_IFMT */
#  ifndef _S_IFLNK
#    define _S_IFLNK 0120000
#  endif /* _S_IFLNK */
#  ifndef _S_IFSOCK
#    define _S_IFSOCK 0140000
#  endif /* _S_IFSOCK */
#  ifndef S_ISLNK
#    define S_ISLNK(mode)	(((mode) & (_S_IFMT)) == (_S_IFLNK))
#  endif /* S_ISLNK */
#  ifndef S_ISSOCK 
#    define S_ISSOCK(mode)	(((mode) & (_S_IFMT)) == (_S_IFSOCK))
#  endif /* S_ISSOCK */
#endif /* NOT _POSIX_SOURCE */

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
