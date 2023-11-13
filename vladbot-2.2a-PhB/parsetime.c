/* 
 * parsetime.c - parse time for at(1)
 * Copyright (C) 1993  Thomas Koenig
 *
 * modifications for english-language times
 * Copyright (C) 1993  David Parsons
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. The name of the author(s) may not be used to endorse or promote
 *    products derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR(S) ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  at [NOW] PLUS NUMBER MINUTES|HOURS|DAYS|WEEKS
 *     /NUMBER [DOT NUMBER] [AM|PM]\ /[MONTH NUMBER [NUMBER]]             \
 *     |NOON                       | |[TOMORROW]                          |
 *     |MIDNIGHT                   | |NUMBER [SLASH NUMBER [SLASH NUMBER]]|
 *     \TEATIME                    / \PLUS NUMBER MINUTES|HOURS|DAYS|WEEKS/
 */

/* System Headers */

#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>

/* Local headers */

/* #include "at.h" */

/* Structures and unions */

enum {	/* symbols */
    MIDNIGHT, NOON, TEATIME,
    PM, AM, TOMORROW, TODAY, NOW,
    MINUTES, HOURS, DAYS, WEEKS,
    NUMBER, PLUS, DOT, SLASH, ID, JUNK,
    JAN, FEB, MAR, APR, MAY, JUN,
    JUL, AUG, SEP, OCT, NOV, DEC
};

/*
 * parse translation table - table driven parsers can be your FRIEND!
 */
struct {
    char *name;	/* token name */
    int value;	/* token id */
} Specials[] = {
    { "minuit", MIDNIGHT },	/* 00:00:00 of today or tomorrow */
    { "midi", NOON },		/* 12:00:00 of today or tomorrow */
    { "bouffe", NOON },		/* 12:00:00 of today or tomorrow */
    { "cafe", TEATIME },	/* 16:00:00 of today or tomorrow */
    { "am", AM },		/* morning times for 0-12 clock */
    { "pm", PM },		/* evening times for 0-12 clock */
    { "demain", TOMORROW },	/* execute 24 hours from time */
    { "aujourd'hui", TODAY },		/* execute today - don't advance time */
    { "maintenant", NOW },		/* opt prefix for PLUS */

    { "minute", MINUTES },	/* minutes multiplier */
    { "min", MINUTES },
    { "mins", MINUTES },
    { "m", MINUTES },
    { "minutes", MINUTES },	/* (pluralized) */
    { "heure", HOURS },		/* hours ... */
    { "hr", HOURS },		/* abbreviated */
    { "h", HOURS },
    { "heures", HOURS },		/* (pluralized) */
    { "jour", DAYS },		/* days ... */
    { "j", DAYS },
    { "jours", DAYS },		/* (pluralized) */
    { "semaine", WEEKS },		/* week ... */
    { "s", WEEKS },
    { "semaines", WEEKS },		/* (pluralized) */
    { "janv", JAN },
    { "fev", FEB },
    { "mar", MAR },
    { "avr", APR },
    { "mai", MAY },
    { "jui", JUN },
    { "jul", JUL },
    { "auo", AUG },
    { "sep", SEP },
    { "oct", OCT },
    { "nov", NOV },
    { "dec", DEC }
} ;

/* File scope variables */

static char **scp;	/* scanner - pointer at arglist */
static char scc;	/* scanner - count of remaining arguments */
static char *sct;	/* scanner - next char pointer in current argument */
static int need;	/* scanner - need to advance to next argument */

static char *sc_token;	/* scanner - token buffer */
static size_t sc_len;   /* scanner - lenght of token buffer */
static int sc_tokid;	/* scanner - token id */

static char rcsid[] = "$Id: parsetime.c,v 1.2 1994/03/08 23:49:19 cgd Exp $";

/* Local functions */

/*
 * parse a token, checking if it's something special to us
 */
static int
parse_token(arg)
	char *arg;
{
    int i;

    for (i=0; i<(sizeof Specials/sizeof Specials[0]); i++)
	if (strcasecmp(Specials[i].name, arg) == 0) {
	    return sc_tokid = Specials[i].value;
	}

    /* not special - must be some random id */
    return ID;
} /* parse_token */


/*
 * init_scanner() sets up the scanner to eat arguments
 */
static int
init_scanner(argc, argv)
	int argc;
	char **argv;
{
    scp = argv;
    scc = argc;
    need = 1;
    sc_len = 1;
    while (argc-- > 0)
	sc_len += strlen(*++argv);

    sc_token = (char *) malloc(sc_len);
    if (sc_token == NULL)
	return(-1);
} /* init_scanner */

/*
 * token() fetches a token from the input stream
 */
static int
token()
{
    int idx;

    while (1) {
	memset(sc_token, 0, sc_len);
	sc_tokid = EOF;
	idx = 0;

	/*
	 * if we need to read another argument, walk along the argument list;
	 * when we fall off the arglist, we'll just return EOF forever
	 */
	if (need) {
	    if (scc < 1)
		return sc_tokid;
	    sct = *scp;
	    scp++;
	    scc--;
	    need = 0;
	}
	/*
	 * eat whitespace now - if we walk off the end of the argument,
	 * we'll continue, which puts us up at the top of the while loop
	 * to fetch the next argument in
	 */
	while (isspace(*sct))
	    ++sct;
	if (!*sct) {
	    need = 1;
	    continue;
	}

	/*
	 * preserve the first character of the new token
	 */
	sc_token[0] = *sct++;

	/*
	 * then see what it is
	 */
	if (isdigit(sc_token[0])) {
	    while (isdigit(*sct))
		sc_token[++idx] = *sct++;
	    sc_token[++idx] = 0;
	    return sc_tokid = NUMBER;
	} else if (isalpha(sc_token[0])) {
	    while (isalpha(*sct))
		sc_token[++idx] = *sct++;
	    sc_token[++idx] = 0;
	    return parse_token(sc_token);
	}
	else if (sc_token[0] == ':' || sc_token[0] == '.')
	    return sc_tokid = DOT;
	else if (sc_token[0] == '+')
	    return sc_tokid = PLUS;
	else if (sc_token[0] == '/')
	    return sc_tokid = SLASH;
	else
	    return sc_tokid = JUNK;
    } /* while (1) */
} /* token */


/* 
 * expect() gets a token and dies most horribly if it's not the token we want
 */
static int
expect(desired)
	int desired;
{
    if (token() != desired)
	return(-1);	/* and we die here... */
} /* expect */


/*
 * dateadd() adds a number of minutes to a date.  It is extraordinarily
 * stupid regarding day-of-month overflow, and will most likely not
 * work properly
 */
static void
dateadd(minutes, tm)
	int minutes;
	struct tm *tm;
{
    /* increment days */

    while (minutes > 24*60) {
	minutes -= 24*60;
	tm->tm_mday++;
    }

    /* increment hours */
    while (minutes > 60) {
	minutes -= 60;
	tm->tm_hour++;
	if (tm->tm_hour > 23) {
	    tm->tm_mday++;
	    tm->tm_hour = 0;
	}
    }

    /* increment minutes */
    tm->tm_min += minutes;

    if (tm->tm_min > 59) {
	tm->tm_hour++;
	tm->tm_min -= 60;

	if (tm->tm_hour > 23) {
	    tm->tm_mday++;
	    tm->tm_hour = 0;
	}
    }
} /* dateadd */


/*
 * plus() parses a now + time
 *
 *  at [NOW] PLUS NUMBER [MINUTES|HOURS|DAYS|WEEKS]
 *
 */

static int
plus(tm)
	struct tm *tm;
{
    int delay;

    if (expect(NUMBER) == -1)
	return(-1);

    delay = atoi(sc_token);

    switch (token()) {
    case WEEKS:
	    delay *= 7;
    case DAYS:
	    delay *= 24;
    case HOURS:
	    delay *= 60;
    case MINUTES:
	    dateadd(delay, tm);
	    return(0);
    }
    return(-1);
} /* plus */


/*
 * tod() computes the time of day
 *     [NUMBER [DOT NUMBER] [AM|PM]]
 */
static int
tod(tm)
	struct tm *tm;
{
    int hour, minute = 0;
    int tlen;

    hour = atoi(sc_token);
    tlen = strlen(sc_token);

    /*
     * first pick out the time of day - if it's 4 digits, we assume
     * a HHMM time, otherwise it's HH DOT MM time
     */
    if (token() == DOT) {
	if (expect(NUMBER) == -1)
	    return(-1);
	minute = atoi(sc_token);
	if (minute > 59)
	    return(-1);
	token();
    } else if (tlen == 4) {
	minute = hour%100;
	if (minute > 59)
	    return(-1);
	hour = hour/100;
    }

    /*
     * check if an AM or PM specifier was given
     */
    if (sc_tokid == AM || sc_tokid == PM) {
	if (hour > 12)
	    return(-1);

	if (sc_tokid == PM)
	    hour += 12;
	token();
    } else if (hour > 23)
	return(-1);

    /*
     * if we specify an absolute time, we don't want to bump the day even
     * if we've gone past that time - but if we're specifying a time plus
     * a relative offset, it's okay to bump things
     */
    if ((sc_tokid == EOF || sc_tokid == PLUS) && tm->tm_hour > hour)
	tm->tm_mday++;

    tm->tm_hour = hour;
    tm->tm_min = minute;
    if (tm->tm_hour == 24) {
	tm->tm_hour = 0;
	tm->tm_mday++;
    }
    return(0);
} /* tod */


/*
 * assign_date() assigns a date, wrapping to next year if needed
 */
static int
assign_date(tm, mday, mon, year)
	struct tm *tm;
	long mday, mon, year;
{
    if (year > 99) {
	if (year > 1899)
	    year -= 1900;
	else
	    return(-1);
    }

    if (year < 0 &&
	(tm->tm_mon > mon ||(tm->tm_mon == mon && tm->tm_mday > mday)))
	year = tm->tm_year + 1;

    tm->tm_mday = mday;
    tm->tm_mon = mon;

    if (year >= 0)
	tm->tm_year = year;
    return(0);
} /* assign_date */


/* 
 * month() picks apart a month specification
 *
 *  /[<month> NUMBER [NUMBER]]           \
 *  |[TOMORROW]                          |
 *  |NUMBER [SLASH NUMBER [SLASH NUMBER]]|
 *  \PLUS NUMBER MINUTES|HOURS|DAYS|WEEKS/
 */
static int
month(tm)
	struct tm *tm;
{
    long year= (-1);
    long mday, mon;
    int tlen;

    switch (sc_tokid) {
    case PLUS:
	    if (plus(tm) == -1)
		return(-1);
	    break;

    case TOMORROW:
	    /* do something tomorrow */
	    tm->tm_mday ++;
    case TODAY:	/* force ourselves to stay in today - no further processing */
	    token();
	    break;

    case JAN: case FEB: case MAR: case APR: case MAY: case JUN:
    case JUL: case AUG: case SEP: case OCT: case NOV: case DEC:
	    /*
	     * do month mday [year]
	     */
	    mon = (sc_tokid-JAN);
	    if (expect(NUMBER) == -1)
		return(-1);
	    mday = atol(sc_token);
	    if (token() == NUMBER) {
		year = atol(sc_token);
		token();
	    }
	    if (assign_date(tm, mday, mon, year) == -1)
		return(-1);
	    break;

    case NUMBER:
	    /*
	     * get numeric MMDDYY, mm/dd/yy, or dd.mm.yy
	     */
	    tlen = strlen(sc_token);
	    mon = atol(sc_token);
	    token();

	    if (sc_tokid == SLASH || sc_tokid == DOT) {
		int sep;

		sep = sc_tokid;
		if (expect(NUMBER) == -1)
		    return(-1);
		mday = atol(sc_token);
		if (token() == sep) {
		    if (expect(NUMBER) == -1)
		        return(-1);
		    year = atol(sc_token);
		    token();
		}

		/*
		 * flip months and days for european timing
		 */
		if (sep == DOT) {
		    int x = mday;
		    mday = mon;
		    mon = x;
		}
	    } else if (tlen == 6 || tlen == 8) {
		if (tlen == 8) {
		    year = (mon % 10000) - 1900;
		    mon /= 10000;
		} else {
		    year = mon % 100;
		    mon /= 100;
		}
		mday = mon % 100;
		mon /= 100;
	    } else
		return(-1);

	    mon--;
	    if (mon < 0 || mon > 11 || mday < 1 || mday > 31)
		return(-1);

	    if (assign_date(tm, mday, mon, year) == -1)
		return(-1);
	    break;
    } /* case */
    return(0);
} /* month */


/* Global functions */

time_t
parsetime(wanted)
	char *wanted;
{
/*
 * Do the argument parsing, die if necessary, and return the time the job
 * should be run.
 */
    int argc=2;
    char *argv[20];
    time_t nowtimer, runtimer;
    struct tm nowtime, runtime;
    int i, j=strlen(wanted), hr = 0;
    /* this MUST be initialized to zero for midnight/noon/teatime */

    if (!wanted[0])
	return(-1);

    for (argv[1] = wanted, i=0; i<j; i++)
	if (wanted[i] == ' ') {
	   wanted[i] = '\0';
	   argv[argc++] =  &wanted[i+1];
	}

    nowtimer = time(NULL);
    nowtime = *localtime(&nowtimer);

    runtime = nowtime;
    runtime.tm_sec = 0;
    runtime.tm_isdst = 0;

    if (init_scanner(argc-1, argv+1) == -1)
	return(-1);

    switch (token()) {
    case NOW:	/* now is optional prefix for PLUS tree */
	    if (expect(PLUS) == -1)
		return(-1);
    case PLUS:
	    if (plus(&runtime) == -1)
		return(-1);
	    break;

    case NUMBER:
	    if (tod(&runtime) == -1)
		return(-1);
	    if (month(&runtime) == -1)
		return(-1);
	    break;

	    /*
	     * evil coding for TEATIME|NOON|MIDNIGHT - we've initialised
	     * hr to zero up above, then fall into this case in such a
	     * way so we add +12 +4 hours to it for teatime, +12 hours
	     * to it for noon, and nothing at all for midnight, then
	     * set our runtime to that hour before leaping into the
	     * month scanner
	     */
    case TEATIME:
	    hr += 4;
    case NOON:
	    hr += 12;
    case MIDNIGHT:
	    if (runtime.tm_hour >= hr)
		runtime.tm_mday++;
	    runtime.tm_hour = hr;
	    runtime.tm_min = 0;
	    token();
	    /* fall through to month setting */
    default:
	    if (month(&runtime) == -1)
		return(-1);
	    break;
    } /* ugly case statement */
/*    if (expect(EOF) == -1)
	return(-1);
*/

    /*
     * adjust for daylight savings time
     */
    runtime.tm_isdst = -1;
    runtimer = mktime(&runtime);
    if (runtime.tm_isdst > 0) {
	runtimer -= 3600;
	runtimer = mktime(&runtime);
    }

    if (runtimer < 0)
	return(-1);

    if (nowtimer > runtimer)
	return(-2);

    return runtimer;
} /* parsetime */
