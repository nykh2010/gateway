#ifndef __SIMPLE_FRAME_H__
#define __SIMPLE_FRAME_H__

/* non-master non-feedback simple serial asynchronous protocol 
 * 
 * 
 */

// simple serial async frame head, 2 bytes
#define SIMPLE_SERIAL_ASYNC_FRAME_HEAD_INDEX           0x00
#define SIMPLE_SERIAL_ASYNC_FRAME_HEAD_BYTE_0_INDEX    0x00
#define SIMPLE_SERIAL_ASYNC_FRAME_HEAD_BYTE_1_INDEX    0x01
#define SIMPLE_SERIAL_ASYNC_FRAME_HEAD_SIZE            0x02
#define SIMPLE_SERIAL_ASYNC_FRAME_HEAD_BYTE_0          0x55
#define SIMPLE_SERIAL_ASYNC_FRAME_HEAD_BYTE_1          0xAA

// simple serial async frame length, 1 bytes
// the following data size (include check byte)
#define SIMPLE_SERIAL_ASYNC_FRAME_LENGTH_INDEX         0x02
#define SIMPLE_SERIAL_ASYNC_FRAME_LENGTH_SIZE          0x01

// simple serial async frame payload bytes, n bytes
#define SIMPLE_SERIAL_ASYNC_FRAME_PAYLOAD_INDEX        0x03

// simple serial async frame xor check byte, 1 bytes
#define SIMPLE_SERIAL_ASYNC_FRAME_CHECK_SIZE           0x01

// simple serial async frame control byte, 1 bytes
#define SIMPLE_SERIAL_ASYNC_FRAME_PAYLOAD_CTRL_INDEX           0x03
#define SIMPLE_SERIAL_ASYNC_FRAME_PAYLOAD_CTRL_SIZE            0x01
#define SIMPLE_SERIAL_ASYNC_FRAME_PAYLOAD_CTRL_IDLE            0x00
#define SIMPLE_SERIAL_ASYNC_FRAME_PAYLOAD_CTRL_MASK            0x80
#define SIMPLE_SERIAL_ASYNC_FRAME_PAYLOAD_CTRL_DATA            0x00
#define SIMPLE_SERIAL_ASYNC_FRAME_PAYLOAD_CTRL_COMM            0x80
#define SIMPLE_SERIAL_ASYNC_FRAME_PAYLOAD_CTRL_DATA_MASK       0x7F
#define SIMPLE_SERIAL_ASYNC_FRAME_PAYLOAD_CTRL_COMM_MASK       0x7F
//#define SIMPLE_SERIAL_ASYNC_FRAME_PAYLOAD_CTRL_SET             0x03
//#define SIMPLE_SERIAL_ASYNC_FRAME_PAYLOAD_CTRL_GET             0x04
// ...
#define SIMPLE_SERIAL_ASYNC_FRAME_PAYLOAD_CTRL_ACK             0x81

// simple serial async frame payload bytes, n bytes
#define SIMPLE_SERIAL_ASYNC_FRAME_PAYLOAD_DATA_INDEX           0x04





#endif /*__SIMPLE_FRAME_H__*/
