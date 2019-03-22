#ifndef __SIMPLE_RTS_H__
#define __SIMPLE_RTS_H__

#include <stdint.h>
#include <signal.h>

#include "../payload.h"
#include "../ots/ots.h"

enum {
	RTS_SEND_OK = 0x00,
	RTS_SEND_ERROR,
	RTS_SEND_BUSY,
	RTS_SEND_TIMEOUT,
};


struct rts_conn;

struct rts_callbacks {
  // Called when a packet has been received by the rts module.
  void (* recv_data)(struct rts_conn *ptr);
  void (* recv_ctrl)(struct rts_conn *ptr);
  void (* sent)(struct rts_conn *ptr, int status, int num_tx);
};

struct rts_conn {
    struct ots_conn ots;
    const struct rts_callbacks * u;
    pthread_cond_t data_req_cond;
    pthread_mutex_t data_req_mutex;
    pthread_cond_t cmd_req_cond;
    pthread_mutex_t cmd_req_mutex;
//	uint8_t sndnxt;
	uint8_t is_tx;
//	uint8_t rx_miss;
	uint8_t tx_max;
};

int rts_open (struct rts_conn * c, const struct rts_callbacks * callbacks);

void rts_close(struct rts_conn *c);

int rts_send (struct rts_conn * c, struct simple_payload_buf * s, long int msec);


#endif
