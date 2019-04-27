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
#include "../serialp/serial.h"
#include "../bilink/bilink_packet.h"
#include "../packet/packet.h"
#include "..//payload.h"
#include "../rts/rts.h"
#include "../timer/timer.h"
#include "../inifun/inirw.h"
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

	int tmp;
	serial_packet_entry_t * newEntry = NULL;
	struct serial_handler * handler = (struct serial_handler* )ptr;
#if 0
	log_debug("rts recv %d bytes data", ptr->ots.pld.rbuf.size-1);

	for (tmp=0; tmp<(ptr->ots.pld.rbuf.size-1); tmp++) {
		printf("%02X ",  ptr->ots.pld.rbuf.payload.data[tmp]);
	}
	printf("\n");
#endif

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
	char tmp_str[64];
	FILE *fp;
	int len;

	if ((fp = fopen(SN_PATH, "r")) == NULL) {
		log_error("fopen %s error.", SN_PATH);
	}
	fgets(tmp_str, 64, fp);
	// close the file
	fclose(fp);
	tmp_str[16] = 0;
	log_debug("selfId = %s", tmp_str);
	str_to_hex(c->selfaddr, tmp_str, 16);

    c->timeout_ms = readIntValue("[bilink]", "rtsSendTimeoutMs", CONFIG_PATH);
    log_debug("rtsSendTimeoutMs = %d", (int)c->timeout_ms);

	len = IniReadValue("[bilink]", "initKey", tmp_str, CONFIG_PATH);
	tmp_str[len] = '\0';
	str_to_hex(c->initkey, tmp_str, 4);
	log_debug("initKey %s", tmp_str);

	len = IniReadValue("[bilink]", "newKey", tmp_str, CONFIG_PATH);
	tmp_str[len] = '\0';
	str_to_hex(c->newkey, tmp_str, 4);
	log_debug("newKey = %s", tmp_str);

    c->tx_max = 1;
    c->u = callbacks;
    if ((ret = rts_open(&c->rts, &rts_callback_obj)) == 0) {

        log_info("rts successfully opened.");
//        log_debug("wait for write semaphore.");
        sem_wait(&c->rts.ots.pld.serial.writeDataSem);
//        log_debug("wait for read semaphore.");
        sem_wait(&c->rts.ots.pld.serial.readDataSem);
        log_info("got read and write semaphores.");
//        log_debug("wait for MCU status.");

        if (0 == rts_send_ctrl(&c->rts, SIMPLE_PAYLOAD_STATUS, 500)) {
        	log_info("got MCU status.");
        	if (0 != rts_send_ctrl(&c->rts, SIMPLE_PAYLOAD_RESE_MODE, 500)) {
        		log_error("Radio change to RESE mode error.");
        	} else {
        		log_info("Set radio to RESE status.");
        	}

        } else {
        	log_error("The MCU status is not OK.");
//        	return -1;
        }

    } else {
    	log_error("rts open failed.");
    }

    return ret;
}

void bilink_close(struct bilink_conn *c) {
	rts_close(&c->rts);
    log_info("rts closed.");
	free(c);
}

int bilink_send_ctrl (struct bilink_conn * c, struct simple_payload_buf * s) {
	int ret = 0;
	return ret;
}
/*--------------------------------------------------------------------------------*/

int bilink_send_data (struct bilink_conn * c, struct simple_payload_buf * s) {
	int ret;
	uint8_t tx = 0;
//	log_info("bilink send a pkt.");
	if (s->size <= 0 || s->size > SIMPLE_PAYLOAD_LENGTH_MAX) {
		return -1;
	}

//	s->buf[SIMPLE_PAYLOAD_TYPE_INDEX] = SIMPLE_PAYLOAD_TYPE_DATA;
	s->payload.ctrl = SIMPLE_PAYLOAD_TYPE_DATA;
	s->size += 1;

	for (tx=0; tx<c->tx_max; tx++) {
	    ret = rts_send(&c->rts, s, c->timeout_ms);
	    if (RTS_SEND_OK == ret)
	    	break;
	}

	return ret;
}





