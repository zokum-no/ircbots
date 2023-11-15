#include "config.h"  
#include "commands.h"

typedef struct  COMMAND_struct
{
  char		name[MAXLEN];       /* Commands name */
  struct COMMAND_struct *next;      /* ptr to next command */
} COMMAND_list;

typedef struct  ALIAS_struct
{
  char			name[MAXLEN];	/* command's name */
  COMMAND_list 		*commands;	/* List of commands alias runs */
  struct ALIAS_struct	*next;		/* ptr to next channel */
} ALIAS_list;

void list_script(char *from);
int init_script();
void add_alias(char *from, char *to, char *rest);
void run_alias(char *from, char *to, char *rest);
ALIAS_list *find_alias(char *name);

static PyObject *rock_sendtouser(PyObject *self, PyObject *args);
static PyObject *rock_sendtoserver(PyObject *self, PyObject *args);
static PyObject *rock_command(PyObject *self, PyObject *args);
