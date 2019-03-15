

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
#include "../source/ots/ots.h"


/*--------------------------------------------------------------------------------*/
#define DEBUG_PRINTF 1
#if DEBUG_PRINTF
    #define PRINTF(...) printf(__VA_ARGS__)
#else
    #define PRINTF(...)
#endif
/*--------------------------------------------------------------------------------*/


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
int ots_test (int argc, char * argv[]) {
    // variables define
    char mode;
    long int count = 0;
    struct timeval now;
    struct simple_payload_buf payload;
    struct ots_conn ots_conn_obj;
    struct ots_callbacks ots_callback_obj;
    ots_callback_obj.recv = ots_recv;
	ots_callback_obj.sent = ots_sent;


    if (argc > 0) {
        ots_open(&ots_conn_obj, &ots_callback_obj);

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

    sleep(1);
    return 0;
}
