/* 
 *  parsetime.c - parse time for at(1)
 *  Copyright (C) 1993, 1994  Thomas Koenig
 *
 *  modifications for English-language times
 *  Copyright (C) 1993  David Parsons
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
 * IN NO EVENT SHALL THE AUTHOR(S) BE LIABLE FOR ANY DIRECT, INDIRECT,
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
 *     |MIDNIGHT                   | |[DAY OF WEEK]                       |
 *     \TEATIME                    / |NUMBER [SLASH NUMBER [SLASH NUMBER]]|
 *                                   \PLUS NUMBER MINUTES|HOURS|DAYS|WEEKS/
 */

#include <sys/cdefs.h>

/* System Headers */

#include <sys/types.h>
#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#ifndef __FreeBSD__
#include <getopt.h>
#endif

/* Local headers */

#include "parsetime.h"

/* Local headers */

/* #include "at.h" */

/* Structures and unions */

enum {	/* symbols */
    MIDNIGHT, NOON, TEATIME,
    PM, AM, TOMORROW, TODAY, NOW,
    MINUTES, HOURS, DAYS, WEEKS, MONTHS, YEARS,
    NUMBER, PLUS, DOT, SLASH, ID, JUNK,
    JAN, FEB, MAR, APR, MAY, JUN,
    JUL, AUG, SEP, OCT, NOV, DEC,
    SUN, MON, TUE, WED, THU, FRI, SAT
    };

/*
 * parse translation table - table driven parsers can be your FRIEND!
 */
struct {
    const char *name;	/* token name */
    int value;	/* token id */
    int plural;	/* is this plural? */
} Specials[] = {
    { "midnight", MIDNIGHT,0 },	/* 00:00:00 of today or tomorrow */
    { "minuit", MIDNIGHT,0 },	/* 00:00:00 of today or tomorrow */
    { "noon", NOON,0 },		/* 12:00:00 of today or tomorrow */
    { "midi", NOON, 0 },		/* 12:00:00 of today or tomorrow */
    { "bouffe", NOON, 0 },		/* 12:00:00 of today or tomorrow */
    { "teatime", TEATIME,0 },	/* 16:00:00 of today or tomorrow */
    { "cafe", TEATIME, 0 },	/* 16:00:00 of today or tomorrow */
    { "am", AM,0 },		/* morning times for 0-12 clock */
    { "pm", PM,0 },		/* evening times for 0-12 clock */
    { "tomorrow", TOMORROW,0 },	/* execute 24 hours from time */
    { "demain", TOMORROW,0 },	/* execute 24 hours from time */
    { "today", TODAY, 0 },	/* execute today - don't advance time */
    { "aujourd'hui", TODAY,0 },		/* execute today - don't advance time */
    { "now", NOW,0 },		/* opt prefix for PLUS */
    { "maintenant", NOW,0 },		/* opt prefix for PLUS */

    { "minute", MINUTES, 0 },	/* minutes multiplier */
    { "minutes", MINUTES, 1 },	/* (pluralized) */
    { "min", MINUTES, 0 },
    { "mins", MINUTES, 1 },
    { "m", MINUTES, 0 },
    { "hour", HOURS,0 },	/* hours ... */
    { "heure", HOURS, 0 },		/* hours ... */
    { "hours", HOURS, 1 },	/* (pluralized) */
    { "heures", HOURS, 1 },		/* (pluralized) */
    { "hr", HOURS, 0 },		/* abbreviated */
    { "h", HOURS, 0 },
    { "day", DAYS,0 },		/* days ... */
    { "jour", DAYS, 0 },		/* days ... */
    { "days", DAYS, 1 },		/* (pluralized) */
    { "jours", DAYS, 1 },		/* (pluralized) */
    { "j", DAYS, 0 },
    { "week", WEEKS,0 },	/* week ... */
    { "semaine", WEEKS,0 },		/* week ... */
    { "weeks", WEEKS,1 },	/* (pluralized) */
    { "semaines", WEEKS, 1 },		/* (pluralized) */
    { "year", YEARS,0 },	/* year ... */
    { "année", YEARS,0 },	/* year ... */
    { "years", YEARS,1 },	/* (pluralized) */
    { "années", YEARS,1 },	/* (pluralized) */
    { "jan", JAN, 0 },
    { "feb", FEB, 0 },
    { "mar", MAR, 0 },
    { "apr", APR, 0 },
    { "may", MAY, 0 },
    { "jun", JUN, 0 },
    { "jul", JUL, 0 },
    { "aug", AUG, 0 },
    { "sep", SEP, 0 },
    { "oct", OCT, 0 },
    { "nov", NOV, 0 },
    { "dec", DEC, 0 },
    { "janv", JAN, 0 },
    { "fev", FEB, 0 },
    { "mar", MAR, 0 },
    { "avr", APR, 0 },
    { "mai", MAY, 0 },
    { "jui", JUN, 0 },
    { "jul", JUL, 0 },
    { "auo", AUG, 0 },
    { "sep", SEP, 0 },
    { "oct", OCT, 0 },
    { "nov", NOV, 0 },
    { "dec", DEC, 0 },
    { "january", JAN,0 },
    { "february", FEB,0 },
    { "march", MAR,0 },
    { "april", APR,0 },
    { "may", MAY,0 },
    { "june", JUN,0 },
    { "july", JUL,0 },
    { "august", AUG,0 },
    { "september", SEP,0 },
    { "october", OCT,0 },
    { "november", NOV,0 },
    { "december", DEC,0 },
    { "january", JAN,0 },
    { "february", FEB,0 },
    { "march", MAR,0 },
    { "april", APR,0 },
    { "may", MAY,0 },
    { "june", JUN,0 },
    { "july", JUL,0 },
    { "august", AUG,0 },
    { "september", SEP,0 },
    { "october", OCT,0 },
    { "november", NOV,0 },
    { "december", DEC,0 },
    { "janvier", JAN,0 },
    { "février", FEB,0 },
    { "mars", MAR,0 },
    { "avril", APR,0 },
    { "mai", MAY,0 },
    { "juin", JUN,0 },
    { "juillet", JUL,0 },
    { "août", AUG,0 },
    { "septembre", SEP,0 },
    { "octobre", OCT,0 },
    { "novembre", NOV,0 },
    { "décembre", DEC,0 },
} ;

/* File scope variables */

static char **scp;	/* scanner - pointer at arglist */
static char scc;	/* scanner - count of remaining arguments */
static char *sct;	/* scanner - next char pointer in current argument */
static int need;	/* scanner - need to advance to next argument */

static char *sc_token;	/* scanner - token buffer */
static size_t sc_len;   /* scanner - length of token buffer */
static int sc_tokid;	/* scanner - token id */
static int sc_tokplur;	/* scanner - is token plural? */

/* Local functions */

/*
 * parse a token, checking if it's something special to us
 */
static int parse_token(char *arg)
{
    size_t i;

    for (i=0; i<(sizeof Specials/sizeof Specials[0]); i++)
	if (strcasecmp(Specials[i].name, arg) == 0) {
	    sc_tokplur = Specials[i].plural;
	    return sc_tokid = Specials[i].value;
	}

    /* not special - must be some random id */
    return ID;
} /* parse_token */


/*
 * init_scanner() sets up the scanner to eat arguments
 */
static int init_scanner(int argc, char **argv)
{
    scp = argv;
    scc = argc;
    need = 1;
    sc_len = 1;
    while (argc-- > 0)
	sc_len += strlen(*argv++);

    if ((sc_token = malloc(sc_len)) == NULL)
		/*errx(EXIT_FAILURE, "virtual memory exhausted");*/
		return(-1);

	return(0);
} /* init_scanner */

/*
 * token() fetches a token from the input stream
 */
static int token(void)
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
 * plonk() gives an appropriate error message if a token is incorrect
 */
static void plonk(int tok)
{
    /*
	  panic((tok == EOF) ? "incomplete time"
		       : "garbled time");
	*/
} /* plonk */

/* 
 * expect() gets a token and dies most horribly if it's not the token we want
 */
static int expect(int desired)
{
    if (token() != desired)
		return(-1);	/* and we die here... */
	return 0;
} /* expect */


/*
 * dateadd() adds a number of minutes to a date.  It is extraordinarily
 * stupid regarding day-of-month overflow, and will most likely not
 * work properly
 */
static void dateadd(minutes, tm)
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

static int plus(struct tm *tm)
{
    int delay;
    int expectplur;

    if (expect(NUMBER) == -1)
	return(-1);

    delay = atoi(sc_token);
    expectplur = (delay != 1) ? 1 : 0;

    switch (token()) {
    case YEARS:
	    tm->tm_year += delay;
	    break;
    case MONTHS:
	    tm->tm_mon += delay;
	    break;
    case WEEKS:
	    delay *= 7;
    case DAYS:
	    tm->tm_mday += delay;
	    break;
    case HOURS:
	    tm->tm_hour += delay;
	    break;
    case MINUTES:
	    tm->tm_min += delay;
	    break;
    default:
    	/*plonk(sc_tokid);*/
		return(-1);
	    break;
    }

    /*
	if (expectplur != sc_tokplur)
		warnx("pluralization is wrong");
	*/

    tm->tm_isdst = -1;
    if (mktime(tm) < 0)
		/*plonk(sc_tokid);*/
		return(-1);

	return(0);
} /* plus */


/*
 * tod() computes the time of day
 *     [NUMBER [DOT NUMBER] [AM|PM]]
 */
static int tod(struct tm *tm)
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

	if (sc_tokid == PM) {
	    if (hour != 12)	/* 12:xx PM is 12:xx, not 24:xx */
			hour += 12;
	} else {
	    if (hour == 12)	/* 12:xx AM is 00:xx, not 12:xx */
			hour = 0;
	}
	token();
    } else if (hour > 23)
		return(-1);

    /*
     * if we specify an absolute time, we don't want to bump the day even
     * if we've gone past that time - but if we're specifying a time plus
     * a relative offset, it's okay to bump things
     */
    if ((sc_tokid == EOF || sc_tokid == PLUS) && tm->tm_hour > hour) {
	tm->tm_mday++;
	tm->tm_wday++;
    }

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
static int assign_date(struct tm *tm, long mday, long mon, long year)
{

   /*
    * Convert year into tm_year format (year - 1900).
    * We may be given the year in 2 digit, 4 digit, or tm_year format.
    */
    if (year != -1) {
		if (year >= 1900)
			year -= 1900;   /* convert from 4 digit year */
		else if (year < 100) {
			/* convert from 2 digit year */
			struct tm *lt;
			time_t now;

			time(&now);
			lt = localtime(&now);

			/* Convert to tm_year assuming current century */
			year += (lt->tm_year / 100) * 100;
			
			if (year == lt->tm_year - 1) year++;
			else if (year < lt->tm_year)
				year += 100;    /* must be in next century */
		}
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
 *  |[DAY OF WEEK]                       |
 *  |NUMBER [SLASH NUMBER [SLASH NUMBER]]|
 *  \PLUS NUMBER MINUTES|HOURS|DAYS|WEEKS/
 */
static int month(struct tm *tm)
{
    long year= (-1);
    long mday = 0, wday, mon;
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

		case SUN: case MON: case TUE:
		case WED: case THU: case FRI:
		case SAT:
			/* do a particular day of the week
		     */
			wday = (sc_tokid-SUN);

			mday = tm->tm_mday;

			/* if this day is < today, then roll to next week
		     */
			if (wday < tm->tm_wday)
				mday += 7 - (tm->tm_wday - wday);
			else
				mday += (wday - tm->tm_wday);
			
			tm->tm_wday = wday;
			
			assign_date(tm, mday, tm->tm_mon, tm->tm_year);
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
				 * flip months and days for European timing
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

time_t parsetime(char *wanted)
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
    case NOW:	
	    if (scc < 1) {
			return nowtimer;
	    }
		/* now is optional prefix for PLUS tree */
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
	    if (runtime.tm_hour >= hr) {
		runtime.tm_mday++;
		runtime.tm_wday++;
	    }
	    runtime.tm_hour = hr;
	    runtime.tm_min = 0;
	    token();
	    /* FALLTHROUGH to month setting */
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
