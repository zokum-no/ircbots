/*
 * ftp-dcc.c - ftp-interface for dcc
 * Copyright (C) 1993-94 VladDrac (irvdwijk@cs.vu.nl)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Provide a ftp-ish interface for dcc
 */

#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>

#include "config.h"
#include "debug.h"
#include "fnmatch.h"
#include "ftp_dcc.h"
#include "misc.h"
#include "session.h"
#include "vladbot.h"

extern	botinfo	*currentbot;

void	pwd(char *from)
{
	sendreply("Current working directory: %s", get_cwd(from));
}

char	*real_root(char *path, char *buf)
/*
 * Converts path with it's relative root to a real path,
 * i.e. /etc -> /home/bot/tmp/etc
 */
{
	sprintf(buf, "%s/%s", currentbot->downloaddir, path);
	return buf;
}

int	rr_stat(char *name, struct stat *buf)
/* Like stat(), but with real_root in stead of the relative root */
{
	char	*buffer;
	int	result;

	buffer = (char *)malloc(strlen(name)+strlen(currentbot->downloaddir)+2);
	result = stat(real_root(name, buffer), buf);
	free(buffer);
	return result;
}

int	rr_access(char *name, int mode)
/* Like access(), but with correct root */
{
	char	*buffer;
	int	result;

	buffer = (char *)malloc(strlen(name)+strlen(currentbot->downloaddir)+2);
	result = access(real_root(name, buffer), mode);
	free(buffer);
	return result;
}

DIR	*rr_opendir(char *dirname)
/* Like opendir(), but with correct root */
{
	char	*buffer;
	DIR	*result;

	buffer = (char *)malloc(strlen(dirname)+strlen(currentbot->downloaddir)+2);
	result = opendir(real_root(dirname, buffer));
	free(buffer);
	return result;
}

char	*make_dir(char *old, char *new, char *dirbuffer)
/*
 * Constructs a new dir using old and new. Leading ../'s will be translated
 * (for safety). Result will be placed in buffer.
 */
{
	char	tmp[MAXLEN] = "/";
	char	*dest;
	char	*s, *p;
	char	*component;
	

	if(*new != '/')
		sprintf(dirbuffer, "%s/%s", old, new);
	else
		strcpy(dirbuffer, new);

	s = dirbuffer;
	dest = tmp;

	while(component = get_token(&s, "/"))
		if(STREQUAL("..", component))
		{
			p = strrchr(dest, '/');
			if(p != dest)
				*p = '\0';
			else
				*(p+1) = '\0';
		}
		else if(!STREQUAL(".", component))
		{
			if(!STREQUAL(dest, "/"))
				strcat(dest, "/");
			strcat(dest, component);
			p = dest + strlen(dest) - 1;
		}

	strcpy(dirbuffer, tmp);	
	return dirbuffer;
}

void	sort_dir(char *list[], int n)
/*
 * Sorts array list which has n elements (bubble sort)
 */
{
	int	j, k, sorted;
	
	for(k=n-1, sorted = FALSE; k>0 && !sorted; k--)
	{
		sorted = TRUE;
		for(j=0; j<k; j++)
			if(strcmp(list[j], list[j+1])>0)
			{
				char	*tmp;
	
				mstrcpy(&tmp, list[j+1]);
				free(list[j+1]);
				mstrcpy(&(list[j+1]), list[j]);
				free(list[j]);
				mstrcpy(&(list[j]), tmp);
				free(tmp);
				sorted = FALSE;
			}
	}
}

void	output_dir(char *from, char *path, char *list[], int n)
/*
 * This routine will show a fancy listing of files in list[] that
 * match mask
 */
{
	int	i;
	struct	stat	buf;
	char	real_path[MAXLEN];
	char	modebits[11];	/* drwxr-xr-x */
	char	file_type;	/* link, dir, ... */
	
	for(i=0; i<n; i++)
	{
		if(rr_stat(make_dir(path, list[i], real_path), &buf) == -1)
		{
#ifdef DBUG
			debug(ERROR, "output_dir(): Could not stat file %s!", list[i]);
#endif
			continue;		/* should not happen! */
		}
		file_type = S_ISDIR(buf.st_mode)?'d':
			    S_ISCHR(buf.st_mode)?'c':
			    S_ISBLK(buf.st_mode)?'b':
			    S_ISREG(buf.st_mode)?'-':
			    S_ISLNK(buf.st_mode)?'l':
			    S_ISSOCK(buf.st_mode)?'s':
			    S_ISFIFO(buf.st_mode)?'f':
			    '-';
		sprintf(modebits, "%c%c%c%c%c%c%c%c%c%c", file_type,
			buf.st_mode&S_IRUSR?'r':'-',
			buf.st_mode&S_IWUSR?'w':'-',
			buf.st_mode&S_IXUSR?'x':buf.st_mode&S_ISUID?'s':'-',
			buf.st_mode&S_IRGRP?'r':'-',
			buf.st_mode&S_IWGRP?'w':'-',
			buf.st_mode&S_IXGRP?'x':buf.st_mode&S_ISGID?'s':'-',
			buf.st_mode&S_IROTH?'r':'-',
			buf.st_mode&S_IWOTH?'w':'-',
			buf.st_mode&S_IXOTH?'x':'-');
		sendreply("%s %8u %20s %s%s", modebits, buf.st_size, 
		          time2str(buf.st_mtime), list[i],
			     S_ISDIR(buf.st_mode)?"/":
			     S_ISLNK(buf.st_mode)?"@":
			     (buf.st_mode&S_IXUSR)?"*":
			     "");

	}
}

int	read_dir(char *list[], char *path, char *pattern)
/* Read a directory and places all files that match pattern in list.
   returns the number of files read 
 */
{
	DIR	*dir;
	struct	dirent	*dir_entry;
	int	n=0;

	dir = rr_opendir(path);
	while((n < LISTSIZE) && (dir_entry = readdir(dir)))
		if(!STREQUAL(dir_entry->d_name, "..") &&
	           !STREQUAL(dir_entry->d_name, ".") &&
	           !fnmatch(pattern, dir_entry->d_name, FNM_PERIOD))
			mstrncpy(&(list[n++]), dir_entry->d_name, 127);
	closedir(dir);
	return n;
}

void	close_dir(char *list[], int n)
{
	int	i;

	for(i = 0; i < n; i++)
		free(list[i]);
}

void	do_ls(char *from, char *path)
{
	struct	stat	buf;
	char	real_path[MAXLEN];
	char	*dir_list[128];
	int	n;
	
	make_dir(get_cwd(from), path, real_path);
	if(rr_stat(real_path, &buf) != -1)
	{	/* existing file/directory */
		if(S_ISDIR(buf.st_mode))	/* list directory	*/
			if(rr_access(real_path, R_OK) || rr_access(real_path, X_OK))
			{
				sendreply("%s: Permission denied", path);
				return;
			}
			else
				n = read_dir(dir_list, real_path, "*");
		else	/* a path with file */
		{
			char	*file;

			file = strrchr(real_path, '/');
			*(file++) = '\0';
			if(rr_stat(real_path, &buf) == -1)
			{
				sendreply("%s: No such file or directory", 
				          path);
				return;
			}
			if(rr_access(real_path, R_OK))
			{
				sendreply("%s: Permission denied", path);
				return;
			}
			n = read_dir(dir_list, real_path, file);
		}
	}
	else	/* path with pattern or typo */
	{
		char	*pattern;

		pattern = strrchr(real_path, '/');
		*(pattern++) = '\0';
		if(rr_stat(real_path, &buf) == -1)
		{
			sendreply("%s: No such file or directory", path);
			return;
		}
		if(rr_access(real_path, R_OK) || rr_access(real_path, X_OK))
		{
			sendreply("%s: Permission denied", path);
			return;
		}	
		if((n = read_dir(dir_list, real_path, pattern)) == 0)
		{
			sendreply("%s: No such file or directory", path);
			return;
		}
	}
	sort_dir(dir_list, n);
	output_dir(from, real_path, dir_list, n);
	close_dir(dir_list, n);
}

void	send_file(char *from, char *name)
{
	struct	stat	buf;
	char	real_path[MAXLEN];
	char	sendfile[MAXLEN];
	char	rr_sendfile[MAXLEN];
	char	*dir_list[128];
	int	n, i;
	
	botlog(DCCLOGFILE, "DCC get from %s: %s received", from, name);

	make_dir(get_cwd(from), name, real_path);
	if(rr_stat(real_path, &buf) != -1)
	{
		if(S_ISDIR(buf.st_mode))	/* list directory	*/
			if(rr_access(real_path, R_OK) || rr_access(real_path, X_OK))
			{
				sendreply("%s: Permission denied", name);
				return;
			} 
			else /* user is grabbing entire directory */
				n = read_dir(dir_list, real_path, "*");
		else	/* seperate path and pattern (pattern can be filename) */
		{
			char	*file;

			file = strrchr(real_path, '/');
			*(file++) = '\0';
			if(rr_stat(real_path, &buf) == -1)
			{
				sendreply("%s: No such file or directory", name);
				return;
			}
			if(rr_access(real_path, R_OK) || rr_access(real_path, X_OK))
			{
				sendreply("%s: Permission denied", name);
				return;
			}			
			if((n = read_dir(dir_list, real_path, file)) == 0)
				sendreply("%s: No such file or directory", name);
		}
	}
	else	/* path with pattern or typo */
	{
		char	*pattern;

		pattern = strrchr(real_path, '/');
		*(pattern++) = '\0';
		if(rr_stat(real_path, &buf) == -1)
		{
			sendreply("%s: No such file or directory", name);
			return;
		}
		if(rr_access(real_path, R_OK) || rr_access(real_path, X_OK))
		{
			sendreply("%s: Permission denied", name);
			return;
		}		
		if((n = read_dir(dir_list, real_path, pattern)) == 0)
		{
			sendreply("%s: No such file or directory", name);
			return;
		}
	}

	dcc_sendlist(from, real_path , dir_list, n);
	for(i=0; i<n; i++)
		free(dir_list[i]);
}

int	send_file_from_list(char *user, char *path, char *file)
/*
 * Does some check on file, returns 0 on error (no file, no access etc)
 * or send the file and returns 1
 */
{
	struct	stat	buf;
	char	sendfile[MAXLEN];
	char	rr_sendfile[MAXLEN];
	
	if(rr_stat(make_dir(path, file, sendfile), &buf) == -1)
		return FALSE;	
	if(S_ISDIR(buf.st_mode))
		sendreply("%s: Is a directory", file);
	else if(rr_access(sendfile, R_OK))
		sendreply("%s: Permission denied", file);
	else
	{
		dcc_sendfile(user, real_root(sendfile, rr_sendfile));
		sendreply("Now please type: /dcc get %s %s", 
			  currentbot->nick, file);
		return TRUE;
	}
	return FALSE;	
}

void	do_chdir(char *from, char *new_dir)
{
	char	real_path[MAXLEN];
	struct	stat	buf;
	
	make_dir(get_cwd(from), new_dir, real_path);

	if(rr_stat(real_path, &buf)!= -1)
	{
		if(S_ISDIR(buf.st_mode))
		{
			if(rr_access(real_path, R_OK) || rr_access(real_path, X_OK))
			{
				sendreply("%s: Permission denied", new_dir);
				return;
			}
			change_dir(from, real_path);
			sendreply("Current working directory: %s", get_cwd(from));
		}
		else
			sendreply("%s: Not a directory.", new_dir);
	}	
	else
		sendreply("%s: No such file or directory.", new_dir);
}

FILE	*openindex(char *from, char *name)
{
	char	fname[MAXLEN];
	char	rr_buf[MAXLEN];
	struct	stat	buf;

	make_dir(get_cwd(from), name, fname);
	if(rr_stat(fname, &buf) != -1)
		if(S_ISREG(buf.st_mode))
			return fopen(real_root(fname, rr_buf), "r");
	return NULL;
}
