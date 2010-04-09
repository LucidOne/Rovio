#ifndef __MY_TIME_H__
#define __MY_TIME_H__

void tzset(void);
struct tm *localtime_r(register const time_t * timer,
					   register struct tm * result);
struct tm *gmtime_r(const time_t * timer,
					struct tm * result);
time_t mktime(struct tm *timeptr);

#endif
