#ifndef __BILINK_PACKET_H__
#define __BILINK_PACKET_H__

#include <stdint.h>

#pragma pack(1)

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

#define BILINK_PACKECT_CTRL_COMM_MASK  0x7F

/*------------------------------------------------------------------------*/
// CTRL BYTE command define
// node to gateway ,BC
#define BILINK_COMM_BEAT_REQ 0x50
// gateway to node
#define BILINK_COMM_BEAT_RESP 0x60

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
	//
	#define BILINK_COMM_UPDATE_ACCESS_KEY_SUB_NEW_KEY_LEN 0x02

// node to gateway
#define BILINK_COMM_UPDATE_ACCESS_KEY_RESP 0x56



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


uint64_t addr_to_uint64 (uint8_t * from);

int uint64_to_addr (uint64_t from, uint8_t * to);

char * addr_to_string (char * to, uint8_t * from);

uint8_t * string_to_addr (uint8_t * to, char * from);

char * data_id_to_string (char * to, uint8_t * from);

uint8_t * string_to_data_id (uint8_t * to, char * from);

int create_bilink_unicast_packet (uint8_t * buf, uint8_t * srcaddr, uint8_t * destaddr, uint8_t * key, uint8_t ctrl, ...);

int create_bilink_broadcast_packet (uint8_t * buf, uint8_t * srcaddr, uint8_t * key, uint8_t ctrl, ...);

int authorization_key (uint8_t * word, uint8_t * src, uint8_t * key);

int create_broadcast_packet (uint8_t * buf, uint8_t * srcaddr, uint8_t ctrl, uint8_t config);

int create_unicast_packet (uint8_t * buf, uint8_t * srcaddr, uint8_t ctrl, uint8_t * destaddr, uint8_t config);

int packet_add_content (uint8_t * buf, int index, uint8_t type, uint8_t len, uint8_t * val);

int packet_add_autherization_key (uint8_t * buf, int index, uint8_t * key);

#endif
