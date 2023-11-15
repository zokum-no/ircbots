#ifdef PYTHON
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#ifndef MSDOS
#include <strings.h>
#endif
#include <time.h>
#include <signal.h>
#include "config.h"

#include "rockbot.h"
#include "channel.h"
#include "userlist.h"
#include "dcc.h"
#include "ftext.h"
#include "misc.h"
#include "commands.h"
#include "script.h"

extern char cmdchar;
extern char *PYNICK;

ALIAS_list *Script = NULL;
ALIAS_list *Script_last = NULL;


/* These are just for reference */
#ifdef NOTDEF
typedef struct COMMAND_struct
{
  char name[MAXLEN];		/* Commands name */
  /*  char          paramiters[MAXLEN]; Commands paramiters part of name */
  struct COMMAND_struct *next;	/* ptr to next command */
}
COMMAND_list;

typedef struct ALIAS_struct
{
  char name[MAXLEN];		/* command's name */
  COMMAND_list *commands;	/* List of commands alias runs */
  struct ALIAS_struct *next;	/* ptr to next channel */
}
ALIAS_list;

#endif

int init_script()
{
  /* Allocate ram for the first entry */
  Script = (ALIAS_list *) malloc(sizeof(*Script));

  /* Set first entry as a test */
  strcpy(Script->name, "TEST");
  Script->commands = (COMMAND_list *) malloc(sizeof(*Script->commands));
  strcpy(Script->commands->name, "MSG Rubin Hi this is a TEST!");
  Script->commands->next = NULL;

  /* No more entrys */
  Script->next = NULL;
  Script_last = Script;
}

void add_alias(char *from, char *to, char *rest)
{
  ALIAS_list *new_alias;
  char buf[MAXLEN];
  int n = 0;
  char *tmpbuf;
  char *tmp1, *tmp1old;
  COMMAND_list *tmp_command;

  new_alias = (ALIAS_list *) malloc(sizeof(*new_alias));
  if (!rest)
  {
    send_to_user(from, "\x2Syntax: ALIAS <name> <command>[:<command>...]\x2");
    return;
  }
  strncpy(buf, rest, MAXLEN - 1);
  tmpbuf = strchr(buf, ' ');
  if (!tmpbuf)
  {
    send_to_user(from, "\x2Syntax: ALIAS <name> <command>[:<command>...]\x2");
    return;
  }
  tmpbuf[0] = '\0';
  strcpy(new_alias->name, buf);
  tmpbuf++;
  if ((tmp1 = strtok(tmpbuf, ":")) == NULL)
  {
    tmp1 = strtok(tmpbuf, "\0");
    if (tmp1)
      send_to_user(from, "\x2strtok returned %s\x2", tmp1);
  }
  if (tmp1)
  {
    tmp_command = (COMMAND_list *) malloc(sizeof(*tmp_command));
    tmp_command->next = NULL;
    strncpy(tmp_command->name, tmp1, MAXLEN - 1);
    new_alias->commands = tmp_command;
  }
  else
  {
    send_to_user(from, "\x2Sir, Spesify a command to run.\x2");
    return;
  }
  if ((tmp1 = strtok(NULL, ":")) == NULL)
    tmp1 = strtok(NULL, "\0");
  while (tmp1)
  {
    tmp_command->next = (COMMAND_list *) malloc(sizeof(*tmp_command->next));
    tmp_command = tmp_command->next;
    strncpy(tmp_command->name, tmp1, MAXLEN - 1);	/* put param into command list */
    tmp_command->next = NULL;
    if ((tmp1 = strtok(NULL, ":")) == NULL)
      tmp1 = strtok(NULL, "\0");
  }

  /* Add into the Script list */
  new_alias->next = NULL;
  Script_last->next = new_alias;	/* Add ours to the end */
  Script_last = new_alias;	/* Ours is now last */
  return;
}

void list_script(char *from)
{
  ALIAS_list *alias;
  COMMAND_list *command;

  alias = Script;
  send_to_user(from, "%sAlias list\x2", "\x2");
  send_to_user(from, "----------");
  while (alias != NULL)
  {
    send_to_user(from, "\x2%s\x2", alias);
    command = alias->commands;
    while (command != NULL)
    {
      send_to_user(from, "   %s", command->name);
      command = command->next;
    }
    send_to_user(from, "----------");
    alias = alias->next;

  }
}

void run_alias(char *from, char *to, char *rest)
{
  int n = 0;
  char *parm[20];
  char *aliasname, *paramiters;
  char buf[MAXLEN];
  ALIAS_list *alias;
  COMMAND_list *cmdptr;

  aliasname = rest;

  alias = find_alias(aliasname);
  if (alias == NULL)
  {
    send_to_user(from, "\x2Sorry Sir, Alias not found.\x2");
    return;
  }
  cmdptr = alias->commands;
  while (cmdptr != NULL)
  {
    sprintf(buf, "%c%s", cmdchar, cmdptr->name);
    on_msg(from, to, buf);
    cmdptr = cmdptr->next;
  }
}

ALIAS_list *find_alias(char *name)
{
  ALIAS_list *alias;

  alias = Script;
  while (alias != NULL)
  {
    /* send_to_user("rubin", "comparing %s and %s", alias->name, name); */
    if (strcasecmp(alias->name, name) == 0)
      return (alias);
    alias = alias->next;
  }
  return (NULL);

}

/* ------------ Python stuffz ------------- */
/* This function is called by the interpreter to get its own name */
char *
 getprogramname()
{
  return "RockBot";
}

/* A static module */

static PyObject *xyzzy_foo(self, args)
     PyObject *self;		/* Not used */
     PyObject *args;
{
  printf("I'm in xyzzy_foo\n");
  if (!PyArg_ParseTuple(args, ""))
    return NULL;
  return PyInt_FromLong(42L);
}

static PyObject *xyzzy_print(self, args)
     PyObject *self;
     PyObject *args;
{
  char *command;

  if (!PyArg_ParseTuple(args, "s", &command))
    return (NULL);
  send_to_user("rubin", "args = %s", command);
  return PyInt_FromLong(42L);
}

static PyMethodDef xyzzy_methods[] =
{
  {"foo", xyzzy_foo, 1},
  {"test1", xyzzy_print, 1},
  {NULL, NULL}			/* sentinel */
};

static PyMethodDef rock_methods[] =
{
  {"sendtouser", rock_sendtouser, 1},
  {"sendtoserver", rock_sendtoserver, 1},
  {"command", rock_command, 3},
  {NULL, NULL}			/* sentinel */
};

void initxyzzy()
{
  PyImport_AddModule("xyzzy");
  Py_InitModule("xyzzy", xyzzy_methods);
}

void initrock()
{
  PyImport_AddModule("rock");
  Py_InitModule("rock", rock_methods);
}

/* This is the part with all the actual functions python scripts can call... */

static PyObject *rock_sendtouser(PyObject * self, PyObject * args)
{
  char *command;
  char *nick, *rest;

  if (!PyArg_ParseTuple(args, "s", &command))
    return (NULL);
  nick = (char *) strtok(command, " ");
  rest = (char *) strtok(NULL, "\0");
  if (rest && nick)
  {
    send_to_user(nick, rest);
  }
  else
  {
    send_to_user(PYNICK, "Error: rock.sendtouser requires a destination and text to send.");
  }
  return PyInt_FromLong(42L);
}

static PyObject *rock_sendtoserver(PyObject * self, PyObject * args)
{
  char *command;

  if (!PyArg_ParseTuple(args, "s", &command))
    return (NULL);
  if (command && *command)
  {
    send_to_server(command);
  }
  else
  {
    send_to_user(PYNICK, "Error: rock.sendtoserver requires text to send.");
  }
  return PyInt_FromLong(42L);
}

static PyObject *rock_command(PyObject * self, PyObject * args)
{
  char *command;
  char *from, *to, *rest = NULL;

  if (!PyArg_ParseTuple(args, "sss", &from, &to, &rest))
    return (NULL);

  if (from && to && rest)
  {
    on_msg(from, to, rest);
  }
  else
  {
    send_to_user(PYNICK, "Error: rock.command() paramiters incorrect.");
  }
  return PyInt_FromLong(42L);
}


#endif  /* ifdef PYTHON */
