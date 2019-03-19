#include "../serial/serial.h"

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

#include "../config.h"
#include "../inifun/inirw.h"
#include "../packet/frame.h"
#include "../packet/packet.h"
#include "../timer/timer.h"

#define DEBUG_PRINTF 0
#if DEBUG_PRINTF
    #define PRINTF(...) printf(__VA_ARGS__)
#else
    #define PRINTF(...)
#endif

int baud_arr[] = { 
	B921600, B460800, B230400, B115200, B57600, B38400, B19200, 
	B9600, B4800, B2400, B1200, B300, 
};

int name_arr[] = {
	921600, 460800, 230400, 115200, 57600, 38400,  19200,  
	9600,  4800,  2400,  1200,  300,  
};


void serial_config (struct serial_handler * handler) {
    // initial
	int ret = 0;
	// char buf[64];
	ret = IniReadValue(CONFIG_SECTION_SERIAL,
					CONFIG_KEY_DEVICE, 
					handler->port, 
					SERIAL_CONFIG_PATH);
	handler->port[ret] = '\0';

	printf("ret = %d\n", ret);

	handler->param.baud = readIntValue(CONFIG_SECTION_SERIAL,
				 CONFIG_KEY_BAUDRATE,
				 SERIAL_CONFIG_PATH);
	IniReadValue(CONFIG_SECTION_SERIAL, 
					CONFIG_KEY_PARITY, 
					(char *)&handler->param.parity,
					SERIAL_CONFIG_PATH);
	handler->param.databits = readIntValue(CONFIG_SECTION_SERIAL,
				 CONFIG_KEY_DATABITS,
				 SERIAL_CONFIG_PATH);
	handler->param.stopbits = readIntValue(CONFIG_SECTION_SERIAL,
				 CONFIG_KEY_STOPBITS,
				 SERIAL_CONFIG_PATH);
	handler->readListMaxLen = readIntValue(CONFIG_SECTION_SERIAL,
				 CONFIG_KEY_READ_LIST_MAX_LEN,
				 SERIAL_CONFIG_PATH);
	handler->writeListMaxLen = readIntValue(CONFIG_SECTION_SERIAL,
				 CONFIG_KEY_WRITE_LIST_MAX_LEN,
				 SERIAL_CONFIG_PATH);

    // handler->param.baud = 115200;
    // handler->param.parity = 'N';
    // handler->param.databits = 8;
    // handler->param.stopbits = 1;

    // message list
    // handler->readListMaxLen = 30;
    // handler->writeListMaxLen = 30;
    PRINTF("init serial with port=%s, baud=%d, parity=%c, databits=%d, stopbits=%d, readListLen=%d, writeListLen=%d\n",
    		handler->port,
    		handler->param.baud,
			handler->param.parity,
			handler->param.databits,
			handler->param.stopbits,
			handler->readListMaxLen,
			handler->writeListMaxLen);
}

// serial write start routine
static void * write_thread_exec (void * arg) {
	struct serial_handler * handler = (struct serial_handler *)arg;
    struct timeval now;
    struct timespec waitMoment;
    simple_packet_t _packet;
    simple_packet_t * packet = &_packet;
    PRINTF("write thread exec created\n");
    tcflush(handler->fd, TCIOFLUSH);
    while (!handler->writeThreadAttr.execStop) {
        serial_packet_entry_t * newEntry = NULL;

        pthread_mutex_lock(&handler->writeDataMut);
        newEntry = SIMPLEQ_FIRST(&handler->writeList);

        if (newEntry == NULL) {
            gettimeofday(&now, NULL);

            // PRINTF("time sec %ld , usec %ld\n", now.tv_sec, now.tv_usec);
            waitMoment.tv_sec = now.tv_sec + 5;
            waitMoment.tv_nsec = now.tv_usec * 1000;
            pthread_cond_timedwait(&handler->writeDataCond, &handler->writeDataMut, &waitMoment);
            newEntry = SIMPLEQ_FIRST(&handler->writeList);
            // PRINTF("stop to wait write cond\n");
        }

        if (newEntry != NULL) {
            if (create_simple_packet(packet, newEntry->data, newEntry->length) > 0) {
            	write(handler->fd, packet->buf, packet->size);
#if 0
            	if (packet->size > 0) {
                	int i;
                	printf("send: ");
                	for (i=0; i<packet->size; i++) {
                		printf("%02X ", packet->buf[i]);
                	}
                	printf("\n");
                }
#endif
//            	packet->buf[packet->size-1] = '\0';

            	PRINTF("serial send a %d bytes packet: %s\n", packet->size, (packet->buf+4));
            }
            SIMPLEQ_REMOVE_HEAD(&handler->writeList, next);
            handler->writeListLen--;
            free(newEntry->data);
            free(newEntry);
        }
        pthread_mutex_unlock(&handler->writeDataMut);
    }

    pthread_exit(NULL);
    return NULL;
}

// serial read start routine
static void * read_thread_exec (void * arg) {
	struct serial_handler * handler = (struct serial_handler* )arg;
    simple_packet_recv_t packet;
    simple_packet_recv_t * pRecv = &packet;
    PRINTF("read thread exec created\n");
    clear_simple_packet_recv(pRecv);
    pthread_mutex_init(&pRecv->mutex, NULL);
    tcflush(handler->fd, TCIFLUSH);

    while (!handler->readThreadAttr.execStop) {
    	timer_wait_us(100);
    	pthread_mutex_lock(&pRecv->mutex);
        pRecv->size = read(handler->fd, SIMPLE_PACKET_BUF_ADDR(pRecv), pRecv->sizeToRecv);
#if 0
        if (pRecv->size > 0) {
        	int i;
        	PRINTF("to Receive %d, received %d, all recvd %d, recvd data ", pRecv->sizeToRecv, pRecv->size, pRecv->sizeRecieved);
        	for (i=0; i<(pRecv->sizeRecieved+pRecv->size+1); i++) {
        		PRINTF("%02X ", pRecv->buf[i]);
        	}
        	PRINTF("\n");
        }
#endif
        pthread_mutex_unlock(&pRecv->mutex);


        pthread_mutex_lock(&pRecv->mutex);
        if (parse_simple_packet(pRecv) == 0) {
        	handler->u->serial_recv(handler, SIMPLE_PACKET_PAYLOAD_ADDR(pRecv), SIMPLE_PACKET_PAYLOAD_SIZE(pRecv));
            clear_simple_packet_recv(pRecv);
#if 0
            serial_packet_entry_t * newEntry = NULL;
            newEntry = (serial_packet_entry_t *)malloc(sizeof(serial_packet_entry_t));

            newEntry->length = SIMPLE_PACKET_PAYLOAD_SIZE(pRecv);

            newEntry->data = (uint8_t *)malloc(newEntry->length);
            memcpy(newEntry->data, SIMPLE_PACKET_PAYLOAD_ADDR(pRecv), newEntry->length);
            if (handler->readListLen < handler->readListMaxLen) {
                SIMPLEQ_INSERT_TAIL(&handler->readList, newEntry, next);
                handler->readListLen++;
                pthread_cond_signal(&handler->readDataCond);
            }
#endif

        }
        pthread_mutex_unlock(&pRecv->mutex);

    }

    pthread_exit(NULL);
    return NULL;
}   

#if 1
int serial_receive_from_list (struct serial_handler * handler, uint8_t * data, int msec) {
    serial_packet_entry_t * newEntry = NULL;
    int size = 0;
    pthread_mutex_lock(&handler->readDataMut);
    newEntry = SIMPLEQ_FIRST(&handler->readList);

    if (newEntry == NULL) {
        struct timeval now;
        struct timespec waitMoment;
        gettimeofday(&now, NULL);
        // PRINTF("time sec %ld , usec %ld\n", now.tv_sec, now.tv_usec);
        now.tv_usec += (msec * 1000);
        if (now.tv_usec > 1000000) {
            waitMoment.tv_sec = now.tv_sec + now.tv_usec/1000000;
            waitMoment.tv_nsec = (now.tv_usec % 1000000) * 1000;
        } else {
            waitMoment.tv_sec = now.tv_sec;
            waitMoment.tv_nsec = now.tv_usec * 1000;
        }
        pthread_cond_timedwait(&handler->readDataCond, &handler->readDataMut, &waitMoment);
        newEntry = SIMPLEQ_FIRST(&handler->readList);
        // PRINTF("stop to wait write cond\n");
    }

    if (newEntry != NULL) {
        // PRINTF("write list send a packet\n");
        size = newEntry->length;
        memcpy(data, newEntry->data, newEntry->length);
        SIMPLEQ_REMOVE_HEAD(&handler->readList, next);
        handler->readListLen--;
        free(newEntry->data);
        free(newEntry);
        pthread_mutex_unlock(&handler->readDataMut);
        return size;
    }

    pthread_mutex_unlock(&handler->readDataMut);
    return size;
}
#endif

int serial_send_to_list (struct serial_handler * handler, uint8_t * data, int size) {
    serial_packet_entry_t * newEntry = NULL;
    newEntry = (serial_packet_entry_t *)malloc(sizeof(serial_packet_entry_t));
    newEntry->data = (uint8_t *)malloc(size);
    if (newEntry == NULL) {
        return -1;
    }
    newEntry->length = size;
    memcpy(newEntry->data, data, size);
    
    pthread_mutex_lock(&handler->writeDataMut);
    if (handler->writeListLen < handler->writeListMaxLen) {
        SIMPLEQ_INSERT_TAIL(&handler->writeList, newEntry, next);
        handler->writeListLen++;
        pthread_cond_signal(&handler->writeDataCond);
    }
    pthread_mutex_unlock(&handler->writeDataMut);

    return 0;
}

int serial_clear_list (serial_list_head_t * head, pthread_mutex_t * mutex, int * len) {
    serial_packet_entry_t * newEntry = NULL;
    pthread_mutex_lock(mutex);
    
    for ( ; newEntry != NULL; ) {
        newEntry = SIMPLEQ_FIRST(head);
        SIMPLEQ_REMOVE_HEAD(head, next);
        (*len)--;
        free(newEntry->data);
        free(newEntry);
    }
    (*len) = 0;
    pthread_mutex_unlock(mutex);
    return 0;
}

serial_state_t serial_start (struct serial_handler * handler, struct serial_callbacks * u) {
    serial_config(handler);
    handler->u = u;
    // open serial device
    // handler->fd = open(handler->port, O_RDWR | O_NONBLOCK | O_NOCTTY | O_NDELAY);
    handler->fd = open(handler->port, O_RDWR | O_NOCTTY);
    // initial 
    handler->threadStarted = 0;
    handler->readExec = read_thread_exec;
    handler->writeExec = write_thread_exec;
    handler->readThreadAttr.execArgs = (void *)handler;
    handler->writeThreadAttr.execArgs = (void *)handler;
    handler->readThreadAttr.execStop = 0;
    handler->writeThreadAttr.execStop = 0;
    pthread_mutex_init(&handler->readDataMut, NULL);
    pthread_mutex_init(&handler->writeDataMut, NULL);
    pthread_cond_init(&handler->readDataCond, NULL);
    pthread_cond_init(&handler->writeDataCond, NULL);

    // simple queue
    SIMPLEQ_INIT(&handler->readList);
    SIMPLEQ_INIT(&handler->writeList);
    handler->readListLen = 0;
    handler->writeListLen = 0;

    if (handler->fd < 0) {
        PRINTF("open %s error\n", handler->port);
        // close(handler->fd);
        return ERROR_OPENING;
    } else {
        int idx;
        int baudListLen;
        struct termios newtio;
        memset(&newtio, 0, sizeof(newtio));
        PRINTF("open %s successfully\n", handler->port);
        if (tcgetattr(handler->fd, &newtio) != 0) {
            PRINTF("could not get serial port setting\n");
        }
        //
        // cfmakeraw(&newtio);
        // set baud rate
        baudListLen = sizeof(name_arr);
        for (idx=0; idx<baudListLen; idx++) {
            if (name_arr[idx] == handler->param.baud)
                break;
        }
        if (idx >= baudListLen) {
            perror("baudrate");
            goto ERROR_STARTING_quit;
        } else {
            cfsetispeed(&newtio, baud_arr[idx]);
            cfsetospeed(&newtio, baud_arr[idx]);
        }
        // 
        newtio.c_cflag &= ~CSIZE ;
        // set data bits
        switch (handler->param.databits) { 
            case 7:
                newtio.c_cflag |= CS7;
                break;
            case 8:
                newtio.c_cflag |= CS8;
                break;
            default:
                // fprintf(stderr,"Unsupported data size\n");
                PRINTF("Unsupported data size\n");
                goto ERROR_STARTING_quit;
        }
        // set parity
        switch (handler->param.parity) {
            case 'n':
            case 'N':
                newtio.c_cflag &= ~PARENB;   /* Clear parity enable */
                newtio.c_iflag &= ~INPCK;     /* Enable parity checking */
            break;
            case 'o':
            case 'O':
                newtio.c_cflag |= (PARODD | PARENB);  /* 设置为奇效验*/
                newtio.c_iflag |= INPCK;             /* Disnable parity checking */
            break;
            case 'e':
            case 'E':
                newtio.c_cflag |= PARENB;     /* Enable parity */
                newtio.c_cflag &= ~PARODD;   /* 转换为偶效验*/ 
                newtio.c_iflag |= INPCK;       /* Disnable parity checking */
            break;
            case 'S':	
            case 's':  /*as no parity*/
                newtio.c_cflag &= ~PARENB;
                newtio.c_cflag &= ~CSTOPB;
            break;
            default:
                // fprintf(stderr,"Unsupported parity\n");
                PRINTF("Unsupported parity\n");
                goto ERROR_STARTING_quit;
        }
        // newtio.c_cflag &= ~CRTSCTS ;
        // set stop bit
        switch (handler->param.stopbits) {
            case 1:
                newtio.c_cflag &= ~CSTOPB;
            break;
            case 2:
                newtio.c_cflag |= CSTOPB;
            break;
            default:
                // fprintf(stderr,"Unsupported stop bits\n");
                PRINTF("Unsupported stop bits\n");
                goto ERROR_STARTING_quit;
 	    }
        newtio.c_cflag |= ( CLOCAL | CREAD ) ;

        // Set input parity option
        // if (handler->param.parity != 'n' || handler->param.parity != 'N')
        //     newtio.c_iflag |= INPCK;

        newtio.c_cc[VTIME] = 5; // 5 * 100-milliseconds
        newtio.c_cc[VMIN] = 0;
        newtio.c_lflag &= ~(ECHO | ICANON);

        // test flush input/output queue
        if (tcflush(handler->fd, TCIOFLUSH) == -1) {
            PRINTF("could not flush input/output queue for terminal\n");
            goto ERROR_STARTING_quit;
        }
        // set serial device attr
        if (tcsetattr(handler->fd, TCSANOW, &newtio) == -1) {
            PRINTF("could not set terminal attribution\n");
            goto ERROR_STARTING_quit;
        }

        //_serial_packet_entryeate write thread
        if (pthread_create(&handler->writeThread, NULL, handler->writeExec, (void *)handler->writeThreadAttr.execArgs) != 0) {
            PRINTF("create write thread failed\n");
            goto ERROR_STARTING_quit;
        } 
#if DEBUG_PRINTF
        else {
            PRINTF("create write thread successfully\n");
        }
#endif
        // create read thread
        if (pthread_create(&handler->readThread, NULL, handler->readExec, (void *)handler->readThreadAttr.execArgs) != 0) {
            PRINTF("create read thread failed\n");
            if (pthread_join(handler->writeThread, NULL) == 0) {
                PRINTF("successfully waited until serial write thread stopped execution\n");
            }
            goto ERROR_STARTING_quit;
        } 
#if DEBUG_PRINTF
        else {
      PRINTF("Create read thread successfully\n");
        }
      
#endif
        handler->threadStarted = 0;
    }
    if (0) {
        ERROR_STARTING_quit:
        close(handler->fd);
        return ERROR_STARTING;
    }
    return STARTED;
}
void serial_close (struct serial_handler * handler) {
	handler->readThreadAttr.execStop = 1;
	handler->writeThreadAttr.execStop = 1;
    if (pthread_join(handler->writeThread, NULL) == 0) {
        PRINTF("successfully waited until serial write thread stopped execution\n");
    }
    if (pthread_join(handler->readThread, NULL) == 0) {
        PRINTF("successfully waited until serial read thread stopped execution\n");
    }
    serial_clear_list(&handler->readList, &handler->readDataMut, &handler->readListLen);
    serial_clear_list(&handler->writeList, &handler->writeDataMut, &handler->writeListLen);
    close(handler->fd);
}

