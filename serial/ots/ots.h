#ifndef __SIMPLE_OTS_H__
#define __SIMPLE_OTS_H__

#include <stdint.h>

#include "../packet/frame.h"
#include "../packet/packet.h"
#include "../payload.h"


struct ots_conn;

struct ots_callbacks {
  // Called when a packet has been received by the ots module.
  void (* recv)(struct ots_conn *ptr);
  void (* sent)(struct ots_conn *ptr, int status, int num_tx);
};

struct ots_conn {
    struct simple_payload pld;
    const struct ots_callbacks * u;
};

int ots_open (struct ots_conn * c, const struct ots_callbacks * callbacks);

void ots_close(struct ots_conn *c);

int ots_send (struct ots_conn *c, struct simple_payload_buf * s);

int ots_send_ctrl (struct ots_conn *c, uint8_t ctrl);

#endif /*__SIMPLE_OTS_H__*/
