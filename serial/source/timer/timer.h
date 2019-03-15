#ifndef __TIMER_H__
#define __TIMER_H__

struct tm_utc{
  int tm_sec;			/* Seconds.	[0-60] (1 leap second) */
  int tm_min;			/* Minutes.	[0-59] */
  int tm_hour;			/* Hours.	[0-23] */
  int tm_mday;			/* Day.		[1-31] */
  int tm_mon;			/* Month.	[1-12] */
  int tm_year;			/* Year     [1900-]*/
};

void timer_wait_us (long us);

void timer_wait_ms (long ms);

int get_current_time (struct timeval * t);

long time_interval_us (struct timeval * t);

long time_interval_ms (struct timeval * t);

struct tm_utc * get_system_time (struct tm_utc * tm_out);

struct tm_utc * string_to_system_time(struct tm_utc * tm, char * str);

int system_time_to_string (char * str, struct tm_utc * tm);

#endif /*__TIMER_H__*/
