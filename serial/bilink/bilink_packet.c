

#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "../bilink/bilink_packet.h"
#include "../crc/crc.h"
#include "../config.h"
/*--------------------------------------------------------------------------------*/
#define BILINK_TEST 0

#define LITTLE_ENDIAN 0
#define SIZE_OF_INT64 8
#define SIZE_OF_INT32 4

/*--------------------------------------------------------------------------------*/
char hex_to_ascii (char hex) {
	hex &= 0x0F;
	// 'A'=0x41=65  '0'=0x30=48
	return (hex > 9 ? hex+55 : hex+48);
}
/*--------------------------------------------------------------------------------*/
char ascii_to_hex (char ascii) {
	// 'a'=97
	if (ascii > 96) {
		return (ascii - 87);
	}
	// 'A'=65
	if (ascii > 64) {
		return (ascii - 55);
	}
	// 'A'=0x41=65  '0'=0x30=48
	return (ascii - 48);
}
/*--------------------------------------------------------------------------------*/
char * hex_to_str (char * dest, uint8_t * src, int src_len) {
	int i = 0;
	for (i=0; i<src_len; i++) {
		dest[(i<<1)] = hex_to_ascii(src[i]>>4);
		dest[(i<<1)+1] = hex_to_ascii(src[i]);
	}
	dest[(i<<1)] = '\0';
	return dest;
}

/*--------------------------------------------------------------------------------*/
uint8_t * str_to_hex (uint8_t * dest, char * src, int src_len) {
	int i = 0;
	for (i=0; i<src_len; i+=2) {
		dest[(i>>1)] = ascii_to_hex(src[i]);
		dest[(i>>1)] <<= 4;
		dest[(i>>1)] |= ascii_to_hex(src[i+1]);
	}
	return dest;
}
/*--------------------------------------------------------------------------------*/
uint32_t array_to_uint32 (uint8_t * from) {
	uint32_t ret = 0;
	int num = 0;
	if (from == NULL) {
		return ret;
	}
	for (num=0; num<SIZE_OF_INT32; num++){
#if LITTLE_ENDIAN
		// Low bits first (Little-Endian)
		ret += (uint32_t)from[SIZE_OF_INT32-num-1];
		ret <<= 8;

#else
		// High bits first (Big-Endian)
		ret += (uint32_t)from[num];
		ret <<= 8;
#endif
	}
	return ret;
}
/*--------------------------------------------------------------------------------*/
int uint32_to_array (uint32_t from, uint8_t * to) {
	int num = 0;
	if (to == NULL) {
		return -1;
	}
	for (num=0; num<SIZE_OF_INT32; num++){
#if LITTLE_ENDIAN
		// Low bits first (Little-Endian)
		to[num] = (uint8_t)((from >> num*8) & 0x000000FF);
#else
		// High bits first (Big-Endian)
		to[SIZE_OF_INT32-num-1] = (uint8_t)((from >> num*8) & 0x000000FF);
#endif
	}
	return 0;
}

uint64_t array_to_uint64 (uint8_t * from) {
	uint64_t ret = 0;
	int num = 0;
	if (from == NULL) {
		return ret;
	}
	for (num=0; num<SIZE_OF_INT64; num++){
#if LITTLE_ENDIAN
		// Low bits first (Little-Endian)
		ret += (uint64_t)from[SIZE_OF_INT64-num-1];
		ret <<= 8;

#else
		// High bits first (Big-Endian)
		ret += (uint64_t)from[num];
		ret <<= 8;
#endif
	}
	return ret;
}

/*--------------------------------------------------------------------------------*/
int uint64_to_array (uint64_t from, uint8_t * to) {
	int num = 0;
	if (to == NULL) {
		return -1;
	}
	for (num=0; num<SIZE_OF_INT64; num++){
#if LITTLE_ENDIAN
		// Low bits first (Little-Endian)
		to[num] = (uint8_t)((from >> num*8) & 0x00000000000000FF);
#else
		// High bits first (Big-Endian)
		to[SIZE_OF_INT64-num-1] = (uint8_t)((from >> num*8) & 0x00000000000000FF);
#endif
	}
	return 0;
}
/*--------------------------------------------------------------------------------*/
char * addr_to_string (char * to, uint8_t * from) {
	uint64_t ret = -1;
#if LITTLE_ENDIAN
		// Low bits first (Little-Endian)
		ret = snprintf(to,  (SIZE_OF_INT64<<1)+1, "%2X%2X%2X%2X%2X%2X%2X%2X",
				*(from+7), *(from+6), *(from+5), *(from+4), *(from+3), *(from+2), *(from+1), *(from));

#else
		// High bits first (Big-Endian)
		ret = snprintf(to,  (SIZE_OF_INT64<<1)+1, "%2X%2X%2X%2X%2X%2X%2X%2X",
				*(from), *(from+1), *(from+2), *(from+3), *(from+4), *(from+5), *(from+6), *(from+7));
#endif

	return (ret > 0 ? to : NULL);
}
/*--------------------------------------------------------------------------------*/
uint8_t * string_to_addr (uint8_t * to, char * from) {
	uint64_t ret = -1;
	uint32_t recv[8];
#if LITTLE_ENDIAN
		// Low bits first (Little-Endian)
	ret = sscanf(from, "%2X%2X%2X%2X%2X%2X%2X%2X", (recv+7), (recv+6), (recv+5), (recv+4), (recv+3), (recv+2), (recv+1), (recv));

#else
		// High bits first (Big-Endian)
	sscanf(from, "%2X%2X%2X%2X%2X%2X%2X%2X", (recv), (recv+1), (recv+2), (recv+3), (recv+4), (recv+5), (recv+6), (recv+7));
#endif
	to[0] = (uint8_t)recv[0];
	to[1] = (uint8_t)recv[1];
	to[2] = (uint8_t)recv[2];
	to[3] = (uint8_t)recv[3];
	to[4] = (uint8_t)recv[4];
	to[5] = (uint8_t)recv[5];
	to[6] = (uint8_t)recv[6];
	to[7] = (uint8_t)recv[7];
	return (ret > 0 ? to : NULL);
}
/*--------------------------------------------------------------------------------*/
char * data_id_to_string (char * to, uint8_t * from) {
	uint64_t ret = -1;
#if LITTLE_ENDIAN
		// Low bits first (Little-Endian)
		ret = snprintf(to,  5, "%2X%2X", *(from+1), *(from));

#else
		// High bits first (Big-Endian)
		ret = snprintf(to,  5, "%2X%2X", *(from), *(from+1));
#endif

	return (ret > 0 ? to : NULL);
}
/*--------------------------------------------------------------------------------*/
uint8_t * string_to_data_id (uint8_t * to, char * from) {
	uint64_t ret = -1;
	uint32_t recv[2];
#if LITTLE_ENDIAN
		// Low bits first (Little-Endian)
	ret = sscanf(from, "%2X%2X", (recv+1), (recv));

#else
		// High bits first (Big-Endian)
	ret = sscanf(from, "%2X%2X", (recv), (recv+1));
#endif
	to[0] = (uint8_t)recv[0];
	to[1] = (uint8_t)recv[1];
	return (ret > 0 ? to : NULL);
}

/*--------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------*/
/* CITT CRC16 polynomial ^16 + ^12 + ^5 + 1 */
/*---------------------------------------------------------------------------*/
uint16_t crc16_add(uint8_t b, uint16_t acc)
{
  acc ^= b;
  acc  = (acc >> 8) | (acc << 8);
  acc ^= (acc & 0xff00) << 4;
  acc ^= (acc >> 8) >> 4;
  acc ^= (acc & 0xff00) >> 5;
  return acc;
}
/*---------------------------------------------------------------------------*/
uint16_t crc16_data(const uint8_t *data, int len, uint16_t acc)
{
  int i;

  for(i = 0; i < len; ++i) {
    acc = crc16_add(*data, acc);
    ++data;
  }
  return acc;
}
/*---------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------*/
int authorization_key (uint8_t * word, uint8_t * src, uint8_t * key) {
	uint8_t  crc_src[10];
	uint16_t crc;
	memcpy(crc_src, src, 8);
	memcpy(crc_src+8, key, 2);
	crc = CRC16Calc(crc_src, 10);

#if LITTLE_ENDIAN
	// Low bits first (Little-Endian)
	word[0] = (uint8_t)(crc & 0x00FF);
	word[1] = (uint8_t)((crc >> 8) & 0x00FF);
#else
	// High bits first (Big-Endian)
	word[1] = (uint8_t)(crc & 0x00FF);
	word[0] = (uint8_t)((crc >> 8) & 0x00FF);
#endif
	return 0;
}
/*--------------------------------------------------------------------------------*/
int create_broadcast_packet (uint8_t * buf, uint8_t * srcaddr, uint8_t ctrl, uint8_t config) {
	int ret = 0;
	union bilink_packet * bpkt = (union bilink_packet *)buf;
	memcpy(bpkt->srcaddr, srcaddr, BILINK_PACKECT_SRC_ADDR_SIZE);
	bpkt->ctrl.broad = 1;
	bpkt->ctrl.comm = ctrl & BILINK_PACKECT_CTRL_COMM_MASK;
	ret = 9;
	if (config == WITH_CONTENT) {
		bpkt->bc.length = 0;
		ret++;
	}
	return ret;
}
/*--------------------------------------------------------------------------------*/
int create_unicast_packet (uint8_t * buf, uint8_t * srcaddr, uint8_t ctrl, uint8_t * destaddr, uint8_t config) {
	int ret = 0;
	union bilink_packet * bpkt = (union bilink_packet *)buf;
	memcpy(bpkt->srcaddr, srcaddr, BILINK_PACKECT_SRC_ADDR_SIZE);
	bpkt->ctrl.broad = 0;
	bpkt->ctrl.comm = ctrl & BILINK_PACKECT_CTRL_COMM_MASK;
	memcpy(bpkt->uc.destaddr, destaddr, BILINK_PACKECT_DEST_ADDR_SIZE);
	ret = 17;
	if (config == WITH_CONTENT) {
		bpkt->uc.length = 0;
		ret++;
	}
	return ret;
}
/*--------------------------------------------------------------------------------*/
int packet_add_content (uint8_t * buf, int index, uint8_t type, uint8_t len, uint8_t * val) {
	union bilink_packet * bpkt = (union bilink_packet *)buf;
	buf[index++] = type;
	buf[index++] = len;
	if(bpkt->ctrl.broad) {
		bpkt->bc.length += 2;
	} else {
		bpkt->uc.length += 2;
	}

	if (len != 0 && val != NULL) {
		memcpy(buf+index, val, len);
		index += len;
		if(bpkt->ctrl.broad) {
			bpkt->bc.length += len;
		} else {
			bpkt->uc.length += len;
		}
	}
	return index;
}

int packet_add_data (uint8_t * buf, int index, int seq, int len, uint8_t * data) {
	union bilink_packet * bpkt = (union bilink_packet *)buf;
	buf[index++] = (uint8_t)((seq>>8) & 0x000000FF);
	buf[index++] = (uint8_t)(seq & 0x000000FF);
//	log_debug("len : %d", len);
	if(bpkt->ctrl.broad) {
		bpkt->bc.length += 2;
//		log_debug("length : %d", bpkt->bc.length);
	} else {
		bpkt->uc.length += 2;
//		log_debug("length : %d", bpkt->bc.length);
	}

	if (len != 0 && data != NULL) {
		memcpy(buf+index, data, len);
		index += len;
		if(bpkt->ctrl.broad) {
			bpkt->bc.length += (uint8_t)len;
//			log_debug("length : %d", bpkt->bc.length);
		} else {
			bpkt->uc.length += (uint8_t)len;
//			log_debug("length : %d", bpkt->bc.length);
		}
	}
	return index;
}
/*--------------------------------------------------------------------------------*/
int packet_add_key (uint8_t * buf, int index, uint8_t * key) {
//	union bilink_packet * bpkt = (union bilink_packet *)buf;
	memcpy(buf+index, key, BILINK_PACKECT_AUTHORIZATION_KEY_SIZE);
	index += 2;
//	if(bpkt->ctrl.broad) {
//		bpkt->bc.length += 2;
//	} else {
//		bpkt->uc.length += 2;
//	}
	return index;
}

/*--------------------------------------------------------------------------------*/
int packet_add_autherization_key (uint8_t * buf, int index, uint8_t * key) {
//	union bilink_packet * bpkt = (union bilink_packet *)buf;
	authorization_key(buf+index, ((union bilink_packet *)buf)->srcaddr, key);
	index += 2;
//	if(bpkt->ctrl.broad) {
//		bpkt->bc.length += 2;
//	} else {
//		bpkt->uc.length += 2;
//	}
	return index;
}

