/*
 * payload.h
 *
 *  Created on: Mar 6, 2019
 *      Author: user
 */
#ifndef __SIMPLE_PAYLOAD_H__
#define __SIMPLE_PAYLOAD_H__

#include <stdio.h>
#include <stdint.h>

#include "serialp/serial.h"

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
//
#define SIMPLE_PAYLOAD_SEND_MODE            0x06
#define SIMPLE_PAYLOAD_RECV_MODE            0x07
#define SIMPLE_PAYLOAD_RESE_MODE            0x08
#define SIMPLE_PAYLOAD_RESE_SUSPEND         0x09
// ...
#define SIMPLE_PAYLOAD_ACK                  0x7F


enum {
	PAYLOAD_ATTR_OPENLOOP_SENDING = 0x00,
	PAYLOAD_ATTR_RELIABLE_SENDING,

	PAYLOAD_ATTR_OPENLOOP_RECVING,
	PAYLOAD_ATTR_RELIABLE_RECVING,

	PAYLOAD_ATTR_RETURN_BUSY,
	PAYLOAD_ATTR_WAITING_ACK,

	PAYLOAD_ATTR_WAITING_CMD,
	// PAYLOAD_ATTR_WAITING_STATUS,
	// PAYLOAD_ATTR_WAITING_RESET,
	// PAYLOAD_ATTR_WAITING_SEND,
	// PAYLOAD_ATTR_WAITING_RECV,
	// PAYLOAD_ATTR_WAITING_RESE,
	// PAYLOAD_ATTR_WAITING_CANCEL,
	PAYLOAD_ATTR_MAX,
};



typedef uint32_t simple_payload_attr_t;

struct simple_payload_buf {
	int size;
	union {
		uint8_t buf[SIMPLE_PAYLOAD_LENGTH_MAX];
		struct {
			uint8_t ctrl;
			uint8_t data[SIMPLE_PAYLOAD_LENGTH_MAX-1];
		} payload;
	};
};

struct simple_payload {
	struct serial_handler serial;
	struct simple_payload_buf rbuf;
	simple_payload_attr_t attr;
};

typedef struct simple_payload simple_payload_t;
typedef struct simple_payload_buf simple_payload_buf_t;

// // byte
#define IS_TYPE_BYTE_COMM(b)         (((b) & SIMPLE_PAYLOAD_TYPE_MASK) == SIMPLE_PAYLOAD_TYPE_COMM)
#define COMM_BYTE_MASK(b)            ((b) & SIMPLE_PAYLOAD_COMM_MASK)

// payload attribution
#define PAYLOAD_ATTR_SET(p, b)       ((p).attr |= (0x00000001 << b))
#define PAYLOAD_ATTR_RESET(p, b)     ((p).attr &= ~(0x00000001 << b))
#define PAYLOAD_ATTR_CLEAR(p)        ((p).attr = 0x00000000)
#define PAYLOAD_ATTR_GET(p, b)       ((p).attr & (0x00000001 << b))


#endif /*__SIMPLE_PAYLOAD_H__*/



