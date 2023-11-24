#ifndef CONFIG_H
#define CONFIG_H

/*
 * fRitE+ v4.3 -- config.h file
 * 
 * I have reorganized the config.h and added documentation.  Information
 * on all the defines can be found in docs/defines.doc
 *
 * 				Pyber (pyber@alias.undernet.org)
 */

#define IRCGECOS 	"I'm a ^BSucKeR^B for PunIshMenT"	/* Shown in /who */
#define SHITCOMMENT "It has been put forther that you are no longer welcome here..  So LEAVE!!"

/*
 * -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 *  Preproccesor Optimizations
 *
 * Please see the "docs/predfines.doc" file for more information on how to modify
 * this section.  The default is set to compile everything into the bot.
 *
 */
#define _POETRY_
#define _QUOTE_
#define _RULES_
#define _HELP_
#define _OPHELP_

/*
 * If you would like your bot to compile, don't change anything below here
 */
#define VERSION "4.2.5_BETA_"

#define MAXLEN		255
#define WAYTOBIG        1024
#define BIG_BUFFER	1024
#define MAXNICKLEN	10	
#define NICKLEN		9	

#define null(type) (type) 0L

#define FAIL	0
#define SUCCESS 1

#ifndef TRUE
#define TRUE     1
#define FALSE    0
#endif

#endif /* CONFIG_H */

