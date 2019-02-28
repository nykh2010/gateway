#ifndef __PAYLOAD_H__
#define __PAYLOAD_H__

#include <stdio.h>
#include <stdint.h>
#include "./serial/serial.h"

#define SIMPLE_PAYLOAD_LENGTH_MAX           256

#define SIMPLE_PAYLOAD_CTRL_INDEX           0x00
// two kinds of payload : COMM and DATA
// It is set by the bit7 of CTRL byte
// bit7 = 1 : it is a data payload
// bit7 = 0 : it is a command payload
#define SIMPLE_PAYLOAD_TYPE_INDEX           0x00
#define SIMPLE_PAYLOAD_TYPE_MASK            0x80
#define SIMPLE_PAYLOAD_TYPE_COMM            0x00
#define SIMPLE_PAYLOAD_TYPE_DATA            0x80
//
#define SIMPLE_PAYLOAD_DATA_INDEX           0x01
// 
#define SIMPLE_PAYLOAD_CONF_INDEX           0x01

#define SIMPLE_PAYLOAD_COMM_MASK            0x7F
#define SIMPLE_PAYLOAD_NULL                 0x00
#define SIMPLE_PAYLOAD_BUSY                 0x01
#define SIMPLE_PAYLOAD_ERROR                0x02
#define SIMPLE_PAYLOAD_OK                   0x03
#define SIMPLE_PAYLOAD_STATUS               0x04
#define SIMPLE_PAYLOAD_RESET                0x05
// ...
#define SIMPLE_PAYLOAD_ACK                  0x7F


enum {
	PAYLOAD_ATTR_OPENLOOP_SENDING = 0x00,
	PAYLOAD_ATTR_RELIABLE_SENDING,

	PAYLOAD_ATTR_OPENLOOP_RECVING,
	PAYLOAD_ATTR_RELIABLE_RECVING,
	
	PAYLOAD_ATTR_RETURN_BUSY,
	PAYLOAD_ATTR_WAITING_ACK,
	PAYLOAD_ATTR_MAX,
};



typedef uint32_t simple_payload_attr_t;

struct simple_payload_buf {
	int size;
	uint8_t buf[SIMPLE_PAYLOAD_LENGTH_MAX];
};

struct simple_payload {
	struct serial_handler serial;
	struct simple_payload_buf rbuf;
	simple_payload_attr_t attr;
};

typedef struct simple_payload simple_payload_t;
typedef struct simple_payload_buf simple_payload_buf_t;

// // byte
#define IS_TYPE_BYTE_COMM(b)            (((b) & SIMPLE_PAYLOAD_TYPE_MASK) == SIMPLE_PAYLOAD_TYPE_COMM)
#define COMM_BYTE_MASK(b)               ((b) & SIMPLE_PAYLOAD_COMM_MASK)

// #define SIMPLE_PAYLOAD_INIT(p) do {
// 	(p)->s.size = 0;
// 	(p)->r.size = 0;
// 	(p)->attr = 0;
// } while(0)

// // recv
// #define SIMPLE_PAYLOAD_TYPE_BYTE(p)          ((p)->r->buf[SIMPLE_PAYLOAD_TYPE_INDEX])
// #define SIMPLE_PAYLOAD_RECV_TYPE_BYTE(p)     ((p)->r->buf[SIMPLE_PAYLOAD_TYPE_INDEX])
// #define SIMPLE_PAYLOAD_SEND_TYPE_BYTE(p)     ((p)->s->buf[SIMPLE_PAYLOAD_TYPE_INDEX])


// #define IS_SIMPLE_PAYLOAD_TYPE_COMM(p)  (((p)->r->buf[SIMPLE_PAYLOAD_TYPE_INDEX] & SIMPLE_PAYLOAD_TYPE_MASK) == SIMPLE_PAYLOAD_TYPE_COMM)
// #define IS_SIMPLE_PAYLOAD_TYPE_DATA(p)  (((p)->r->buf[SIMPLE_PAYLOAD_TYPE_INDEX] & SIMPLE_PAYLOAD_TYPE_MASK) == SIMPLE_PAYLOAD_TYPE_DATA)

// #define IS_SIMPLE_PAYLOAD_NULL(p)       (((p)->r->buf[SIMPLE_PAYLOAD_CTRL_INDEX] & SIMPLE_PAYLOAD_COMM_MASK) == SIMPLE_PAYLOAD_NULL)
// #define IS_SIMPLE_PAYLOAD_BUSY(p)       (((p)->r->buf[SIMPLE_PAYLOAD_CTRL_INDEX] & SIMPLE_PAYLOAD_COMM_MASK) == SIMPLE_PAYLOAD_BUSY)
// #define IS_SIMPLE_PAYLOAD_ERROR(p)      (((p)->r->buf[SIMPLE_PAYLOAD_CTRL_INDEX] & SIMPLE_PAYLOAD_COMM_MASK) == SIMPLE_PAYLOAD_ERROR)
// #define IS_SIMPLE_PAYLOAD_OK(p)         (((p)->r->buf[SIMPLE_PAYLOAD_CTRL_INDEX] & SIMPLE_PAYLOAD_COMM_MASK) == SIMPLE_PAYLOAD_OK)
// #define IS_SIMPLE_PAYLOAD_STATUS(p)     (((p)->r->buf[SIMPLE_PAYLOAD_CTRL_INDEX] & SIMPLE_PAYLOAD_COMM_MASK) == SIMPLE_PAYLOAD_STATUS)
// #define IS_SIMPLE_PAYLOAD_RESET(p)      (((p)->r->buf[SIMPLE_PAYLOAD_CTRL_INDEX] & SIMPLE_PAYLOAD_COMM_MASK) == SIMPLE_PAYLOAD_RESET)
// // ,,,
// #define IS_SIMPLE_PAYLOAD_ACK(p)        (((p)->r->buf[SIMPLE_PAYLOAD_CTRL_INDEX] & SIMPLE_PAYLOAD_COMM_MASK) == SIMPLE_PAYLOAD_ACK)
// //
// #define SIMPLE_PAYLOAD_RECV_DATA_ADDR(p)     ((p)->r->buf+SIMPLE_PAYLOAD_DATA_INDEX)
// #define SIMPLE_PAYLOAD_RECV_CONF_ADDR(p)     ((p)->r->buf+SIMPLE_PAYLOAD_CONF_INDEX)
// #define SIMPLE_PAYLOAD_RECV_BUF_ADDR(p)      ((p)->r->buf) 
// #define SIMPLE_PAYLOAD_RECV_BUF_SIZE(p)      ((p)->r->size) 


// // send
// #define SET_SIMPLE_PAYLOAD_DATA(p)       ((p)->s->buf[SIMPLE_PAYLOAD_TYPE_INDEX] = SIMPLE_PAYLOAD_TYPE_DATA)
// #define SET_SIMPLE_PAYLOAD_NULL(p)       ((p)->s->buf[SIMPLE_PAYLOAD_CTRL_INDEX] = SIMPLE_PAYLOAD_NULL)
// #define SET_SIMPLE_PAYLOAD_BUSY(p)       ((p)->s->buf[SIMPLE_PAYLOAD_CTRL_INDEX] = SIMPLE_PAYLOAD_BUSY)
// #define SET_SIMPLE_PAYLOAD_ERROR(p)      ((p)->s->buf[SIMPLE_PAYLOAD_CTRL_INDEX] = SIMPLE_PAYLOAD_ERROR)
// #define SET_SIMPLE_PAYLOAD_OK(p)         ((p)->s->buf[SIMPLE_PAYLOAD_CTRL_INDEX] = SIMPLE_PAYLOAD_OK)
// #define SET_SIMPLE_PAYLOAD_STATUS(p)     ((p)->s->buf[SIMPLE_PAYLOAD_CTRL_INDEX] = SIMPLE_PAYLOAD_STATUS)
// #define SET_SIMPLE_PAYLOAD_RESET(p)      ((p)->s->buf[SIMPLE_PAYLOAD_CTRL_INDEX] = SIMPLE_PAYLOAD_RESET)
// // ,,,
// #define SET_SIMPLE_PAYLOAD_ACK(p)        ((p)->s->buf[SIMPLE_PAYLOAD_CTRL_INDEX] = SIMPLE_PAYLOAD_ACK)
// //
// #define SIMPLE_PAYLOAD_SEND_DATA_ADDR(p)     ((p)->s->buf+SIMPLE_PAYLOAD_DATA_INDEX)
// #define SIMPLE_PAYLOAD_SEND_CONF_ADDR(p)     ((p)->s->buf+SIMPLE_PAYLOAD_CONF_INDEX)
// #define SIMPLE_PAYLOAD_SEND_BUF_ADDR(p)      ((p)->s->buf) 
// #define SIMPLE_PAYLOAD_SEND_BUF_SIZE(p)      ((p)->s->size) 


// payload attribution 
#define PAYLOAD_ATTR_SET(p, b)    ((p).attr |= (0x00000001 << b))
#define PAYLOAD_ATTR_RESET(p, b)  ((p).attr &= ~(0x00000001 << b))
#define PAYLOAD_ATTR_CLEAR(p)     ((p).attr = 0x00000000)
#define PAYLOAD_ATTR_GET(p, b)       ((p).attr & (0x00000001 << b))



#endif /*__PAYLOAD_H__*/

