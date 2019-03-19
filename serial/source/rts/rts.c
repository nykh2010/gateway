

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include "../config.h"
#include "../ots/ots.h"
#include "../packet/frame.h"
#include "../packet/packet.h"
#include "../payload.h"
#include "../timer/timer.h"
#include "../rts/rts.h"

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
//	PAYLOAD_ATTR_SET(ptr->pld, PAYLOAD_ATTR_RELIABLE_RECVING);
	ctrl = ptr->pld.rbuf.buf[SIMPLE_PAYLOAD_TYPE_INDEX];

	if (IS_TYPE_BYTE_COMM(ctrl)) {
		// receive command
		switch (COMM_BYTE_MASK(ctrl)) {
			case SIMPLE_PAYLOAD_ACK :
				pthread_mutex_lock(&c->req_mutex);
//				PRINTF("rts recv ack\n");
				PAYLOAD_ATTR_RESET(ptr->pld, PAYLOAD_ATTR_WAITING_ACK);
				pthread_cond_signal(&c->req_cond);
				pthread_mutex_unlock(&c->req_mutex);
				break;
			case SIMPLE_PAYLOAD_BUSY :
				pthread_mutex_lock(&c->req_mutex);
//				PRINTF("rts recv busy\n");
				PAYLOAD_ATTR_SET(ptr->pld, PAYLOAD_ATTR_RETURN_BUSY);
				pthread_cond_signal(&c->req_cond);
				pthread_mutex_unlock(&c->req_mutex);
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
//	PRINTF("sent by ots\n");
}

static struct ots_callbacks rts_for_ots = {
	recv_from_ots,
	sent_by_ots,
};

int rts_open (struct rts_conn * c, const struct rts_callbacks * callbacks) {
	PRINTF("rts_open\n");
	pthread_cond_init(&c->req_cond, NULL);
	pthread_mutex_init(&c->req_mutex, NULL);
//	c->sndnxt = 0;
	c->is_tx = 0;
//	c->rx_miss = 0;
	c->tx_max = 5;
	c->u = callbacks;
	return ots_open(&c->ots, &rts_for_ots);
}

void rts_close (struct rts_conn *c) {
	pthread_cond_destroy(&c->req_cond);
	pthread_mutex_destroy(&c->req_mutex);
	ots_close(&c->ots);
}

static int rts_send_sub (struct rts_conn * c, struct simple_payload_buf * s, long msec) {
	int ret = RTS_SEND_TIMEOUT;
	long usec_temp;
	struct timeval startMoment;
	struct timespec waitMoment;
	if (c == NULL || s == NULL) {
		return RTS_SEND_ERROR;
	}
//	struct simple_payload * p = &c->ots.pld;
//	PAYLOAD_ATTR_SET(c->ots.pld, PAYLOAD_ATTR_RELIABLE_SENDING);
	// PRINTF("rts_send start\n");
	get_current_time(&startMoment);
	PRINTF("rts_send_sub sec=%ld usec=%ld\n", startMoment.tv_sec, startMoment.tv_usec);
//    if (msec >= 1000) {
    waitMoment.tv_sec = startMoment.tv_sec + msec/1000;
//    }
    usec_temp = (msec % 1000);
    usec_temp *= 1000;
    usec_temp += startMoment.tv_usec;
	if (usec_temp > 1000000) {
		waitMoment.tv_sec += 1;
		waitMoment.tv_nsec = (usec_temp - 1000000) * 1000;
	} else {
		waitMoment.tv_nsec = usec_temp * 1000;
	}

	ots_send(&c->ots, s);
	PRINTF("rts_send_sub sec=%ld nsec=%ld\n", waitMoment.tv_sec, waitMoment.tv_nsec);
	pthread_mutex_lock(&c->req_mutex);
	PAYLOAD_ATTR_SET(c->ots.pld, PAYLOAD_ATTR_WAITING_ACK);
//	timer_wait_ms(msec);
//	pthread_cond_destroy(&c->req_cond);
	pthread_cond_timedwait(&c->req_cond, &c->req_mutex, &waitMoment);
	// have not receive ACK
	if(PAYLOAD_ATTR_GET(c->ots.pld, PAYLOAD_ATTR_WAITING_ACK)) {
		if (PAYLOAD_ATTR_GET(c->ots.pld, PAYLOAD_ATTR_RETURN_BUSY)) {
			PAYLOAD_ATTR_RESET(c->ots.pld, PAYLOAD_ATTR_RETURN_BUSY);
			ret = RTS_SEND_BUSY;
		} else {
			PAYLOAD_ATTR_RESET(c->ots.pld, PAYLOAD_ATTR_WAITING_ACK);
			ret = RTS_SEND_TIMEOUT;
		}
	} else {
//		PAYLOAD_ATTR_SET(c->ots.pld, PAYLOAD_ATTR_RELIABLE_SENDING);
		ret = RTS_SEND_OK;
	}
	pthread_mutex_unlock(&c->req_mutex);

//	PAYLOAD_ATTR_RESET(c->ots.pld, PAYLOAD_ATTR_RELIABLE_SENDING);
	PRINTF("rts_send_sub wait time %ld us\n",time_interval_us(&startMoment));
	get_current_time(&startMoment);
	PRINTF("rts_send_sub sec=%ld usec=%ld\n", startMoment.tv_sec, startMoment.tv_usec);
	return ret;
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
		timer_wait_ms(RTS_BUSY_TIME_MS);
	}
	c->u->sent(c, ret, c->is_tx);
	// timeout
	return ret;
}

