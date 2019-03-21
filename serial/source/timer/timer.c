

#include <stdio.h>
#include <sys/select.h>
#include <unistd.h>

//#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
//#include <fcntl.h>
//#include <sys/queue.h>
#include <time.h>
#include <sys/time.h>
//#include <sys/stat.h>
#include <sys/types.h>
//#include <string.h>
//#include <termios.h>

#include "timer.h"

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#define USE_USLEEP 1 // with devation 17~26 us
#define USE_SELECT 2 // long time (50000 us) with a small devation as 17,
                     // short time (1 us) with a large deviation as 1000 us
#define USE_COND   3
#define USE_METHOD USE_USLEEP

void timer_wait_us (long us) {
#if (USE_METHOD == USE_USLEEP)
	usleep(us);
#elif (USE_METHOD == USE_SELECT)
    struct timeval delay;
    if (us < 1000000) {
        delay.tv_sec = 0;
        delay.tv_usec = us;
    } else {
        delay.tv_sec = us/1000000;
        delay.tv_usec = us % 1000000;
    }printf
    select(0,NULL,NULL,NULL,&delay);
#elif (USE_METHOD == USE_COND)
    struct timeval now;
    struct timespec waitMoment;
    pthread_cond_t emptyCond;
    pthread_mutex_t emptyMut;
    pthread_cond_init(&emptyCond, NULL);
    pthread_mutex_init(&emptyMut, NULL);
    pthread_mutex_lock(&emptyMut);
    gettimeofday(&now, NULL);
    if (us >= 1000000) {
    	waitMoment.tv_sec = now.tv_sec + us/1000000;
    	us = us%1000000;
    }
	now.tv_usec += us;
	if (now.tv_usec > 1000000) {
		waitMoment.tv_sec += 1;
		waitMoment.tv_nsec = (now.tv_usec - 1000000) * 1000;
	} else {
		waitMoment.tv_nsec = now.tv_usec * 1000;
	}
    pthread_cond_timedwait(&emptyCond, &emptyMut, &waitMoment);
    pthread_mutex_unlock(&emptyMut);
#endif
}

void timer_wait_ms (long ms) {
#if (USE_METHOD == USE_USLEEP)
	usleep(ms*1000);
#elif (USE_METHOD == USE_SELECT)
    struct timeval delay;
    if (ms < 1000) {
        delay.tv_sec = 0;
        delay.tv_usec = ms*1000;
    } else {
        delay.tv_sec = ms/1000;
        delay.tv_usec = (ms%1000)*1000;
    }
    select(0,NULL,NULL,NULL,&delay);
#elif (USE_METHOD == USE_COND)
    struct timeval now;
    struct timespec waitMoment;
    pthread_cond_t emptyCond;
    pthread_mutex_t emptyMut;
    pthread_cond_init(&emptyCond, NULL);
    pthread_mutex_init(&emptyMut, NULL);
    pthread_mutex_lock(&emptyMut);
    gettimeofday(&now, NULL);
    if (ms >= 1000) {
    	waitMoment.tv_sec = now.tv_sec + ms/1000;
    	ms = ms%1000;
    }
	now.tv_usec += (ms * 1000);
	if (now.tv_usec > 1000000) {
		waitMoment.tv_sec += 1;
		waitMoment.tv_nsec = (now.tv_usec - 1000000) * 1000;
	} else {
		waitMoment.tv_nsec = now.tv_usec * 1000;
	}

    pthread_cond_timedwait(&emptyCond, &emptyMut, &waitMoment);
    pthread_mutex_unlock(&emptyMut);
#endif
}



int get_current_time (struct timeval * t) {
	return gettimeofday(t, NULL);
}

long time_interval_us (struct timeval * t) {
	long val;
	struct timeval now;
	gettimeofday(&now, NULL);
	if (now.tv_sec == t->tv_sec) {
		val = now.tv_usec - t->tv_usec;
	} else {
		val = now.tv_sec - t->tv_sec;
		val *= 1000000;
		val += now.tv_usec;
		val -= t->tv_usec;
	}
	return val;
}

long time_interval_ms (struct timeval * t) {
	long val;
	struct timeval now;
	gettimeofday(&now, NULL);
	if (now.tv_sec == t->tv_sec) {
		val = now.tv_usec - t->tv_usec;
		val /= 1000;
	} else {
		val = now.tv_sec - t->tv_sec;
		val *= 1000;
		val += (now.tv_usec - t->tv_usec)/1000;
	}
	return val;
}

struct tm_utc * get_system_time (struct tm_utc * tm_out) {
	time_t timer;
	struct tm * t_tm;
	time(&timer);
	t_tm = localtime(&timer);
	tm_out->tm_year = t_tm->tm_year+1900;
	tm_out->tm_mon = t_tm->tm_mon+1;
	tm_out->tm_mday = t_tm->tm_mday;
	tm_out->tm_hour = t_tm->tm_hour;
	tm_out->tm_min = t_tm->tm_min;
	tm_out->tm_sec = t_tm->tm_sec;
	PRINTF("today is %4d%02d%02d%02d%02d%02d/n",
			t_tm.tm_year+1900,
			t_tm.tm_mon+1,
			t_tm.tm_mday,
			t_tm.tm_hour,
			t_tm.tm_min,
			t_tm.tm_sec);
	return tm_out;
}

struct tm_utc * string_to_system_time(struct tm_utc * tm, char * str) {
    sscanf(str, "%d-%d-%d %d:%d:%d", &tm->tm_year,
        &tm->tm_mon, &tm->tm_mday,&tm->tm_hour,
        &tm->tm_min, &tm->tm_sec);
	return tm;
}

time_t string_to_time(char * str) {
	struct tm ptm;
	sscanf(str, "%d-%d-%d %d:%d:%d", &ptm.tm_year,
        &ptm.tm_mon, &ptm.tm_mday, &ptm.tm_hour,
        &ptm.tm_min, &ptm.tm_sec);
	ptm.tm_year -= 1900;
	ptm.tm_mon -= 1;
	return mktime(&ptm);
}

char * time_to_string(char * str, time_t t) {
	struct tm *ptm;
	ptm = gmtime(&t);
	snprintf(str, 20, "%d-%d-%d %d:%d:%d", ptm->tm_year+1900, ptm->tm_mon+1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
	return str;
}

int system_time_to_string (char * str, struct tm_utc * tm) {
	snprintf(str, 20, "%d-%d-%d %d:%d:%d", tm->tm_year, tm->tm_mon, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
	return 0;
}



int set_system_time(char * date) {
    struct tm rtc_time;
    struct tm _tm;
    struct timeval tv;
    time_t timep;
    sscanf(date, "%d-%d-%d %d:%d:%d",
    		&rtc_time.tm_year,
			&rtc_time.tm_mon,
			&rtc_time.tm_mday,
			&rtc_time.tm_hour,
			&rtc_time.tm_min,
			&rtc_time.tm_sec);

    _tm.tm_sec = rtc_time.tm_sec;
    _tm.tm_min = rtc_time.tm_min;
    _tm.tm_hour = rtc_time.tm_hour;
    _tm.tm_mday = rtc_time.tm_mday;
    _tm.tm_mon = rtc_time.tm_mon - 1;
    _tm.tm_year = rtc_time.tm_year - 1900;

    timep = mktime(&_tm);
    tv.tv_sec = timep;
    tv.tv_usec = 0;
    if(settimeofday (&tv, (struct timezone *) 0) < 0)
    {
		printf("Set system datatime error!/n");
		return -1;
    }
    return 0;
}
