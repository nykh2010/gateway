#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../packet/frame.h"
#include "../packet/packet.h"
//#include "../thread/thread.h"
#include "../payload.h"
#include "../rts/rts.h"
#include "../ots/ots.h"
#include "../timer/timer.h"

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

static void recv_from_ots (struct ots_conn * ptr) {
	struct rts_conn * c = (struct rts_conn *)ptr;
	uint8_t ctrl;
	PAYLOAD_ATTR_SET(ptr->pld, PAYLOAD_ATTR_RELIABLE_RECVING);
	ctrl = ptr->pld.rbuf.buf[SIMPLE_PAYLOAD_TYPE_INDEX];

	if (IS_TYPE_BYTE_COMM(ctrl)) {
		// receive command
		switch (COMM_BYTE_MASK(ctrl)) {
			case SIMPLE_PAYLOAD_ACK :
				PRINTF("rts recv ack\n");
				PAYLOAD_ATTR_RESET(ptr->pld, PAYLOAD_ATTR_WAITING_ACK);
				break;
			case SIMPLE_PAYLOAD_BUSY :
				PRINTF("rts recv busy\n");
				PAYLOAD_ATTR_SET(ptr->pld, PAYLOAD_ATTR_RETURN_BUSY);
				break;
			default :
				c->u->recv_ctrl(c);
				break;
		}

	} else {
		// return ack
		ots_send_ctrl(ptr, SIMPLE_PAYLOAD_ACK);
		// receive data
		c->u->recv_data(c);

	}
}

static void sent_by_ots (struct ots_conn *ptr, int status, int num_tx) {
	PRINTF("sent by ots\n");
}

static struct ots_callbacks rts_for_ots = {
	recv_from_ots,
	sent_by_ots,
};

int rts_open (const char * dev, struct rts_conn * c, const struct rts_callbacks * callbacks) {
	PRINTF("rts_open\n");
//	c->sndnxt = 0;
	c->is_tx = 0;
//	c->rx_miss = 0;
	c->tx_max = 5;
	c->u = callbacks;
	return ots_open(dev, &c->ots, &rts_for_ots);
}

void rts_close (struct rts_conn *c) {
	ots_close(&c->ots);
}

static int rts_send_sub (struct rts_conn * c, struct simple_payload_buf * s, long int msec) {
//	struct simple_payload * p = &c->ots.pld;
	PAYLOAD_ATTR_SET(c->ots.pld, PAYLOAD_ATTR_RELIABLE_SENDING);
	// PRINTF("rts_send start\n");
	ots_send(&c->ots, s);
	PAYLOAD_ATTR_SET(c->ots.pld, PAYLOAD_ATTR_WAITING_ACK);

	while (msec--) {
		timer_wait_ms(1);
		// have not receive ACK
		if(PAYLOAD_ATTR_GET(c->ots.pld, PAYLOAD_ATTR_WAITING_ACK)) {
			if (msec <= 0) {
				PAYLOAD_ATTR_RESET(c->ots.pld, PAYLOAD_ATTR_WAITING_ACK);
				break;
			}
			if (PAYLOAD_ATTR_GET(c->ots.pld, PAYLOAD_ATTR_RETURN_BUSY)) {
				PAYLOAD_ATTR_RESET(c->ots.pld, PAYLOAD_ATTR_RETURN_BUSY);
				return RTS_SEND_BUSY;
			}
		} else {
			PAYLOAD_ATTR_SET(c->ots.pld, PAYLOAD_ATTR_RELIABLE_SENDING);
			return RTS_SEND_OK;
		}

	}
	PAYLOAD_ATTR_RESET(c->ots.pld, PAYLOAD_ATTR_RELIABLE_SENDING);
	// timeout
	return RTS_SEND_TIMEOUT;
}

int rts_send (struct rts_conn * c, struct simple_payload_buf * s, long int msec) {
	int ret;

	c->is_tx = 0;
	for ((c->is_tx)=0; (c->is_tx) < (c->tx_max); (c->is_tx)++) {
		ret = rts_send_sub(c, s, msec);
		if (ret != RTS_SEND_BUSY) {
			c->is_tx++;
			break;
		}
		timer_wait_ms(1);
	}
	c->u->sent(c, ret, c->is_tx);
	// timeout
	return ret;
}

