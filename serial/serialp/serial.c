#include "../serialp/serial.h"

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
	int ret = 0;
	ret = IniReadValue(CONFIG_SECTION_SERIAL,
					CONFIG_KEY_DEVICE, 
					handler->port, 
					SERIAL_CONFIG_PATH);
	handler->port[ret] = '\0';
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
//    log_info("init serial with \nport=%s, \nbaud=%d, \nparity=%c, \ndatabits=%d, \nstopbits=%d, \nreadListLen=%d, \nwriteListLen=%d",
//    		handler->port,
//    		handler->param.baud,
//			handler->param.parity,
//			handler->param.databits,
//			handler->param.stopbits,
//			handler->readListMaxLen,
//			handler->writeListMaxLen);
    log_info("init serial with %s, %d,%c,%d,%d, readListLen=%d, writeListLen=%d",
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
    log_info("write thread exec created.");
    tcflush(handler->fd, TCIOFLUSH);
    if( 0 != sem_post(&handler->writeDataSem)) {
    	log_error("write semaphore post error.");
    }
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
//            	int tmp;
            	write(handler->fd, packet->buf, packet->size);
//            	log_debug("send %d byte", packet->size);
//				for(tmp=0; tmp < packet->size; tmp++ ){
//            	 log_debug("cmd %02X", packet->buf[tmp]);
//				}
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
    log_info("read thread exec created.");
    clear_simple_packet_recv(pRecv);
    pthread_mutex_init(&pRecv->mutex, NULL);
    tcflush(handler->fd, TCIFLUSH);
    if( 0 != sem_post(&handler->readDataSem)) {
    	log_error("read semaphore post error.");
    }
    while (!handler->readThreadAttr.execStop) {
    	timer_wait_us(100);
    	pthread_mutex_lock(&pRecv->mutex);
    	pRecv->size = 0;
        pRecv->size = read(handler->fd, SIMPLE_PACKET_BUF_ADDR(pRecv), pRecv->sizeToRecv);
        pthread_mutex_unlock(&pRecv->mutex);
//        if ( pRecv->size > 0) {
//        	printf("%02X ", *(pRecv->buf+pRecv->sizeRecieved));
//        	int i;
//        	for (i=0; i<pRecv->sizeRecieved; i++) {
//        		printf("%02X ", *(pRecv->buf+i));
//        	}
//        	printf("\n");
//        	log_debug("recv %d byte.", pRecv->size );
//        }

        pthread_mutex_lock(&pRecv->mutex);
        if (parse_simple_packet(pRecv) == 0) {
        	handler->u->serial_recv(handler, SIMPLE_PACKET_PAYLOAD_ADDR(pRecv), SIMPLE_PACKET_PAYLOAD_SIZE(pRecv));
//        	log_debug("recv %d byte.", SIMPLE_PACKET_PAYLOAD_SIZE(pRecv));
            clear_simple_packet_recv(pRecv);
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
        log_error("open %s error.", handler->port);
        // close(handler->fd);
        return ERROR_OPENING;
    } else {
        struct termios newtio, oldtio;

        if ( tcgetattr( handler->fd, &oldtio) != 0)
        {
            printf("SetupSerial 1");
            goto ERROR_STARTING_quit;;
        }
        bzero( &newtio, sizeof( newtio ) );
        newtio.c_cflag |= CLOCAL | CREAD;
        newtio.c_cflag &= ~CSIZE;

        switch( handler->param.databits )
        {
        case 7:
            newtio.c_cflag |= CS7;
            break;
        case 8:
            newtio.c_cflag |= CS8;
            break;
        }

        switch( handler->param.parity)
        {
        case 'O':
            newtio.c_cflag |= PARENB;
            newtio.c_cflag |= PARODD;
            newtio.c_iflag |= INPCK;
            break;
        case 'E':
            newtio.c_iflag |= INPCK ;
            newtio.c_cflag |= PARENB;
            newtio.c_cflag &= ~PARODD;
            break;
        case 'N':
            newtio.c_cflag &= ~PARENB;
            break;
        case 'S':
    	newtio.c_cflag &= ~PARENB;
    	newtio.c_cflag &= ~CSTOPB;
    	break;
        }
        switch( handler->param.baud )
        {
        case 2400:
            cfsetispeed(&newtio, B2400);
            cfsetospeed(&newtio, B2400);
            break;
        case 4800:
            cfsetispeed(&newtio, B4800);
            cfsetospeed(&newtio, B4800);
            break;
        case 9600:
            cfsetispeed(&newtio, B9600);
            cfsetospeed(&newtio, B9600);
            break;
        case 115200:
            cfsetispeed(&newtio, B115200);
            cfsetospeed(&newtio, B115200);
            break;
        default:
            cfsetispeed(&newtio, B9600);
            cfsetospeed(&newtio, B9600);
            break;
        }
        if ( handler->param.stopbits == 1 )
        {
            newtio.c_cflag &= ~CSTOPB;
        }
        else if ( handler->param.stopbits == 2 )
        {
            newtio.c_cflag |= CSTOPB;
        }
        // remove ICANON standard model
        // remove ISIG   0x03  ^c (CTRL + C)
    	newtio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

    	// remove ICRNL IGNCR --> 0x0D ok
    	// remove IXON IXOFF --> 0x13 ok
    	newtio.c_iflag &= ~ (INLCR | ICRNL | IGNCR | IXON | IXOFF | IXANY);
    	//
    	newtio.c_oflag &= ~(OPOST | ONLCR | OCRNL);

    	newtio.c_cc[VTIME] = 10;			//20
    	newtio.c_cc[VMIN]  = 50;			//100

        tcflush(handler->fd, TCIFLUSH);
        if ((tcsetattr(handler->fd, TCSANOW, &newtio)) != 0)
        {
            log_error("Com set error");
            goto ERROR_STARTING_quit;;
        }
        // init semaphore
        sem_init(&handler->readDataSem, 0, 0);
        sem_init(&handler->writeDataSem, 0, 0);

        //_serial_packet_entryeate write thread
        if (pthread_create(&handler->writeThread, NULL, handler->writeExec, (void *)handler->writeThreadAttr.execArgs) != 0) {
            log_error("create write thread failed.");
            goto ERROR_STARTING_quit;
        } 
        // create read thread
        if (pthread_create(&handler->readThread, NULL, handler->readExec, (void *)handler->readThreadAttr.execArgs) != 0) {
            log_error("create read thread failed.");
            if (pthread_join(handler->writeThread, NULL) == 0) {
                log_info("successfully waited until serial write thread stopped execution.");
            }
            goto ERROR_STARTING_quit;
        } 
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
        log_info("successfully waited until serial write thread stopped execution.");
    }
    if (pthread_join(handler->readThread, NULL) == 0) {
        log_info("successfully waited until serial read thread stopped execution.");
    }
    serial_clear_list(&handler->readList, &handler->readDataMut, &handler->readListLen);
    serial_clear_list(&handler->writeList, &handler->writeDataMut, &handler->writeListLen);
    close(handler->fd);
}

