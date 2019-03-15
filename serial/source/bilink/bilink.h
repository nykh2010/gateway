#ifndef __SIMPLE_BILINK_H__
#define __SIMPLE_BILINK_H__

#include <stdint.h>
#include <signal.h>

#include "../payload.h"
#include "../rts/rts.h"



struct bilink_conn;

struct bilink_callbacks {
  // Called when a packet has been received by the rts module.
  void (* recv_data)(struct bilink_conn *ptr);
//  void (* recv_ctrl)(struct bilink_conn *ptr);
  void (* sent)(struct bilink_conn *ptr, int status, int num_tx);
};

struct bilink_conn {
    struct rts_conn rts;
    const struct bilink_callbacks * u;
    uint8_t selfaddr[8];
    long timeout_ms;
    uint8_t initkey[2];
    uint8_t newkey[2];
    uint8_t tx_max;
    uint8_t dataFilePath[128];
};

int bilink_open (struct bilink_conn * c, const struct bilink_callbacks * callbacks);

void bilink_close(struct bilink_conn *c);

int bilink_send_data (struct bilink_conn * c, struct simple_payload_buf * s);

#endif /*__SIMPLE_BILINK_H__*/
