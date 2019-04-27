#ifndef __BILINK_PACKET_H__
#define __BILINK_PACKET_H__

#include <stdint.h>

//#pragma pack(1)

#if 0
#define BILINK_PACKECT_MAX_SIZE 128
#define BILINK_PACKECT_MAX_UNICAST_MSG_SIZE   108
#define BILINK_PACKECT_MAX_BROADCAST_MSG_SIZE 116
#else
#define BILINK_PACKECT_MAX_SIZE                  63
//#define BILINK_PACKECT_MAX_UNICAST_HEAD_SIZE     20
#define BILINK_PACKECT_MAX_UNICAST_MSG_SIZE      43
//#define BILINK_PACKECT_MAX_BROADCAST_HEAD_SIZE   12
#define BILINK_PACKECT_MAX_BROADCAST_MSG_SIZE    51
#define BILINK_PACKECT_BROADCAST_MSG_SIZE        48
#endif

#define BILINK_PACKECT_MAX_UNICAST_HEAD_SIZE     20
#define BILINK_PACKECT_MAX_BROADCAST_HEAD_SIZE   12
#define BILINK_PACKECT_SRC_ADDR_SIZE            8
#define BILINK_PACKECT_CTRL_BYTE_SIZE           1
#define BILINK_PACKECT_DEST_ADDR_SIZE           8
#define BILINK_PACKECT_LENGTH_SIZE              1
#define BILINK_PACKECT_AUTHORIZATION_KEY_SIZE   2
#define BILINK_PACKECT_CTRL_COMM_MASK           0x7F

/*------------------------------------------------------------------------*/
// CTRL BYTE command define
// node to gateway ,BC
// gateway to node
#define BILINK_REGISTER_BEAT_REQ  0x00
#define BILINK_REGISTER_REQ       0x11
#define BILINK_REGISTER_RESP      0x01
#define BILINK_BEAT_REQ           0x02
#define BILINK_ACCESS_REQ         0x13
#define BILINK_ACCESS_RESP        0x03
#define BILINK_CONTROL_REQ        0x14
#define BILINK_DATA_REQ           0x15
#define BILINK_REISSUE_REQ        0x16
#define BILINK_QUERY_REQ          0x17
#define BILINK_QUERY_RESP         0x07


#define BILINK_DATA_VERSION_LEN   8
#define BILINK_TIME_LEN           4
#define BILINK_SEQUEUE_NUM_LEN    2
#define BILINK_POWER_LEN          1
#define BILINK_KEY_LEN            2

// SUb conmand
// #define BILINK_REGISTER_BEAT_REQ  0x00
// no sub command
// #define BILINK_REGISTER_REQ       0x11
#define BILINK_REGISTER_REQ_SUB_KEY_NEWKEY             0x01
#define BILINK_REGISTER_REQ_SUB_LEN_NEWKEY             BILINK_KEY_LEN
// #define BILINK_REGISTER_RESP      0x01
// no sub command
// #define BILINK_BEAT_REQ           0x02
#define BILINK_BEAT_REQ_SUB_KEY_POWER                  0x02
#define BILINK_BEAT_REQ_SUB_LEN_POWER                  BILINK_POWER_LEN
#define BILINK_BEAT_REQ_SUB_KEY_VERSION                0x03
#define BILINK_BEAT_REQ_SUB_LEN_VERSION                BILINK_DATA_VERSION_LEN
// #define BILINK_ACCESS_REQ         0x13
#define BILINK_ACCESS_REQ_SUB_KEY_FLUSH_TIME           0x04
#define BILINK_ACCESS_REQ_SUB_LEN_FLUSH_TIME           BILINK_TIME_LEN
#define BILINK_ACCESS_REQ_SUB_KEY_UTC_TIME             0x05
#define BILINK_ACCESS_REQ_SUB_LEN_UTC_TIME             BILINK_TIME_LEN
// #define BILINK_ACCESS_RESP        0x03
//#define BILINK_ACCESS_RESP_SUB_KEY_POWER               0x01
//#define BILINK_ACCESS_RESP_SUB_LEN_POWER               0x01
#define BILINK_ACCESS_RESP_SUB_KEY_LAST_FLUSH_TIME     0x06
#define BILINK_ACCESS_RESP_SUB_LEN_LAST_FLUSH_TIME     BILINK_TIME_LEN
//#define BILINK_ACCESS_RESP_SUB_KEY_VERSION             0x03
//#define BILINK_ACCESS_RESP_SUB_LEN_VERSION             0x08
// #define BILINK_CONTROL_REQ        0x14
#define BILINK_CONTROL_REQ_SUB_KEY_VERSION             0x08
#define BILINK_CONTROL_REQ_SUB_LEN_VERSION             BILINK_DATA_VERSION_LEN
#define BILINK_CONTROL_REQ_SUB_KEY_PKT_NUM             0x09
#define BILINK_CONTROL_REQ_SUB_LEN_PKT_NUM             BILINK_SEQUEUE_NUM_LEN
#define BILINK_CONTROL_REQ_SUB_KEY_NEXT_FLUSH_TIME     0x07
#define BILINK_CONTROL_REQ_SUB_LEN_NEXT_FLUSH_TIME     BILINK_TIME_LEN
// #define BILINK_DATA_REQ           0x15
// SEQUEUE + DATA
// #define BILINK_REISSUE_REQ        0x16
// SEQUEUE + DATA
// #define BILINK_QUERY_REQ          0x17
#define BILINK_QUERY_REQ_SUB_KEY_REISSUE_TIME          0x0A
#define BILINK_QUERY_REQ_SUB_LEN_REISSUE_TIME          BILINK_TIME_LEN
// #define BILINK_QUERY_RESP         0x07
#define BILINK_QUERY_RESP_SUB_KEY_LOST_PKT_NUM         0x0B
#define BILINK_QUERY_RESP_SUB_LEN_LOST_PKT_NUM         BILINK_SEQUEUE_NUM_LEN
#define BILINK_QUERY_RESP_SUB_KEY_LOST_PKT_SEQ         0x0C
#define BILINK_QUERY_RESP_SUB_LEN_LOST_PKT_SEQ         BILINK_SEQUEUE_NUM_LEN


#if 0
// node to gateway
#define BILINK_COMM_NETWORK_ACCESS_REQ 0x51
	#define BILINK_COMM_NETWORK_ACCESS_SUB_POWER 0x02
	#define BILINK_COMM_NETWORK_ACCESS_SUB_DATA_VERSION 0x03
	#define BILINK_COMM_NETWORK_ACCESS_SUB_LAST_UPDATE_TIME 0x04
	//
	#define BILINK_COMM_NETWORK_ACCESS_SUB_POWER_LEN 0x01
	#define BILINK_COMM_NETWORK_ACCESS_SUB_DATA_VERSION_LEN 0x02
	#define BILINK_COMM_NETWORK_ACCESS_SUB_LAST_UPDATE_TIME_LEN 0x04
// gateway to node
//#define BILINK_COMM_NETWORK_ACCESS_RESP 0x61
#define BILINK_COMM_APPOINT_UPDATE_SCHEDULE_REQ 0x61
	#define BILINK_COMM_APPOINT_UPDATE_SCHEDULE_SUB_KEEP_BEAT 0x02
	#define BILINK_COMM_APPOINT_UPDATE_SCHEDULE_SUB_DATA_INFO 0x03
	#define BILINK_COMM_APPOINT_UPDATE_SCHEDULE_SUB_NEXT_UPDATE_TIME 0x04
	//
	#define BILINK_COMM_APPOINT_UPDATE_SCHEDULE_SUB_KEEP_BEAT_LEN 0x00
	#define BILINK_COMM_APPOINT_UPDATE_SCHEDULE_SUB_DATA_INFO_LEN 0x06
	#define BILINK_COMM_APPOINT_UPDATE_SCHEDULE_SUB_NEXT_UPDATE_TIME_LEN 0x04
// node to gateway
#define BILINK_COMM_APPOINT_UPDATE_SCHEDULE_RESP 0x52
	#define BILINK_COMM_APPOINT_UPDATE_SCHEDULE_SUB_WHETHER_TO_UPDATE 0x02
	//
	#define BILINK_COMM_APPOINT_UPDATE_SCHEDULE_SUB_WHETHER_TO_UPDATE_LEN 0x01

// gateway to node ,BC
#define BILINK_COMM_BROADCAST_DATA_BLOCK_REQ 0x62
//
	#define BILINK_COMM_BROADCAST_DATA_BLOCK_SUB_DATA_INFO 0x01
	#define BILINK_COMM_BROADCAST_DATA_BLOCK_SUB_DATA_PKT_SEQ 0x02
	//
	#define BILINK_COMM_BROADCAST_DATA_BLOCK_SUB_DATA_INFO_LEN 0x02
	#define BILINK_COMM_BROADCAST_DATA_BLOCK_SUB_DATA_PKT_SEQ_LEN 0x02
//----------------------------------------------------------------------//
	#define BILINK_COMM_BROADCAST_DATA_BLOCK_REQ_SEQ_LEN       2
	#define BILINK_COMM_BROADCAST_DATA_BLOCK_REQ_MAX_DATA_LEN  48


// gateway to node ,BC
#define BILINK_COMM_UPDATE_RESULT_REQ 0x63
	#define BILINK_COMM_UPDATE_RESULT_SUB_AUTO_RESP 0x02
	//
	#define BILINK_COMM_UPDATE_RESULT_SUB_AUTO_RESP_LEN 0x00

// node to gateway
#define BILINK_COMM_UPDATE_RESULT_RESP 0x53
	#define BILINK_COMM_UPDATE_RESULT_SUB_TOTAL_LOST_NUM 0x02
	#define BILINK_COMM_UPDATE_RESULT_SUB_LOST_PKT_ELEMT 0x03
	//
	#define BILINK_COMM_UPDATE_RESULT_SUB_TOTAL_LOST_NUM_LEN 0x02
	#define BILINK_COMM_UPDATE_RESULT_SUB_LOST_PKT_ELEMT_LEN 0x02

// gateway to node ,BC
#define BILINK_COMM_REISSUE_DATA_BLOCK_REQ 0x64
	#define BILINK_COMM_REISSUE_DATA_BLOCK_SUB_DATA_INFO 0x02
	#define BILINK_COMM_REISSUE_DATA_BLOCK_SUB_PKT_SEQ 0x03
	#define BILINK_COMM_REISSUE_DATA_BLOCK_SUB_PKT_DATA 0x04
	//
	#define BILINK_COMM_REISSUE_DATA_BLOCK_SUB_DATA_INFO_LEN 0x02
	#define BILINK_COMM_REISSUE_DATA_BLOCK_SUB_PKT_SEQ_LEN 0x02
	#define BILINK_COMM_REISSUE_DATA_BLOCK_SUB_PKT_DATA_LEN

// node to gateway
#define BILINK_COMM_REISSUE_DATA_BLOCK_RESP 0x54


// gateway to node ,BC, (the command can be delete)
#define BILINK_COMM_APPOINT_REISSUE_UPDATE_SCHEDULE_REQ 0x65
	#define BILINK_COMM_APPOINT_REISSUE_UPDATE_SCHEDULE_SUB_NEXT_TIME 0x02
	//
	#define BILINK_COMM_APPOINT_REISSUE_UPDATE_SCHEDULE_SUB_NEXT_TIME_LEN 0x04

// node to gateway
#define BILINK_COMM_REGISTER_REQ 0x55
// gateway to node
#define BILINK_COMM_UPDATE_ACCESS_KEY_REQ 0x66
	#define BILINK_COMM_UPDATE_ACCESS_KEY_SUB_NEW_KEY 0x01
	#define BILINK_COMM_UPDATE_ACCESS_KEY_SUB_NEW_KEY_LEN 0x02
// node to gateway
#define BILINK_COMM_UPDATE_ACCESS_KEY_RESP 0x56
#endif





#if 0
// gcc uint64_t store in memery: high first (big head)
//
union bilink_packet {
	uint8_t buf[BILINK_PACKECT_MAX_SIZE];
	// unicast
	// source-address  control  direction-address  length  messages  check
	// 8               1        8                   1      n         2
	//
	struct {
		union {
			uint8_t buf[BILINK_PACKECT_SRC_ADDR_SIZE];
			uint64_t val;
		} srcaddr;

		union {
			uint8_t buf[BILINK_PACKECT_DEST_ADDR_SIZE];
			uint64_t val;
		} destaddr;

		union {
			uint8_t buf[BILINK_PACKECT_CTRL_BYTE_SIZE];
			uint8_t val;
		} ctrl;

		union {
			uint8_t buf[BILINK_PACKECT_LENGTH_SIZE];
			uint8_t val;
		} length;

		union {
			uint8_t buf[BILINK_PACKECT_MAX_UNICAST_MSG_SIZE];
		} msg;

	} uc;
	// broadcast
	// source-address  control  length  messages  check
	// 8               1        1       n         2
	//
	struct {
		union {
			uint8_t buf[BILINK_PACKECT_SRC_ADDR_SIZE];
			uint64_t val;
		} srcaddr;

		union {
			uint8_t buf[BILINK_PACKECT_CTRL_BYTE_SIZE];
			uint8_t val;
		} ctrl;

		union {
			uint8_t buf[BILINK_PACKECT_LENGTH_SIZE];
			uint8_t val;
		} length;

		union {
			uint8_t buf[BILINK_PACKECT_MAX_BROADCAST_MSG_SIZE];
		} msg;

	} bc;
};

#elif 0

union bilink_packet {
	uint8_t buf[BILIbilink_conn_objNK_PACKECT_MAX_SIZE];

	// unicast
	// source-address  control  direction-address  length  messages  check
	// 8               1        8                   1      n         2
	//
	struct {
		uint8_t srcaddr[BILINK_PACKECT_SRC_ADDR_SIZE];
		union {
			uint8_t buf[BILINK_PACKECT_CTRL_BYTE_SIZE];
			uint8_t val;
			struct {
				uint8_t comm:7;  // bit 0~6
				uint8_t broad:1;  // bit 7
			};
		} ctrl;
		uint8_t destaddr[BILINK_PACKECT_DEST_ADDR_SIZE];
		uint8_t length;
		uint8_t msg[BILINK_PACKECT_MAX_UNICAST_MSG_SIZE];
	} uc;
	// broadcast
	// source-address  control  length  messages  check
	// 8               1        1       n         2
	//
	struct {
		uint8_t srcaddr[BILINK_PACKECT_SRC_ADDR_SIZE];
		union {
			uint8_t buf[BILINK_PACKECT_CTRL_BYTE_SIZE];
			uint8_t val;
			struct {
				uint8_t comm:7;
				uint8_t broad:1;
			};
		} ctrl;
		uint8_t length;
		uint8_t msg[BILINK_PACKECT_MAX_BROADCAST_MSG_SIZE];
	} bc;

};
bilink_conn_obj

#else

union bilink_packet {
	uint8_t buf[BILINK_PACKECT_MAX_SIZE];

	struct {
		uint8_t srcaddr[BILINK_PACKECT_SRC_ADDR_SIZE];
		union {
			uint8_t buf[BILINK_PACKECT_CTRL_BYTE_SIZE];
			uint8_t val;
			struct {
				uint8_t comm:7;  // bit 0~6
				uint8_t broad:1;  // bit 7
			};
		} ctrl;
		union {
			// unicast
			// source-address  control  direction-address  length  messages  check
			// 8               1        8                   1      n         2
			//
			struct {
				uint8_t destaddr[BILINK_PACKECT_DEST_ADDR_SIZE];
				union {
					struct {
						uint8_t length;
						uint8_t msg[BILINK_PACKECT_MAX_UNICAST_MSG_SIZE];
					};
					uint8_t akey[BILINK_PACKECT_AUTHORIZATION_KEY_SIZE];
				};
			} uc;
			// broadcast
			// source-address  control  length  messages  check
			// 8               1        1       n         2
			//
			union {
				struct {
					uint8_t length;
					uint8_t msg[BILINK_PACKECT_MAX_BROADCAST_MSG_SIZE];
				};
				uint8_t akey[BILINK_PACKECT_AUTHORIZATION_KEY_SIZE];
			} bc;
		};

	};


};

#endif

enum {
	WITHOUT_CONTENT = 0x00,
	WITH_CONTENT = 0x01,
};

uint8_t * str_to_hex (uint8_t * dest, char * src, int src_len);

char * hex_to_str (char * dest, uint8_t * src, int src_len);

int uint32_to_array (uint32_t from, uint8_t * to);

uint32_t array_to_uint32 (uint8_t * from);

uint64_t array_to_uint64 (uint8_t * from);

int uint64_to_array (uint64_t from, uint8_t * to);

char * addr_to_string (char * to, uint8_t * from);

uint8_t * string_to_addr (uint8_t * to, char * from);

char * data_id_to_string (char * to, uint8_t * from);

uint8_t * string_to_data_id (uint8_t * to, char * from);

//int create_bilink_unicast_packet (uint8_t * buf, uint8_t * srcaddr, uint8_t * destaddr, uint8_t * key, uint8_t ctrl, ...);

//int create_bilink_broadcast_packet (uint8_t * buf, uint8_t * srcaddr, uint8_t * key, uint8_t ctrl, ...);

int authorization_key (uint8_t * word, uint8_t * src, uint8_t * key);

int create_broadcast_packet (uint8_t * buf, uint8_t * srcaddr, uint8_t ctrl, uint8_t config);

int create_unicast_packet (uint8_t * buf, uint8_t * srcaddr, uint8_t ctrl, uint8_t * destaddr, uint8_t config);

int packet_add_content (uint8_t * buf, int index, uint8_t type, uint8_t len, uint8_t * val);

int packet_add_data (uint8_t * buf, int index, int seq, int len, uint8_t * data);

int packet_add_autherization_key (uint8_t * buf, int index, uint8_t * key);

int packet_add_key (uint8_t * buf, int index, uint8_t * key);

#endif
