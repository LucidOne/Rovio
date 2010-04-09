/*
 * wpa_supplicant/hostapd / Empty OS specific functions
 * Copyright (c) 2005-2006, Jouni Malinen <jkmaline@cc.hut.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of BSD
 * license.
 *
 * See README and COPYING for more details.
 *
 * This file can be used as a starting point when adding a new OS target. The
 * functions here do not really work as-is since they are just empty or only
 * return an error value. os_internal.c can be used as another starting point
 * or reference since it has example implementation of many of these functions.
 */

#include "includes.h"

#include "os.h"

void *os_malloc(size_t size)
{
	void *ret = malloc(size);
	if (ret == NULL)
	{
		diag_printf("malloc failed\n");
		cyg_interrupt_disable();
		while(1);
	}
	return ret;
}


char *os_strdup(const char *s)
{
	char *tmp;
	if (s == NULL)
		return NULL;

	tmp = os_malloc(strlen(s)+1);
	if (tmp == NULL)
		return NULL;

	strcpy(tmp, s);
	return tmp;
}

void os_sleep(os_time_t sec, os_time_t usec)
{
	cyg_thread_delay( sec * 100 + usec / 10000 );
}


int os_get_time(struct os_time *t)
{
	if (t != NULL)
	{
		cyg_tick_count_t ticks = cyg_current_time();
		t->sec = ticks / 100;
		t->usec = (ticks - t->sec * 100) * 1000;
		return 0;
	}
	return -1;
}


int os_mktime(int year, int month, int day, int hour, int min, int sec,
	      os_time_t *t)
{
	struct tm time;
	time.tm_sec	= sec;
	time.tm_min	= min;
	time.tm_hour	= hour;
	time.tm_mday	=day;
	time.tm_mon		= month;
	time.tm_year	= year;
	time.tm_wday	= 0;
	time.tm_yday	= 0;
	time.tm_isdst	= 0;
	return mktime( &time );
}


int os_daemonize(const char *pid_file)
{
	return -1;
}


void os_daemonize_terminate(const char *pid_file)
{
}


int os_get_random(unsigned char *buf, size_t len)
{
	size_t i;
	for (i = 0; i < len; ++i)
		buf[i] = (char)rand();
	return 0;
}


unsigned long os_random(void)
{
	return rand();
}


char * os_rel2abs_path(const char *rel_path)
{
	return os_strdup(rel_path);
	//return NULL; /* strdup(rel_path) can be used here */
}


int os_program_init(void)
{
	return 0;
}


void os_program_deinit(void)
{
}


int os_setenv(const char *name, const char *value, int overwrite)
{
	return -1;
}


int os_unsetenv(const char *name)
{
	return -1;
}


char * os_readfile(const char *name, size_t *len)
{
	return NULL;
}


void * os_zalloc(size_t size)
{
	char *buf = (char *)os_malloc(size);
	if (buf != NULL)
		memset(buf, 0, size);
	return buf;
}



#ifdef OS_NO_C_LIB_DEFINES
void * os_malloc(size_t size)
{
	return NULL;
}


void * os_realloc(void *ptr, size_t size)
{
	return NULL;
}


void os_free(void *ptr)
{
}


void * os_memcpy(void *dest, const void *src, size_t n)
{
	return dest;
}


void * os_memmove(void *dest, const void *src, size_t n)
{
	return dest;
}


void * os_memset(void *s, int c, size_t n)
{
	return s;
}


int os_memcmp(const void *s1, const void *s2, size_t n)
{
	return 0;
}


char * os_strdup(const char *s)
{
	return NULL;
}


size_t os_strlen(const char *s)
{
	return 0;
}


int os_strcasecmp(const char *s1, const char *s2)
{
	/*
	 * Ignoring case is not required for main functionality, so just use
	 * the case sensitive version of the function.
	 */
	return os_strcmp(s1, s2);
}


int os_strncasecmp(const char *s1, const char *s2, size_t n)
{
	/*
	 * Ignoring case is not required for main functionality, so just use
	 * the case sensitive version of the function.
	 */
	return os_strncmp(s1, s2, n);
}


char * os_strchr(const char *s, int c)
{
	return NULL;
}


char * os_strrchr(const char *s, int c)
{
	return NULL;
}


int os_strcmp(const char *s1, const char *s2)
{
	return 0;
}


int os_strncmp(const char *s1, const char *s2, size_t n)
{
	return 0;
}


char * os_strncpy(char *dest, const char *src, size_t n)
{
	return dest;
}


char * os_strstr(const char *haystack, const char *needle)
{
	return NULL;
}


int os_snprintf(char *str, size_t size, const char *format, ...)
{
	return 0;
}
#endif /* OS_NO_C_LIB_DEFINES */
