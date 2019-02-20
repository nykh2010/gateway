#ifndef __SIMPLE_PACKET_H__
#define __SIMPLE_PACKET_H__

#include <stdint.h>
#include "frame.h"

#define SIMPLE_SERIAL_ASYNC_PACKET_MAX_SIZE       256
#define SIMPLE_SERIAL_ASYNC_PACKET_BUF_MAX_SIZE   260

typedef uint8_t packet_phase_t;

enum {
    PHASE_HEAD_0 = 0x00,
    PHASE_HEAD_1,
    PHASE_LENGTH,
    PHASE_PAYLOAD,
};

typedef struct _simple_packet {
    int size;
    uint8_t buf[SIMPLE_SERIAL_ASYNC_PACKET_BUF_MAX_SIZE];
} simple_packet_t;

typedef struct _simple_packet_recv {
    int size;
    int sizeToRecv;
    int sizeRecieved;
    packet_phase_t phase;
    pthread_mutex_t mutex;
    uint8_t buf[SIMPLE_SERIAL_ASYNC_PACKET_BUF_MAX_SIZE];
} simple_packet_recv_t;

// #define simple_packet_buf_addr(pkt) ((pkt)->buf+(pkt)->sizeRecieved)
#define SIMPLE_PACKET_BUF_ADDR(pkt)  ((pkt)->buf+(pkt)->sizeRecieved)
#define SIMPLE_PACKET_PAYLOAD_ADDR(pkt)  ((pkt)->buf+SIMPLE_SERIAL_ASYNC_FRAME_PAYLOAD_INDEX)
#define SIMPLE_PACKET_PAYLOAD_SIZE(pkt)  ((pkt)->buf[SIMPLE_SERIAL_ASYNC_FRAME_LENGTH_INDEX]-SIMPLE_SERIAL_ASYNC_FRAME_CHECK_SIZE)

void clear_simple_packet (simple_packet_t * pkt);
int create_simple_packet (simple_packet_t * dest, const uint8_t * src, int size);
void clear_simple_packet_recv (simple_packet_recv_t * pkt);
int parse_simple_packet (simple_packet_recv_t * pkt);

#endif /*__SIMPLE_PACKET_H__*/
