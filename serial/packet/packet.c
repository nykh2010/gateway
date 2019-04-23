

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../config.h"
#include "../packet/frame.h"
#include "../packet/packet.h"

#define DEBUG_PRINTF 0
#if DEBUG_PRINTF
    #define PRINTF(...) printf(__VA_ARGS__)
#else
    #define PRINTF(...)
#endif

// block Check Character, XOR chaeck
static uint8_t bcc (const uint8_t * data, int size) {
    uint8_t xorchar = 0;
    int i = 0;
    for (i=0; i<size; i++) {
        xorchar ^= *(data + i);
    }
    return xorchar;
}

void clear_simple_packet (simple_packet_t * pkt) {
    pkt->size = 0;
    pkt->buf[SIMPLE_SERIAL_ASYNC_FRAME_LENGTH_INDEX] = 0; 
}

int create_simple_packet (simple_packet_t * dest, const uint8_t * src, int size) {
    if (dest == NULL) {
    	log_error("error.");
        return -1;
    }
    if (size > (SIMPLE_SERIAL_ASYNC_PACKET_BUF_MAX_SIZE-4)) {
    	log_error("error.");
        return -1;
    }
    dest->buf[SIMPLE_SERIAL_ASYNC_FRAME_HEAD_BYTE_0_INDEX] = SIMPLE_SERIAL_ASYNC_FRAME_HEAD_BYTE_0;
    dest->buf[SIMPLE_SERIAL_ASYNC_FRAME_HEAD_BYTE_1_INDEX] = SIMPLE_SERIAL_ASYNC_FRAME_HEAD_BYTE_1;
    dest->buf[SIMPLE_SERIAL_ASYNC_FRAME_LENGTH_INDEX] = size+1;
    memcpy((dest->buf+SIMPLE_SERIAL_ASYNC_FRAME_PAYLOAD_INDEX), src, size);
    dest->buf[SIMPLE_SERIAL_ASYNC_FRAME_PAYLOAD_INDEX+size] = bcc(src, size);
    dest->size = size + 4;
    return dest->size;
}

void clear_simple_packet_recv (simple_packet_recv_t * pkt) {
    pkt->size = 0;
    pkt->phase = PHASE_HEAD_0;
    pkt->sizeToRecv = 1;
    pkt->sizeRecieved = 0;
    pkt->buf[SIMPLE_SERIAL_ASYNC_FRAME_LENGTH_INDEX] = 0; 
}

int parse_simple_packet (simple_packet_recv_t * pkt) {
    if (pkt->size == 0) {
//    	log_error("error.");
        return -1;
    }
    if (pkt->size > SIMPLE_SERIAL_ASYNC_PACKET_BUF_MAX_SIZE) {
    	log_error("error.");
        return -1;
    }

    if (pkt->phase == PHASE_HEAD_0) {
        if (SIMPLE_SERIAL_ASYNC_FRAME_HEAD_BYTE_0 == *(SIMPLE_PACKET_BUF_ADDR(pkt))) {
            PRINTF("recvd HEAD_0\n");
            pkt->phase = PHASE_HEAD_1;
            pkt->sizeRecieved = 1;
            pkt->sizeToRecv = 1;
            pkt->size = 0;
            return 1;
        }
    } else if (pkt->phase == PHASE_HEAD_1) {
        if (SIMPLE_SERIAL_ASYNC_FRAME_HEAD_BYTE_1 == *(SIMPLE_PACKET_BUF_ADDR(pkt))) {
            PRINTF("recvd HEAD_1\n");
            pkt->phase = PHASE_LENGTH;
            pkt->sizeRecieved += 1;
            pkt->sizeToRecv = 1;
            pkt->size = 0;
            return 1;
        } else {
            pkt->phase = PHASE_HEAD_0;
            pkt->sizeRecieved = 0;
            pkt->sizeToRecv = 1;
            pkt->size = 0;
            return 1;
        }
    } else if (pkt->phase == PHASE_LENGTH) {
        if (*(SIMPLE_PACKET_BUF_ADDR(pkt)) > 1) {
            PRINTF("recvd LENGTH %02X\n", *(SIMPLE_PACKET_BUF_ADDR(pkt)));
            pkt->phase = PHASE_PAYLOAD;
            pkt->sizeToRecv = (int)(*(SIMPLE_PACKET_BUF_ADDR(pkt)));
            pkt->sizeRecieved += 1;
            pkt->size = 0;
            return pkt->sizeToRecv;
        } else {
            pkt->phase = PHASE_HEAD_0;
            pkt->sizeRecieved = 0;
            pkt->sizeToRecv = 1;
            pkt->size = 0;
            return 1;
        }
    } else if (pkt->phase == PHASE_PAYLOAD) {
        if (pkt->size >= pkt->sizeToRecv) {
            PRINTF("received PAYLOAD\n");
            pkt->sizeRecieved += pkt->sizeToRecv;
            pkt->sizeToRecv = 0;
            if (bcc((pkt->buf + SIMPLE_SERIAL_ASYNC_FRAME_PAYLOAD_INDEX), pkt->buf[SIMPLE_SERIAL_ASYNC_FRAME_LENGTH_INDEX]) != 0) {
                PRINTF("check PAYLOAD failed\n");
                pkt->phase = PHASE_HEAD_0;
                pkt->sizeRecieved = 0;
                pkt->sizeToRecv = 1;
                pkt->size = 0;
                return 1;
            }
            pkt->phase = PHASE_HEAD_0;
            pkt->sizeRecieved = 0;
            pkt->sizeToRecv = 1;
            pkt->size = 0;
            return 0;
        } else {
            PRINTF("receiving PAYLOAD %d bytes\n", pkt->size);
            pkt->sizeRecieved += pkt->size;
            pkt->sizeToRecv -= pkt->size;
            pkt->size = 0;
            return pkt->sizeToRecv;
        }
    }
    pkt->size = 0;
    return -1;
}

