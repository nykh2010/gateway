#ifndef __SIMPLE_OTS_H__
#define __SIMPLE_OTS_H__

#include <stdint.h>
#include "../packet/frame.h"
#include "../packet/packet.h"
//#include "../thread/thread.h"
#include "../payload.h"

// typedef struct _simple_ots {
//     // struct _simple_ots * this;
//     simple_packet_t * rpkt;
//     simple_packet_t * spkt;

// } simple_ots_t;


struct ots_conn;

struct ots_callbacks {
  // Called when a packet has been received by the ots module.
  void (* recv)(struct ots_conn *ptr);
  void (* sent)(struct ots_conn *ptr, int status, int num_tx);
};

struct ots_conn {
    struct simple_payload pld;
    const struct ots_callbacks * u;
    // void * hdl;  // to store bus handler
    // unsigned long int thread;
};

int ots_open (const char * dev, struct ots_conn * c, const struct ots_callbacks * callbacks);

void ots_close(struct ots_conn *c);

int ots_send (struct ots_conn *c, struct simple_payload_buf * s);

int ots_send_ctrl (struct ots_conn *c, uint8_t ctrl);

//void ots_input(struct simple_payload * p);

// void ots_sent(struct simple_payload * p, int status, int num_tx);


#endif /*__SIMPLE_OTS_H__*/
