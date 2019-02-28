#ifndef __THREAD_H__
#define __THREAD_H__

#define LINUX 1
#define FREERTOS 2
#define OS_ENVIRONMRNT LINUX

#if (OS_ENVIRONMRNT == LINUX)
typedef unsigned long int thread_t;
#elif (OS_ENVIRONMRNT == FREERTOS)

#endif


int thread_create (thread_t * pthread, void * (* start_routine)(void *), void * arg) ;
int thread_join (thread_t pthread) ;
void thread_exit (void * retval) ;

#endif /*__THREAD_H__*/
