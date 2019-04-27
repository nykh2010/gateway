#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <sys/queue.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <json-c/json.h>
#include <signal.h>

#include "../config.h"

#include "../bilink/bilink.h"
#include "../entrydef.h"
#include "../serialp/serial.h"
#include "../bilink/bilink_packet.h"
#include "../packet/packet.h"
#include "../payload.h"
#include "../rts/rts.h"
#include "../timer/timer.h"
#include "../client/client.h"
#include "../server/server.h"
#include "entry.h"

/*--------------------------------------------------------------------------------*/
#define DEBUG_PRINTF 1
#if DEBUG_PRINTF
    #define PRINTF(...) printf(__VA_ARGS__)
#else
    #define PRINTF(...)
#endif
/*--------------------------------------------------------------------------------*/
struct entry tEntry;


/*--------------------------------------------------------------------------------*/
static void bilink_recv_data (struct bilink_conn *ptr) {

}

static void bilink_sent (struct bilink_conn *ptr, int status, int num_tx) {
//	PRINTF("sent by rts: status=%d num_tx=%d\n", status, num_tx);
}

static struct bilink_callbacks bilink_callback_obj = {
	bilink_recv_data,
	bilink_sent,
};


/*--------------------------------------------------------------------------------*/
//int entry_open (struct entry * e) {
int entry_open (void) {
	if (bilink_open(&tEntry.bhdr, &bilink_callback_obj) != 0) {
		return -1;
	} else {
		log_info("bilink opened.");
		tEntry.shdr.parent = &tEntry;
		tEntry.task_on = 0;
		log_debug("task_on %d", tEntry.task_on);
		tEntry.thdr.parent = &tEntry;
		log_debug("task_on %ld : %d", &tEntry.thdr.parent->task_on, tEntry.thdr.parent->task_on);
		if (server_open(&tEntry.shdr) != 0) {
			bilink_close(&tEntry.bhdr);
			log_error("bilink closed.");
			return -1;
		} else {
			log_info("server opened.");

			tEntry.chdr.parent = &tEntry;
			if (client_open(&tEntry.chdr) != 0) {
				server_close(&tEntry.shdr);
				log_error("server closed.");
				bilink_close(&tEntry.bhdr);
				log_error("bilink closed.");
				return -1;
			} else {
//				int tmp;
				log_info("client opened.");
				log_info("entry opened.");


				return 0;
			}

			return 0;
		}

	}
}

void entry_close (struct entry * e) {
	client_close(&e->chdr);
	log_error("client closed.");
	server_open(&e->shdr);
	log_error("server closed.");
	bilink_close(&e->bhdr);
	log_error("bilink closed.");
}

#if 1
/*-----------------------------------------------------------------------------------------------*/
int entry_send_data (const char * data, int size) {
	int ret;
	if (data == NULL || size > SIMPLE_PAYLOAD_LENGTH_MAX) {
		return -1;
	}
	if (size <= 0) {
		return -1;
	}
	struct simple_payload_buf payload;
	payload.size = size;
	memcpy(payload.payload.data, data, size);

    ret = bilink_send_data (&tEntry.bhdr, &payload);
	return ret;
}


int entry_receive_data (char * data, int msec) {
	int ret = 0;

	ret = serial_receive_from_list (&tEntry.bhdr.rts.ots.pld.serial, (uint8_t *)data, msec);

	return ret;
}

#endif
