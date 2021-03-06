

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
#include "../inifun/inirw.h"

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#define LOG_DEBUG(...) log_debug(__VA_ARGS__)
#ifndef LOG_DEBUG
#define LOG_DEBUG(...)
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
				pthread_mutex_lock(&c->data_req_mutex);
//				PRINTF("rts recv ack\n");
				PAYLOAD_ATTR_RESET(ptr->pld, PAYLOAD_ATTR_WAITING_ACK);
				pthread_cond_signal(&c->data_req_cond);
				pthread_mutex_unlock(&c->data_req_mutex);
//				LOG_DEBUG("recv ack.");
				break;
			case SIMPLE_PAYLOAD_BUSY :
				pthread_mutex_lock(&c->data_req_mutex);
//				PRINTF("rts recv busy\n");
				PAYLOAD_ATTR_SET(ptr->pld, PAYLOAD_ATTR_RETURN_BUSY);
				pthread_cond_signal(&c->data_req_cond);
				pthread_mutex_unlock(&c->data_req_mutex);
				LOG_DEBUG("recv busy.");
				break;
			case SIMPLE_PAYLOAD_OK :
				pthread_mutex_lock(&c->cmd_req_mutex);
//				PRINTF("rts recv ok\n");
				PAYLOAD_ATTR_RESET(ptr->pld, PAYLOAD_ATTR_WAITING_CMD);
				pthread_cond_signal(&c->cmd_req_cond);
				pthread_mutex_unlock(&c->cmd_req_mutex);
//				LOG_DEBUG("recv ok.");
				break;
			case SIMPLE_PAYLOAD_ERROR :
				pthread_mutex_lock(&c->cmd_req_mutex);
//				PRINTF("rts recv error\n");
				PAYLOAD_ATTR_SET(ptr->pld, PAYLOAD_ATTR_WAITING_CMD);
				pthread_cond_signal(&c->cmd_req_cond);
				pthread_mutex_unlock(&c->cmd_req_mutex);
				LOG_DEBUG("recv error.");
				break;
			default :
				c->u->recv_ctrl(c);
				break;
		}

	} else {
		// return ack
		ots_send_ctrl(ptr, SIMPLE_PAYLOAD_ACK);
		// receive data
//		LOG_DEBUG("recv data.");
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
	log_info("rts opened.");
	pthread_cond_init(&c->data_req_cond, NULL);
	pthread_mutex_init(&c->data_req_mutex, NULL);
	pthread_cond_init(&c->cmd_req_cond, NULL);
	pthread_mutex_init(&c->cmd_req_mutex, NULL);

	c->tx_max = readIntValue("[rts]", "maxTxTimes", CONFIG_PATH);
//	c->sndnxt = 0;
	c->is_tx = 0;
//	c->rx_miss = 0;
//	c->tx_max = 5;
	c->u = callbacks;
	return ots_open(&c->ots, &rts_for_ots);
}

void rts_close (struct rts_conn *c) {
	pthread_cond_destroy(&c->data_req_cond);
	pthread_mutex_destroy(&c->data_req_mutex);

	ots_close(&c->ots);
	log_info("ots closed.");
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
	pthread_mutex_lock(&c->data_req_mutex);
	PAYLOAD_ATTR_SET(c->ots.pld, PAYLOAD_ATTR_WAITING_ACK);
//	timer_wait_ms(msec);
//	pthread_cond_destroy(&c->data_req_cond);
	pthread_cond_timedwait(&c->data_req_cond, &c->data_req_mutex, &waitMoment);
	// have not receive ACK
	if(PAYLOAD_ATTR_GET(c->ots.pld, PAYLOAD_ATTR_WAITING_ACK)) {
		if (PAYLOAD_ATTR_GET(c->ots.pld, PAYLOAD_ATTR_RETURN_BUSY)) {
			PAYLOAD_ATTR_RESET(c->ots.pld, PAYLOAD_ATTR_RETURN_BUSY);
			ret = RTS_SEND_BUSY;
			LOG_DEBUG("rts return busy.");
		} else {
			PAYLOAD_ATTR_RESET(c->ots.pld, PAYLOAD_ATTR_WAITING_ACK);
			ret = RTS_SEND_TIMEOUT;
			LOG_DEBUG("rts wait ack timeout.");
		}
	} else {
//		PAYLOAD_ATTR_SET(c->ots.pld, PAYLOAD_ATTR_RELIABLE_SENDING);
		ret = RTS_SEND_OK;
	}
	pthread_mutex_unlock(&c->data_req_mutex);

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
		LOG_DEBUG("rts %dth send.", (c->is_tx + 1));
		if (ret == RTS_SEND_OK) {
			break;
		}

		if (ret == RTS_SEND_BUSY) {
			timer_wait_ms(RTS_BUSY_TIME_MS);
			continue;
		}
	}
	c->u->sent(c, ret, c->is_tx);
	// timeout
	return ret;
}

int rts_send_ctrl (struct rts_conn * c, uint8_t ctrl, long int msec) {
	int ret = -1;
	long usec_temp;
	struct timeval startMoment;
	struct timespec waitMoment;
	if (c == NULL) {
		return -1;
	}
	get_current_time(&startMoment);
	PRINTF("rts_send_ctrl sec=%ld usec=%ld\n", startMoment.tv_sec, startMoment.tv_usec);

    waitMoment.tv_sec = startMoment.tv_sec + msec/1000;
    usec_temp = (msec % 1000);
    usec_temp *= 1000;
    usec_temp += startMoment.tv_usec;
	if (usec_temp > 1000000) {
		waitMoment.tv_sec += 1;
		waitMoment.tv_nsec = (usec_temp - 1000000) * 1000;
	} else {
		waitMoment.tv_nsec = usec_temp * 1000;
	}
	LOG_DEBUG("ots send ctrl %02X.", ctrl);
	ots_send_ctrl(&c->ots, ctrl);
//	PRINTF("rts_send_ctrl sec=%ld nsec=%ld\n", waitMoment.tv_sec, waitMoment.tv_nsec);
//	LOG_DEBUG("ots send ctrl mutex lock.");
	pthread_mutex_lock(&c->cmd_req_mutex);
	PAYLOAD_ATTR_SET(c->ots.pld, PAYLOAD_ATTR_WAITING_CMD);
	pthread_cond_timedwait(&c->cmd_req_cond, &c->cmd_req_mutex, &waitMoment);

	// have not receive OK
	if(PAYLOAD_ATTR_GET(c->ots.pld, PAYLOAD_ATTR_WAITING_CMD)) {
		log_error("wait for OK timeout.");
		PAYLOAD_ATTR_RESET(c->ots.pld, PAYLOAD_ATTR_WAITING_CMD);
		ret = -1;
	} else {
//		LOG_DEBUG("got OK.");
		ret = 0;
	}
	pthread_mutex_unlock(&c->cmd_req_mutex);
	// PRINTF("rts_send_ctrl wait time %ld us\n",time_interval_us(&startMoment));
	// get_current_time(&startMoment);
	// PRINTF("rts_send_sub sec=%ld usec=%ld\n", startMoment.tv_sec, startMoment.tv_usec);
	return ret;
}
