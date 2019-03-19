#include "../bilink/bilink.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <sys/queue.h>
#include <string.h>
#include <stdarg.h>

#include "../config.h"
#include "../serial/serial.h"
#include "../bilink/bilink_packet.h"
#include "../packet/packet.h"
#include "..//payload.h"
#include "../rts/rts.h"
#include "../timer/timer.h"
/*--------------------------------------------------------------------------------*/
#define DEBUG_PRINTF 0
#if DEBUG_PRINTF
    #define PRINTF(...) printf(__VA_ARGS__)
#else
    #define PRINTF(...)
#endif
/*--------------------------------------------------------------------------------*/



/*--------------------------------------------------------------------------------*/
static void rts_recv_data (struct rts_conn *ptr) {


	serial_packet_entry_t * newEntry = NULL;
	struct serial_handler * handler = (struct serial_handler* )ptr;

    PRINTF("rts recv data over: %s\n", ptr->ots.pld.rbuf.buf);
    pthread_mutex_lock(&handler->readDataMut);
	newEntry = (serial_packet_entry_t *)malloc(sizeof(serial_packet_entry_t));

	newEntry->length = ptr->ots.pld.rbuf.size-1;

	newEntry->data = (uint8_t *)malloc(newEntry->length);
	memcpy(newEntry->data, ptr->ots.pld.rbuf.buf+1, newEntry->length);
	if (handler->readListLen < handler->readListMaxLen) {
		SIMPLEQ_INSERT_TAIL(&handler->readList, newEntry, next);
		handler->readListLen++;
		pthread_cond_signal(&handler->readDataCond);
	}
    pthread_mutex_unlock(&handler->readDataMut);
}

static void rts_recv_ctrl (struct rts_conn *ptr) {
//    PRINTF("rts recv ctrl over\n");
}

static void rts_sent (struct rts_conn *ptr, int status, int num_tx) {
//	PRINTF("sent by rts: status=%d num_tx=%d\n", status, num_tx);
}

static struct rts_callbacks rts_callback_obj = {
	rts_recv_data,
	rts_recv_ctrl,
	rts_sent,
};

int bilink_open (struct bilink_conn * c, const struct bilink_callbacks * callbacks) {
	int ret;
	uint64_to_addr(0x0123456789ABCDEF, c->selfaddr);
    c->timeout_ms = RTS_ACK_TIME_MS;
    c->initkey[0] = 0x5A;
    c->initkey[1] = 0xEF;
    c->newkey[0] = 0xAA;
    c->newkey[1] = 0x55;
    c->tx_max = 3;
    c->u = callbacks;
    ret = rts_open(&c->rts, &rts_callback_obj);

    return ret;
}

void bilink_close(struct bilink_conn *c) {
	rts_close(&c->rts);
}

int bilink_send_ctrl (struct bilink_conn * c, struct simple_payload_buf * s) {
	int ret = 0;
	return ret;
}
/*--------------------------------------------------------------------------------*/

int bilink_send_data (struct bilink_conn * c, struct simple_payload_buf * s) {
	int ret;
	uint8_t tx = 0;
	if (s->size <= 0 || s->size > SIMPLE_PAYLOAD_LENGTH_MAX) {
		return -1;
	}

	s->buf[SIMPLE_PAYLOAD_TYPE_INDEX] = SIMPLE_PAYLOAD_TYPE_DATA;
	s->size += 1;
	for (tx=0; tx<c->tx_max; tx++) {
	    ret = rts_send(&c->rts, s, c->timeout_ms);
	    if (RTS_SEND_OK == ret)
	    	break;
	}

	return ret;
}





