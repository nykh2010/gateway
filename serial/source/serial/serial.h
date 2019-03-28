#ifndef __SIMPLE_SERIAL_H__
#define __SIMPLE_SERIAL_H__

#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <sys/queue.h>
#include <signal.h>
#include <stdint.h>


typedef struct _serial_param {
    int baud;
    uint8_t databits;
    uint8_t stopbits;
    uint8_t parity;
} serial_param_t;

typedef enum _serial_state {
    STARTED = 0,
    ERROR_STARTING,
    ERROR_OPENING,
} serial_state_t;

typedef struct _exec_attr {
    uint8_t execStop;
    void * execArgs;
} exec_attr_t;

typedef struct _serial_packet_entry {
    int length;
    uint8_t * data;
    SIMPLEQ_ENTRY(_serial_packet_entry) next;
} serial_packet_entry_t;

typedef SIMPLEQ_HEAD(_serial_list_head, _serial_packet_entry) serial_list_head_t;

struct serial_handler;

struct serial_callbacks {
	int (* serial_recv) (struct serial_handler *, uint8_t *, int );
};

struct serial_handler {
    // thread state
    uint8_t threadStarted;
    // serial name string
    char port[24];
    // opened file handler
    int fd;
    // serial parameter set
    serial_param_t param;

    // thread 
    sem_t readDataSem;
    sem_t writeDataSem;
    pthread_mutex_t readDataMut;
    pthread_mutex_t writeDataMut;
    pthread_cond_t readDataCond;
    pthread_cond_t writeDataCond;
    pthread_t readThread;
    pthread_t writeThread;
    // message list 
    int readListLen;
    int writeListLen;
    int readListMaxLen;
    int writeListMaxLen;
    serial_list_head_t readList;
    serial_list_head_t writeList;
    // thread exec
    exec_attr_t readThreadAttr;
    exec_attr_t writeThreadAttr;
    void * (* writeExec) (void *);
    void * (* readExec) (void *);
    struct serial_callbacks * u;
};

typedef struct serial_handler serial_handler_t;


void msleep (long msec);
int simpleq_length (serial_list_head_t * head);
void serial_config (struct serial_handler * handler);
void serial_process_init (struct serial_handler * handler);
int serial_receive_from_list (struct serial_handler * handler, uint8_t * data, int msec);
int serial_send_to_list (struct serial_handler * handler, uint8_t * data, int size);
int serial_send (struct serial_handler * handler, uint8_t * data, int size);
serial_state_t serial_start (struct serial_handler * handler, struct serial_callbacks * u);
void serial_close (struct serial_handler * handler);


#endif /*__SERIAL_H__*/
