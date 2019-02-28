#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/queue.h>
#include <time.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <termios.h>

#include "thread.h"

#define TEST_MAIN 0

#if (OS_ENVIRONMRNT == LINUX)
#elif (OS_ENVIRONMRNT == FREERTOS)
#endif

#if (OS_ENVIRONMRNT == LINUX)

int thread_create (thread_t * pthread, void * (* start_routine)(void *), void * arg) {
    return pthread_create((pthread_t *)pthread, NULL, start_routine, arg);
}

int thread_join (thread_t pthread) {
    return pthread_join((pthread_t)pthread, NULL);
}

void thread_exit (void * retval) {
    pthread_exit(retval);
}

#elif (OS_ENVIRONMRNT == FREERTOS)

#endif

#if TEST_MAIN
void * test_start_routine (void * arg) {
    printf("hello thread\n");
    sleep(1);
    thread_exit(NULL);
    return NULL;
}

int main (int argc, char * argv[]) {
    thread_t t;
    thread_create(&t,test_start_routine,NULL);
    sleep(2);
    return 0;
}

#endif
