/*
 * main.c
 *
 *  Created on: Feb 18, 2019
 *      Author: user
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <sys/queue.h>

#include "./payload.h"
#include "./serial/serial.h"
#include "./packet/packet.h"

#include "./timer/timer.h"


/*--------------------------------------------------------------------------------*/
#define DEBUG_PRINTF 1
#if DEBUG_PRINTF
    #define PRINTF(...) printf(__VA_ARGS__)
#else
    #define PRINTF(...)
#endif
/*--------------------------------------------------------------------------------*/
#define BUS_DRIVER_EXAMPLE 1 // deleted
#define OTS_EXAMPLE        2 // ok
#define RTS_EXAMPLE        3
#define PTS_EXAMPLE        4
#define EXAMPLE RTS_EXAMPLE
/*--------------------------------------------------------------------------------*/

#if (EXAMPLE == RTS_EXAMPLE)
/*--------------------------------------------------------------------------------*/
#include "./rts/rts.h"

/*--------------------------------------------------------------------------------*/
static void rts_recv_data (struct rts_conn *ptr) {
    PRINTF("rts recv data over: %s\n", ptr->ots.pld.rbuf.buf);
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
int main (int argc, char * argv[]) {
    // variables define
    char mode;
    long int count = 0;
    struct timeval now;
    struct simple_payload_buf payload;
    struct rts_conn rts_conn_obj;

    srand(49);

    if (argc > 0) {
        rts_open(argv[1], &rts_conn_obj, &rts_callback_obj);

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
            timer_wait_ms(100 + rand()%50);
            // sleep(1);
            PRINTF("send:%s\n", (payload.buf+1));

//            timer_wait_ms(10);
            sleep(1);
//            PRINTF("send:%s\n", SIMPLE_PAYLOAD_SEND_DATA_ADDR(payload));
        } else if (mode == 'R') {

            // buffer.size = handler->recv(handler, buffer.buf, 100);

            sleep(1);
            // if (buffer.size) {
            //     buffer.buf[buffer.size] = '\0';
            //     PRINTF("recv: %s\n", buffer.buf);
            // }

        }
    }
#endif
    sleep(1);
    return 0;
}


#elif (EXAMPLE == OTS_EXAMPLE)
#include "./ots/ots.h"
//
/*--------------------------------------------------------------------------------*/
static void ots_recv (struct ots_conn *ptr) {
	struct simple_payload_buf * p = &ptr->pld.rbuf;
	p->buf[p->size] = '\0';
    PRINTF("ots recv %d bytes: %s\n", p->size, (p->buf + 1));
}

static void ots_sent (struct ots_conn *ptr, int status, int num_tx) {

}

/*--------------------------------------------------------------------------------*/
int main (int argc, char * argv[]) {
    // variables define
#if 1
    char mode;
    long int count = 0;
    struct timeval now;
    struct simple_payload_buf payload;
    struct ots_conn ots_conn_obj;
    struct ots_callbacks ots_callback_obj;
    ots_callback_obj.recv = ots_recv;
	ots_callback_obj.sent = ots_sent;


    if (argc > 0) {
        ots_open(argv[1], &ots_conn_obj, &ots_callback_obj);

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
#endif
    printf("serial test\n");
#if 1
    for(;;) {
        if (mode == 'T') {
            count++;
            gettimeofday(&now, NULL);
            payload.size = snprintf((char *)(payload.buf+1), 256, "%ld:%ld", count, now.tv_sec);
            payload.buf[0] = 0x80;
            ots_send(&ots_conn_obj, &payload);
            payload.buf[payload.size] = '\0';
            timer_wait_ms(10);
            sleep(1);
            PRINTF("send:%s\n", (payload.buf+1));
        } else if (mode == 'R') {

            // buffer.size = handler->recv(handler, buffer.buf, 100);

            sleep(1);
            // if (buffer.size) {
            //     buffer.buf[buffer.size] = '\0';
            //     PRINTF("recv: %s\n", buffer.buf);
            // }

        }
    }
#endif
    sleep(1);
    return 0;
}

#elif (EXAMPLE == BUS_DRIVER_EXAMPLE)
#include "./bus/bus.h"
/*--------------------------------------------------------------------------------*/
int main (int argc, char * argv[]) {
    // variables define
    char mode;
    long int count = 0;
    struct timeval now;
    struct bus_driver_t * handler = NULL;
    simple_packet_t buffer;
    char str[256];
    int len;

    if (argc > 0) {
    	handler = (bus_driver_t *)bus_driver_create((void *)handler, argv[1]);

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
    handler->open(handler);
    for(;;) {
        if (mode == 'T') {
            count++;
            gettimeofday(&now, NULL);
            len = snprintf(str, 256, "%ld:%ld", count, now.tv_sec);
            handler->send(handler, (uint8_t *)str, len);
            timer_wait_ms(10);
            sleep(1);
            PRINTF("send:%s\n", str);
        } else if (mode == 'R') {
            buffer.size = handler->recv(handler, buffer.buf, 100);
            // sleep(1);
            if (buffer.size) {
                buffer.buf[buffer.size] = '\0';
                PRINTF("recv: %s\n", buffer.buf);
            }

        }
    }
    sleep(1);
    return 0;
}

#endif




