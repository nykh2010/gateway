#ifndef __SIMPLE_PACKET_H__
#define __SIMPLE_PACKET_H__

#include <stdint.h>
#include <pthread.h>

#include "../packet/frame.h"

#define SIMPLE_SERIAL_ASYNC_PACKET_MAX_SIZE       256
#define SIMPLE_SERIAL_ASYNC_PACKET_BUF_MAX_SIZE   260


typedef struct _simple_packet {
    int size;
    uint8_t buf[SIMPLE_SERIAL_ASYNC_PACKET_BUF_MAX_SIZE];
} simple_packet_t;

typedef uint8_t packet_phase_t;

enum {
    PHASE_HEAD_0 = 0x00,
    PHASE_HEAD_1,
    PHASE_LENGTH,
    PHASE_PAYLOAD,
};

typedef struct _simple_packet_recv {
    int size;
    uint8_t buf[SIMPLE_SERIAL_ASYNC_PACKET_BUF_MAX_SIZE];
    int sizeToRecv;
    int sizeRecieved;
    packet_phase_t phase;
    pthread_mutex_t mutex;
} simple_packet_recv_t;

// #define simple_packet_buf_addr(pkt) ((pkt)->buf+(pkt)->sizeRecieved)
#define SIMPLE_PACKET_BUF_ADDR(p)  ((p)->buf+(p)->sizeRecieved)
#define SIMPLE_PACKET_PAYLOAD_SIZE(p)  ((p)->buf[SIMPLE_SERIAL_ASYNC_FRAME_LENGTH_INDEX]-1)

// for address 
#define SIMPLE_PACKET_HEAD_0_ADDR(p)  ((p)->buf)
#define SIMPLE_PACKET_HEAD_1_ADDR(p)  ((p)->buf + SIMPLE_SERIAL_ASYNC_FRAME_HEAD_BYTE_1_INDEX)
#define SIMPLE_PACKET_LENGTH_ADDR(p)  ((p)->buf + SIMPLE_SERIAL_ASYNC_FRAME_LENGTH_INDEX)
#define SIMPLE_PACKET_PAYLOAD_ADDR(p)  ((p)->buf + SIMPLE_SERIAL_ASYNC_FRAME_PAYLOAD_INDEX)
#define SIMPLE_PACKET_CHECK_ADDR(p)  ((p)->buf + SIMPLE_SERIAL_ASYNC_FRAME_PAYLOAD_INDEX + (p)->buf[SIMPLE_SERIAL_ASYNC_FRAME_LENGTH_INDEX])
// for byte data
#define SIMPLE_PACKET_HEAD_0_BYTE(p)  ((p)->buf[SIMPLE_SERIAL_ASYNC_FRAME_HEAD_BYTE_0_INDEX])
#define SIMPLE_PACKET_HEAD_1_BYTE(p)  ((p)->buf[SIMPLE_SERIAL_ASYNC_FRAME_HEAD_BYTE_1_INDEX])
#define SIMPLE_PACKET_LENGTH_BYTE(p)  ((p)->buf[SIMPLE_SERIAL_ASYNC_FRAME_LENGTH_INDEX])
#define SIMPLE_PACKET_CHECK_BYTE(p)  ((p)->buf[SIMPLE_SERIAL_ASYNC_FRAME_PAYLOAD_INDEX + (p)->buf[SIMPLE_SERIAL_ASYNC_FRAME_LENGTH_INDEX]])

void clear_simple_packet (simple_packet_t * pkt);
int create_simple_packet (simple_packet_t * dest, const uint8_t * src, int size);
void clear_simple_packet_recv (simple_packet_recv_t * pkt);
int parse_simple_packet (simple_packet_recv_t * pkt);

#endif /*__SIMPLE_PACKET_H__*/
