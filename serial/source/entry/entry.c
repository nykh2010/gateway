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
#include <sqlite3.h>
#include "../config.h"

#include "../bilink/bilink.h"
#include "../entrydef.h"
#include "../serial/serial.h"
#include "../bilink/bilink_packet.h"
#include "../packet/packet.h"
#include "../payload.h"
#include "../rts/rts.h"
#include "../timer/timer.h"
#include "../bi_client/bi_client.h"
#include "../bi_server/bi_server.h"
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
//	struct entry * e = &tEntry;
//	e->thdr = NULL;
	if (bilink_open(&tEntry.bhdr, &bilink_callback_obj) != 0) {
		return -1;
	} else {
//		tEntry.bhdr.parent = &tEntry;
		if (server_open(&tEntry.shdr) != 0) {
			bilink_close(&tEntry.bhdr);
			return -1;
		} else {
			tEntry.shdr.parent = &tEntry;
			if (client_open(&tEntry.chdr) != 0) {
				server_close(&tEntry.shdr);
				bilink_close(&tEntry.bhdr);
				return -1;
			} else {
				// opened
				tEntry.chdr.parent = &tEntry;
			}
		}
	}
	PRINTF("entry_open : entry opened\n");
    // open client
    return 0;
}

void entry_close (struct entry * e) {
	client_close(&e->chdr);
	server_open(&e->shdr);
	bilink_close(&e->bhdr);
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
	memcpy(payload.buf+1, data, size);
	// payload.buf[0] = 0x80;
	payload.payload.ctrl = SIMPLE_PAYLOAD_TYPE_DATA;
	payload.size += 1;
    ret = bilink_send_data (&tEntry.bhdr, &payload);
	return ret;
}


int entry_receive_data (char * data, int msec) {
	int ret = 0;

	ret = serial_receive_from_list (&tEntry.bhdr.rts.ots.pld.serial, (uint8_t *)data, msec);

	return ret;
}

#endif
