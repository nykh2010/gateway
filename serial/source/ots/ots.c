

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../payload.h"
#include "../ots/ots.h"

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

// static void * ots_recv_routine(void * arg);
static int recv_from_serial (struct serial_handler * serial, uint8_t * data, int size) {
	struct ots_conn * c = (struct ots_conn *)serial;
	c->pld.rbuf.size = size;
	memcpy(c->pld.rbuf.buf, data, size);
	c->pld.rbuf.buf[size] = 0;
	PRINTF("serial recv %d bytes : %s\n", size, c->pld.rbuf.buf+1);
	c->u->recv(c);
	return size;
}

struct serial_callbacks ots_for_serial = {
		recv_from_serial
};

/*---------------------------------------------------------------------------*/
int ots_open (struct ots_conn * c, const struct ots_callbacks * callbacks) {
	PRINTF("ots_open\n");
  	c->u = callbacks;
  	c->pld.rbuf.size = 0;
  	c->pld.rbuf.buf[0] = 0;
	return (int)(serial_start(&c->pld.serial, &ots_for_serial));
}
/*---------------------------------------------------------------------------*/
void ots_close (struct ots_conn *c) {
	serial_close (&c->pld.serial);
}
/*---------------------------------------------------------------------------*/
int ots_send (struct ots_conn *c, struct simple_payload_buf * s) {
	int ret;
	PAYLOAD_ATTR_SET(c->pld, PAYLOAD_ATTR_OPENLOOP_SENDING);
	ret = serial_send_to_list(&c->pld.serial, s->buf, s->size);
	PAYLOAD_ATTR_RESET(c->pld, PAYLOAD_ATTR_OPENLOOP_SENDING);
	c->u->sent(c, ret, 1);
  	return ret;
}
/*---------------------------------------------------------------------------*/
int ots_send_ctrl (struct ots_conn *c, uint8_t ctrl) {
	int ret;
	struct simple_payload_buf pkt;
	pkt.size = 1;
	pkt.buf[SIMPLE_PAYLOAD_TYPE_COMM] = ctrl;
	PAYLOAD_ATTR_SET(c->pld, PAYLOAD_ATTR_OPENLOOP_SENDING);
	ret = serial_send_to_list(&c->pld.serial, pkt.buf, pkt.size);
	PAYLOAD_ATTR_RESET(c->pld, PAYLOAD_ATTR_OPENLOOP_SENDING);
	PRINTF("return ack\n");
  	return ret;
}


