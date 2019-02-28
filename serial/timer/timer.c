#include <stdio.h>
#include <sys/select.h>
#include <unistd.h>

int timer_wait_ms (long int ms) {
    struct timeval delay;
    if (ms < 1000) {
        delay.tv_sec = 0;
        delay.tv_usec = ms*1000;
    } else {
        delay.tv_sec = ms/1000;
        delay.tv_usec = (ms%1000)*1000;
    }
    select(0,NULL,NULL,NULL,&delay);
    return 0;
}
