#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#include <sys/queue.h>
#include <string.h>

#include "../source/config.h"
#include "../source/payload.h"

#include "../source/serial/serial.h"
#include "../source/packet/packet.h"
#include "../source/timer/timer.h"
#include "../source/rts/rts.h"
/*--------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------*/
#define DEBUG_PRINTF 1
#if DEBUG_PRINTF
    #define PRINTF(...) printf(__VA_ARGS__)
#else
    #define PRINTF(...)
#endif
/*--------------------------------------------------------------------------------*/
static void rts_recv_data (struct rts_conn *ptr) {
	serial_packet_entry_t * newEntry = NULL;
	struct serial_handler * handler = (struct serial_handler* )ptr;

    PRINTF("rts recv data over: %s\n", ptr->ots.pld.rbuf.buf);
#if 1
    pthread_mutex_lock(&handler->readDataMut);
	newEntry = (serial_packet_entry_t *)malloc(sizeof(serial_packet_entry_t));

	newEntry->length = ptr->ots.pld.rbuf.size;

	newEntry->data = (uint8_t *)malloc(newEntry->length);
	memcpy(newEntry->data, ptr->ots.pld.rbuf.buf, newEntry->length);
	if (handler->readListLen < handler->readListMaxLen) {
		SIMPLEQ_INSERT_TAIL(&handler->readList, newEntry, next);
		handler->readListLen++;
		pthread_cond_signal(&handler->readDataCond);
	}
    pthread_mutex_unlock(&handler->readDataMut);
#endif
}
static void rts_recv_ctrl (struct rts_conn *ptr) {
    PRINTF("rts recv ctrl over\n");
}
static void rts_sent (struct rts_conn *ptr, int status, int num_tx) {
	PRINTF("sent by rts: status=%d num_tx=%d\n", status, num_tx);
}

static struct rts_callbacks rts_callback_obj = {
	rts_recv_data,
	rts_recv_ctrl,
	rts_sent,
};

/*--------------------------------------------------------------------------------*/
int rts_test (int argc, char * argv[]) {
    // variables define
    char mode;
    long int count = 0;
    struct timeval now;
    struct simple_payload_buf payload;
    struct rts_conn rts_conn_obj;

    srand(49);

    if (argc > 0) {
        rts_open(&rts_conn_obj, &rts_callback_obj);

        if (argc > 1) {
            mode = *(argv[2]);
        } else {
            PRINTF("need select mode T/R\n");
            return -1;
        }
    } else {
        PRINTF("need device name\n");
        return -1;
    }
    printf("serial test\n");

#if 1
    for(;;) {
        if (mode == 'T') {
            count++;
            gettimeofday(&now, NULL);

            payload.size = snprintf((char *)(payload.buf+1), 256, "%ld:%ld", count, now.tv_sec);
            payload.buf[0] = 0x80;
            rts_send(&rts_conn_obj, &payload, 20);
            payload.buf[payload.size] = '\0';
            timer_wait_ms(1000 + rand()%50);
            // sleep(1);
            PRINTF("send:%s\n", (payload.buf+1));

//            timer_wait_ms(10);
            sleep(1);
//            PRINTF("send:%s\n", SIMPLE_PAYLOAD_SEND_DATA_ADDR(payload));
        } else if (mode == 'R') {

            // buffer.size = handler->recv(handler, buffer.buf, 100);
        	payload.size = serial_receive_from_list (&rts_conn_obj.ots.pld.serial, payload.buf, 20);

             if (payload.size) {
            	 payload.buf[payload.size] = '\0';
                 PRINTF("recv: %s\n", payload.buf+1);
             }

        }
    }
#endif
    sleep(1);
    return 0;
}
