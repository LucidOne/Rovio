/*  Copyright (C) 2002     Manuel Novoa III
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*  ATTENTION!   ATTENTION!   ATTENTION!   ATTENTION!   ATTENTION!
 *
 *  Besides uClibc, I'm using this code in my libc for elks, which is
 *  a 16-bit environment with a fairly limited compiler.  It would make
 *  things much easier for me if this file isn't modified unnecessarily.
 *  In particular, please put any new or replacement functions somewhere
 *  else, and modify the makefile to use your version instead.
 *  Thanks.  Manuel
 *
 *  ATTENTION!   ATTENTION!   ATTENTION!   ATTENTION!   ATTENTION! */

/* June 15, 2002     Initial Notes:
 *
 * Note: It is assumed throught that time_t is either long or unsigned long.
 *       Similarly, clock_t is assumed to be long int.
 *
 * Warning: Assumptions are made about the layout of struct tm!  It is
 *    assumed that the initial fields of struct tm are (in order):
 *    tm_sec, tm_min, tm_hour, tm_mday, tm_mon, tm_year, tm_wday, tm_yday
 *
 * Reached the inital goal of supporting the ANSI/ISO C99 time functions
 * as well as SUSv3's strptime.  All timezone info is obtained from the
 * TZ env variable.
 *
 * Differences from glibc worth noting:
 *
 * Leap seconds are not considered here.
 *
 * glibc stores additional timezone info the struct tm, whereas we don't.
 *
 * Alternate digits and era handling are not currently implemented.
 * The modifiers are accepted, and tested for validity with the following
 * specifier, but are ignored otherwise.
 *
 * strftime does not implement glibc extension modifiers or widths for
 *     conversion specifiers.  However it does implement the glibc
 *     extension specifiers %l, %k, and %s.  It also recognizes %P, but
 *     treats it as a synonym for %p; i.e. doesn't convert to lower case.
 *
 * strptime implements the glibc extension specifiers.  However, it follows
 *     SUSv3 in requiring at least one non-alphanumeric char between
 *     conversion specifiers.  Also, strptime only sets struct tm fields
 *     for which format specifiers appear and does not try to infer other
 *     fields (such as wday) as glibc's version does.
 *
 * TODO - Since glibc's %l and %k can space-pad their output in strftime,
 *     it might be reasonable to eat whitespace first for those specifiers.
 *     This could be done by pushing " %I" and " %H" respectively so that
 *     leading whitespace is consumed.  This is really only an issue if %l
 *     or %k occurs at the start of the format string.
 *
 * TODO - Implement getdate? tzfile? struct tm extensions?
 *
 * TODO - Rework _time_mktime to remove the dependency on long long.
 */

/* Oct 28, 2002
 *
 * Fixed allowed char check for std and dst TZ fields.
 *
 * Added several options concerned with timezone support.  The names will
 * probably change once Erik gets the new config system in place.
 *
 * Defining __TIME_TZ_FILE causes tzset() to attempt to read the TZ value
 * from the file /etc/TZ if the TZ env variable isn't set.  The file contents
 * must be the intended value of TZ, followed by a newline.  No other chars,
 * spacing, etc is allowed.  As an example, an easy way for me to init
 * /etc/TZ appropriately would be:    echo CST6CDT > /etc/TZ
 *
 * Defining __TIME_TZ_FILE_ONCE will cause all further accesses of /etc/TZ
 * to be skipped once a legal value has been read.
 *
 * Defining __TIME_TZ_OPT_SPEED will cause a tzset() to keep a copy of the
 * last TZ setting string and do a "fast out" if the current string is the
 * same.
 *
 * Nov 21, 2002   Fix an error return case in _time_mktime.
 *
 * Nov 26, 2002   Fix bug in setting daylight and timezone when no (valid) TZ.
 *   Bug reported by Arne Bernin <arne@alamut.de> in regards to freeswan.
 *
 * July 27, 2003  Adjust the struct tm extension field support.
 *   Change __tm_zone back to a ptr and add the __tm_tzname[] buffer for
 *   __tm_zone to point to.  This gets around complaints from g++.
 *  Who knows... it might even fix the PPC timezone init problem.
 *
 * July 29, 2003  Fix a bug in mktime behavior when tm_isdst was -1.
 *   Bug reported by "Sid Wade" <sid@vivato.net> in regards to busybox.
 *
 *   NOTE: uClibc mktime behavior is different than glibc's when
 *   the struct tm has tm_isdst == -1 and also had fields outside of
 *   the normal ranges.
 * 
 *   Apparently, glibc examines (at least) tm_sec and guesses the app's
 *   intention of assuming increasing or decreasing time when entering an
 *   ambiguous time period at the dst<->st boundaries.
 *
 *   The uClibc behavior is to always normalize the struct tm and then
 *   try to determing the dst setting.
 *
 *   As long as tm_isdst != -1 or the time specifiec by struct tm is
 *   unambiguous (not falling in the dst<->st transition region) both
 *   uClibc and glibc should produce the same result for mktime.
 *
 * Oct 31, 2003 Kill the seperate __tm_zone and __tm_tzname[] and which
 *   doesn't work if you want the memcpy the struct.  Sigh... I didn't
 *   think about that.  So now, when the extensions are enabled, we
 *   malloc space when necessary and keep the timezone names in a linked
 *   list.
 *
 *   Fix a dst-related bug which resulted in use of uninitialized data.
 *
 * Nov 15, 2003 I forgot to update the thread locking in the last dst fix.
 *
 * Dec 14, 2003 Fix some dst issues in _time_mktime().
 *   Normalize the tm_isdst value to -1, 0, or 1.
 *   If no dst for this timezone, then reset tm_isdst to 0.
 */

#define _GNU_SOURCE
#define _STDIO_UTILITY
#ifndef ECOS
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <limits.h>
#include <assert.h>
#include <errno.h>
#include <ctype.h>
#include <locale.h>
#else
#include "stdio.h"
#include "stdlib.h"
#include "stddef.h"
#include "string.h"
#include "time.h"
#include "ctype.h"
#include "limits.h"
#include "assert.h"
#include "errno.h"
#include "ctype.h"
#include "locale.h"
#endif

#ifndef ECOS

#define     isascii(c)    (((c) & ~0x7f) == 0)    /* If C is a 7 bit value.  */


void tzset(void);
struct tm *localtime_r(register const time_t * timer,
					   register struct tm * result);
struct tm *gmtime_r(const time_t * timer,
					struct tm * result);
time_t mktime(struct tm *timeptr);



#ifndef __isleap
#define __isleap(y) ( !((y) % 4) && ( ((y) % 100) || !((y) % 400) ) )
#endif

#define TZNAME_MAX	6
#ifndef TZNAME_MAX
#define TZNAME_MAX _POSIX_TZNAME_MAX
#endif

/**********************************************************************/
/* The era code is currently unfinished. */
/*  #define ENABLE_ERA_CODE */

#define TZ_BUFLEN		(2*TZNAME_MAX + 56)


/**********************************************************************/

extern struct tm __time_tm;

typedef struct {
	long gmt_offset;
	long dst_offset;
	short day;					/* for J or normal */
	short week;
	short month;
	short rule_type;			/* J, M, \0 */
	char tzname[TZNAME_MAX+1];
} rule_struct;


#define TZLOCK		((void) 0)
#define TZUNLOCK	((void) 0)


extern rule_struct _time_tzinfo[2];

extern struct tm *_time_t2tm(const time_t * timer,
							 int offset, struct tm * result);

extern time_t _time_mktime(struct tm *timeptr, int store_on_success);

/**********************************************************************/

/**********************************************************************/
/* Strictly speaking, this implementation isn't correct.  ANSI/ISO specifies
 * that the implementation of asctime() be equivalent to
 *
 *   char *asctime(const struct tm *timeptr)
 *   {
 *       static char wday_name[7][3] = {
 *           "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
 *       };
 *       static char mon_name[12][3] = {
 *           "Jan", "Feb", "Mar", "Apr", "May", "Jun",
 *           "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" 
 *       };
 *       static char result[26];
 *   
 *       sprintf(result, "%.3s %.3s%3d %.2d:%.2d:%.2d %d\n",
 *           wday_name[timeptr->tm_wday],                   
 *           mon_name[timeptr->tm_mon],
 *           timeptr->tm_mday, timeptr->tm_hour,
 *           timeptr->tm_min, timeptr->tm_sec,  
 *           1900 + timeptr->tm_year);        
 *       return result;
 *   }
 *
 * but the above is either inherently unsafe, or carries with it the implicit
 * assumption that all fields of timeptr fall within their usual ranges, and
 * that the tm_year value falls in the range [-2899,8099] to avoid overflowing
 * the static buffer.
 *
 * If we take the implicit assumption as given, then the implementation below
 * is still incorrect for tm_year values < -900, as there will be either
 * 0-padding and/or a missing negative sign for the year conversion .  But given
 * the ususal use of asctime(), I think it isn't unreasonable to restrict correct
 * operation to the domain of years between 1000 and 9999.
 */

/* This is generally a good thing, but if you're _sure_ any data passed will be
 * in range, you can #undef this. */



/**********************************************************************/
#include <float.h>

#if FLT_RADIX != 2
#error difftime implementation assumptions violated for you arch!
#endif

double difftime(time_t time1, time_t time0)
{
#if (LONG_MAX >> DBL_MANT_DIG) == 0

	/* time_t fits in the mantissa of a double. */
	return ((double) time1) - time0;

#elif ((LONG_MAX >> DBL_MANT_DIG) >> DBL_MANT_DIG) == 0

	/* time_t can overflow the mantissa of a double. */
	time_t t1, t0, d;

	d = ((time_t) 1) << DBL_MANT_DIG;
	t1 = time1 / d;
	time1 -= (t1 * d);
	t0 = time0 / d;
	time0 -= (t0*d);

	/* Since FLT_RADIX==2 and d is a power of 2, the only possible
	 * rounding error in the expression below would occur from the
	 * addition. */
	return (((double) t1) - t0) * d + (((double) time1) - time0);

#else
#error difftime needs special implementation on your arch.
#endif
}


/**********************************************************************/
struct tm *gmtime_r(const time_t * timer,
					struct tm * result)
{
	return _time_t2tm(timer, 0, result);
}



static const unsigned char day_cor[] = { /* non-leap */
	31, 31, 34, 34, 35, 35, 36, 36, 36, 37, 37, 38, 38
/* 	 0,  0,  3,  3,  4,  4,  5,  5,  5,  6,  6,  7,  7 */
/*	    31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 */
};

/* Note: timezone locking is done by localtime_r. */

static int tm_isdst(register const struct tm * ptm)
{
	register rule_struct *r = _time_tzinfo;
	long sec;
	int i, isdst, isleap, day, day0, monlen, mday;
	int oday=0;				/* Note: oday can be uninitialized. */

	isdst = 0;
	if (r[1].tzname[0] != 0) {
		/* First, get the current seconds offset from the start of the year.
		 * Fields of ptm are assumed to be in their normal ranges. */
		sec = ptm->tm_sec
			+ 60 * (ptm->tm_min
					+ 60 * (long)(ptm->tm_hour
								  + 24 * ptm->tm_yday));
		/* Do some prep work. */
		i = (ptm->tm_year % 400) + 1900; /* Make sure we don't overflow. */
		isleap = __isleap(i);
		--i;
		day0 = (1
				+ i				/* Normal years increment 1 wday. */
				+ (i/4)
				- (i/100)
				+ (i/400) ) % 7;
		i = 0;
		do {
			day = r->day;		/* Common for 'J' and # case. */
			if (r->rule_type == 'J') {
				if (!isleap || (day < (31+29))) {
					--day;
				}
			} else if (r->rule_type == 'M') {
				/* Find 0-based day number for 1st of the month. */
				day = 31*r->month - day_cor[r->month -1];
				if (isleap && (day >= 59)) {
					++day;
				}
				monlen = 31 + day_cor[r->month -1] - day_cor[r->month];
				if (isleap && (r->month > 1)) {
					++monlen;
				}
				/* Wweekday (0 is Sunday) of 1st of the month
				 * is (day0 + day) % 7. */
				if ((mday = r->day - ((day0 + day) % 7)) >= 0) {
					mday -= 7;	/* Back up into prev month since r->week>0. */
				}
				if ((mday += 7 * r->week) >= monlen) {
					mday -= 7;
				}
				/* So, 0-based day number is... */
				day += mday;
			}

			if (i != 0) {
				/* Adjust sec since dst->std change time is in dst. */
				sec += (r[-1].gmt_offset - r->gmt_offset);
				if (oday > day) {
					++isdst;	/* Year starts in dst. */
				}
			}
			oday = day;

			/* Now convert day to seconds and add offset and compare. */
			if (sec >= (day * 86400L) + r->dst_offset) {
				++isdst;
			}
			++r;
		} while (++i < 2);
	}

	return (isdst & 1);
}

struct tm *localtime_r(register const time_t * timer,
					   register struct tm * result)
{
	time_t x[1];
	long offset;
	int days, dst;

	TZLOCK;

	tzset();

	dst = 0;
	do {
		days = -7;
		offset = 604800L - _time_tzinfo[dst].gmt_offset;
		if (*timer > (LONG_MAX - 604800L)) {
			days = -days;
			offset = -offset;
		}
		*x = *timer + offset;

		_time_t2tm(x, days, result);
		result->tm_isdst = dst;
	} while ((++dst < 2) && ((result->tm_isdst = tm_isdst(result)) != 0));

	TZUNLOCK;

	return result;
}

/**********************************************************************/
/* Another name for `mktime'.  */
/* time_t timelocal(struct tm *tp) */

time_t mktime(struct tm *timeptr)
{
	return  _time_mktime(timeptr, 1);
}




#define MAX_PUSH 4

#ifdef __UCLIBC_MJN3_ONLY__
#warning TODO: Check multibyte format string validity.
#endif







static const char tzset_vals[] = {
	'T', 'Z', 0,				/* 3 */
	'U', 'T', 'C', 0,			/* 4 */
	25, 60, 60, 1,				/* 4 */
	'.', 1,						/* M */
	5, '.', 1,
	6,  0,  0,					/* Note: overloaded for non-M non-J case... */
	0, 1, 0,					/* J */
	',', 'M',      '4', '.', '1', '.', '0',
	',', 'M', '1', '0', '.', '5', '.', '0', 0
};

#define TZ    tzset_vals
#define UTC   (tzset_vals + 3)
#define RANGE (tzset_vals + 7)
#define RULE  (tzset_vals + 11 - 1)
#define DEFAULT_RULES (tzset_vals + 22)

/* Initialize to UTC. */
int daylight = 0;
long timezone = 0;
char *tzname[2] = { (char *) UTC, (char *) (UTC-1) };


rule_struct _time_tzinfo[2];

static const char *getoffset(register const char *e, long *pn)
{
	register const char *s = RANGE-1;
	long n;
	int f;

	n = 0;
	f = -1;
	do {
		++s;
		if (isdigit(*e)) {
			f = *e++ - '0';
		}
		if (isdigit(*e)) {
			f = 10 * f + (*e++ - '0');
		}
		if (((unsigned int)f) >= *s) {
			return NULL;
		}
		n = (*s) * n + f;
		f = 0;
		if (*e == ':') {
			++e;
			--f;
		}
	} while (*s > 1);

	*pn = n;
	return e;
}

static const char *getnumber(register const char *e, int *pn)
{
	int n, f;

	n = 3;
	f = 0;
	while (n && isdigit(*e)) {
		f = 10 * f + (*e++ - '0');
		--n;
	}

	*pn = f;
	return (n == 3) ? NULL : e;
}


void tzset(void)
{
	register const char *e;
	register char *s;
	long off;
	short *p;
	rule_struct new_rules[2];
	int n, count, f;
	char c;

	TZLOCK;

	e = getenv(TZ);				/* TZ env var always takes precedence. */

	/* Warning!!!  Since uClibc doesn't do lib locking, the following is
	 * potentially unsafe in a multi-threaded program since it is remotely
	 * possible that another thread could call setenv() for TZ and overwrite
	 * the string being parsed.  So, don't do that... */

	if ((!e						/* TZ env var not set... */
		 ) || !*e) {			/* or set to empty string. */
	ILLEGAL:					/* TODO: Clean up the following... */
		memset(_time_tzinfo, 0, 2*sizeof(rule_struct));
		strcpy(_time_tzinfo[0].tzname, UTC);
		goto DONE;
	}

	if (*e == ':') {			/* Ignore leading ':'. */
		++e;
	}

	
	count = 0;
	new_rules[1].tzname[0] = 0;
 LOOP:
	/* Get std or dst name. */
	c = 0;
	if (*e == '<') {
		++e;
		c = '>';
	}

	s = new_rules[count].tzname;
	n = 0;
	while (*e
		   && isascii(*e)		/* SUSv3 requires char in portable char set. */
		   && (isalpha(*e)
			   || (c && (isalnum(*e) || (*e == '+') || (*e == '-'))))
		   ) {
		*s++ = *e++;
		if (++n > TZNAME_MAX) {
			goto ILLEGAL;
		}
	}
	*s = 0;

	if ((n < 3)					/* Check for minimum length. */
		|| (c && (*e++ != c))	/* Match any quoting '<'. */
		) {
		goto ILLEGAL;
	}

	/* Get offset */
	s = (char *) e;
	if ((*e != '-') && (*e != '+')) {
		if (count && !isdigit(*e)) {
			off -= 3600;		/* Default to 1 hour ahead of std. */
			goto SKIP_OFFSET;
		}
		--e;
	}

	++e;
	if ((e = getoffset(e, &off)) == 0) {
		goto ILLEGAL;
	}

	if (*s == '-') {
		off = -off;				/* Save off in case needed for dst default. */
	}
 SKIP_OFFSET:
	new_rules[count].gmt_offset = off;

	if (!count) {
		new_rules[1].gmt_offset = off; /* Shouldn't be needed... */
		if (*e) {
			++count;
			goto LOOP;
		}
	} else {					/* OK, we have dst, so get some rules. */
		count = 0;
		if (!*e) {				/* No rules so default to US rules. */
			e = DEFAULT_RULES;
		}

		do {
			if (*e++ != ',') {
				goto ILLEGAL;
			}

			n = 365;
			s = (char *) RULE;
			if ((c = *e++) == 'M') {
				n = 12;
			} else if (c == 'J') {
				s += 8;
			} else {
				--e;
				c = 0;
				s += 6;
			}

			*(p = &new_rules[count].rule_type) = c;
			if (c != 'M') {
				p -= 2;
			}

			do {
				++s;
				if ((e = getnumber(e, &f)) == 0
					|| (((unsigned int)(f - s[1])) > n)
					|| (*s && (*e++ != *s))
					) {
					goto ILLEGAL;
				}
				*--p = f;
			} while ((n = *(s += 2)) > 0);

			off = 2 * 60 * 60;	/* Default to 2:00:00 */
			if (*e == '/') {
				++e;
				if ((e = getoffset(e, &off)) == 0) {
					goto ILLEGAL;
				}
			}
			new_rules[count].dst_offset = off;
		} while (++count < 2);

		if (*e) {
			goto ILLEGAL;
		}
	}

	memcpy(_time_tzinfo, new_rules, sizeof(new_rules));
 DONE:
	tzname[0] = _time_tzinfo[0].tzname;
	tzname[1] = _time_tzinfo[1].tzname;
	daylight = !!_time_tzinfo[1].tzname[0];
	timezone = _time_tzinfo[0].gmt_offset;

	TZUNLOCK;
}


static const unsigned short _time_t2tm_vals[] = {
	60, 60, 24, 7 /* special */, 36524, 1461, 365, 0
};

static const unsigned char days[] = {
	31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31, /* non-leap */
	    29,
};


/* Notes:
 * If time_t is 32 bits, then no overflow is possible.
 * It time_t is > 32 bits, this needs to be adjusted to deal with overflow.
 */

/* Note: offset is the correction in _days_ to *timer! */

struct tm *_time_t2tm(const time_t * timer,
					  int offset, struct tm * result)
{
	register int *p;
	time_t t1, t, v;
	int wday=0;					/* Note: wday can be uninitialized. */

	{
		register const unsigned short *vp;
		t = *timer;
		p = (int *) result;
		p[7] = 0;
		vp = _time_t2tm_vals;
		do {
			if ((v = *vp) == 7) {
				/* Overflow checking, assuming time_t is long int... */
#if (LONG_MAX > INT_MAX) && (LONG_MAX > 2147483647L)
#if (INT_MAX == 2147483647L) && (LONG_MAX == 9223372036854775807L)
				/* Valid range for t is [-784223472856L, 784223421720L].
				 * Outside of this range, the tm_year field will overflow. */
				if (((unsigned long)(t + offset- -784223472856L))
					> (784223421720L - -784223472856L)
					) {
					return NULL;
				}
#else
#error overflow conditions unknown
#endif
#endif

				/* We have days since the epoch, so caluclate the weekday. */
				wday = ((int)((t % (*vp)) + 11)) % ((int)(*vp)); /* help bcc */
				/* Set divisor to days in 400 years.  Be kind to bcc... */
				v = ((time_t)(vp[1])) << 2;
				++v;
				/* Change to days since 1/1/1601 so that for 32 bit time_t
				 * values, we'll have t >= 0.  This should be changed for
				 * archs with larger time_t types. 
				 * Also, correct for offset since a multiple of 7. */

				/* TODO: Does this still work on archs with time_t > 32 bits? */
				t += (135140L - 366) + offset; /* 146097 - (365*30 + 7) -366 */
			}
			if ((t -= ((t1 = t / v) * v)) < 0) {
				t += v;
				--t1;
			}

			if ((*vp == 7) && (t == v-1)) {
				--t;			/* Correct for 400th year leap case */
				++p[4];			/* Stash the extra day... */
			}

			if (v <= 60) {
				*p++ = t;
				t = t1;
			} else {
				*p++ = t1;
			}
		} while (*++vp);
	}

	if (p[-1] == 4) {
		--p[-1];
		t = 365;
	}


	*p += ((int) t);			/* result[7] .. tm_yday */

	p -= 2;						/* at result[5] */

#if (LONG_MAX > INT_MAX) && (LONG_MAX > 2147483647L)
	/* Protect against overflow.  TODO: Unecessary if int arith wraps? */
	*p = ((((p[-2]<<2) + p[-1])*25 + p[0])<< 2) + (p[1] - 299); /* tm_year */
#else
	*p = ((((p[-2]<<2) + p[-1])*25 + p[0])<< 2) + p[1] - 299; /* tm_year */
#endif

	p[1] = wday;				/* result[6] .. tm_wday */

	{
		register const unsigned char *d = days;

		wday = 1900 + *p;
		if (__isleap(wday)) {
			d += 11;
		}

		wday = p[2] + 1;		/* result[7] .. tm_yday */
		*--p = 0;				/* at result[4] .. tm_mon */
		while (wday > *d) {
			wday -= *d;
			if (*d == 29) {
				d -= 11;		/* Backup to non-leap Feb. */
			}
			++d;
			++*p;				/* Increment tm_mon. */
		}
		p[-1] = wday;			/* result[3] .. tm_mday */
	}
	/* TODO -- should this be 0? */
	p[4] = 0;					/* result[8] .. tm_isdst */

	return result;
}

/**********************************************************************/
static const unsigned char _time_mktime_vals[] = {
	31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31, /* non-leap */
	    29,
};

time_t _time_mktime(struct tm *timeptr, int store_on_success)
{
	long long secs;
	time_t t;
	struct tm x;
	/* 0:sec  1:min  2:hour  3:mday  4:mon  5:year  6:wday  7:yday  8:isdst */
	register int *p = (int *) &x;
	register const unsigned char *s;
	int d, default_dst;

	TZLOCK;

	tzset();

	memcpy(p, timeptr, sizeof(struct tm));

	if (!_time_tzinfo[1].tzname[0]) { /* No dst in this timezone, */
		p[8] = 0;				/* so set tm_isdst to 0. */
	}

	default_dst = 0;
	if (p[8]) {					/* Either dst or unknown? */
		default_dst = 1;		/* Assume advancing (even if unknown). */
		p[8] = ((p[8] > 0) ? 1 : -1); /* Normalize so abs() <= 1. */
	}

	d = 400;
	p[5] = (p[5] - ((p[6] = p[5]/d) * d)) + (p[7] = p[4]/12);
	if ((p[4] -= 12 * p[7]) < 0) {
		p[4] += 12;
		--p[5];
	}

	s = _time_mktime_vals;
	d = (p[5] += 1900);			/* Correct year.  Now between 1900 and 2300. */
	if (__isleap(d)) {
		s += 11;
	}
	
	p[7] = 0;
	d = p[4];
	while (d) {
		p[7] += *s;
		if (*s == 29) {
			s -= 11;			/* Backup to non-leap Feb. */
		}
		++s;
		--d;
	}

	d = p[5] - 1;
	d = -719163L + d*365 + (d/4) - (d/100) + (d/400);
	secs = p[0]
		+ _time_tzinfo[default_dst].gmt_offset
		+ 60*( p[1]
			   + 60*(p[2]
					 + 24*(((146073L * ((long long)(p[6])) + d)
							+ p[3]) + p[7])));

 DST_CORRECT:
	if (((unsigned long long)(secs - LONG_MIN))
		> (((unsigned long long)LONG_MAX) - LONG_MIN)
		) {
		t = ((time_t)(-1));
		goto DONE;
	}

	d = ((struct tm *)p)->tm_isdst;
	t = secs;

	localtime_r(&t, (struct tm *)p);

	if (t == ((time_t)(-1))) {	/* Remember, time_t can be unsigned. */
	    goto DONE;
	}

	if ((d < 0) && (((struct tm *)p)->tm_isdst != default_dst)) {
		secs += (_time_tzinfo[1-default_dst].gmt_offset
				 - _time_tzinfo[default_dst].gmt_offset);
		goto DST_CORRECT;
	}


	if (store_on_success) {
		memcpy(timeptr, p, sizeof(struct tm));
	}


 DONE:
	TZUNLOCK;

	return t;
}


/* Return the number of days in YEAR.  */
int dysize(int year)
{
	return __isleap(year) ? 366 : 365;
}

#endif /* ECOS */
