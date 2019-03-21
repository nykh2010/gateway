#include "../bilink/bilink_packet.h"

#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
/*--------------------------------------------------------------------------------*/
#define BILINK_TEST 0

#define LITTLE_ENDIAN 1
#define SIZE_OF_INT64 8
#define SIZE_OF_INT32 4
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

uint64_t addr_to_uint64 (uint8_t * from) {
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
int uint64_to_addr (uint64_t from, uint8_t * to) {
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
int func_1 (uint8_t * msg, uint8_t size) {

	return 0;
}
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
	uint16_t crc_key;
	uint16_t crc;
#if LITTLE_ENDIAN
	// Low bits first (Little-Endian)
	crc_key = (uint16_t)(key[1]);
	crc_key <<= 8;
	crc_key += key[0];
#else
	// High bits first (Big-Endian)
	crc_key = (uint16_t)(key[0]);
	crc_key <<= 8;
	crc_key += key[1];
#endif

	crc = crc16_data(src, BILINK_PACKECT_SRC_ADDR_SIZE, crc_key);

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
	bpkt->ctrl.broad = 1;
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
	buf[index++] = type;
	buf[index++] = len;
	if (len != 0 && val != NULL) {
		memcpy(buf+index, val, len);
		index += len;
	}
	return index;
}

/*--------------------------------------------------------------------------------*/
int packet_add_key (uint8_t * buf, int index, uint8_t * key) {
	memcpy(buf+index, key, BILINK_PACKECT_AUTHORIZATION_KEY_SIZE);
	index += 2;
	return index;
}

/*--------------------------------------------------------------------------------*/
int packet_add_autherization_key (uint8_t * buf, int index, uint8_t * key) {
	authorization_key(buf+index, ((union bilink_packet *)buf)->srcaddr, key);
	index += 2;
	return index;
}

/*--------------------------------------------------------------------------------*/
int create_bilink_broadcast_packet (uint8_t * buf, uint8_t * srcaddr, uint8_t * key, uint8_t ctrl, ...) {
	va_list ap;
	union bilink_packet * bpkt = (union bilink_packet *)buf;
	uint8_t tol_len = 0;
//	uint8_t tol_len = 0;
	int len = 0;
	int type = 0;
	long valaddr;
	char * val = 0;

	bpkt->bc.length = 0;
	memcpy(bpkt->srcaddr, srcaddr, BILINK_PACKECT_SRC_ADDR_SIZE);
	bpkt->ctrl.broad = 1;
	bpkt->ctrl.comm = ctrl & BILINK_PACKECT_CTRL_COMM_MASK;
	va_start(ap, ctrl);
	while (bpkt->bc.length < BILINK_PACKECT_MAX_BROADCAST_MSG_SIZE) {
		if ((type = va_arg(ap, int)) != -1) {
			if ((len = va_arg(ap, int)) != -1) {
				if (len == 0) {
					bpkt->bc.msg[bpkt->bc.length++] = (uint8_t)type;
					bpkt->bc.msg[bpkt->bc.length++] = (uint8_t)len;
				} else {
					if ((valaddr = va_arg(ap, long)) != -1) {
						val = (char *)valaddr;
						bpkt->bc.msg[bpkt->bc.length++] = (uint8_t)type;
						bpkt->bc.msg[bpkt->bc.length++] = (uint8_t)len;
						memcpy(bpkt->bc.msg+bpkt->bc.length, val, len);
						bpkt->bc.length += len;
					}
				}
			}
		} else {
			break;
		}
	}
	authorization_key(bpkt->bc.msg+bpkt->bc.length, srcaddr, key);

	va_end(ap);
	tol_len = bpkt->bc.length + BILINK_PACKECT_MAX_BROADCAST_HEAD_SIZE;
	return tol_len;
}

/*--------------------------------------------------------------------------------*/
int create_bilink_unicast_packet (uint8_t * buf, uint8_t * srcaddr, uint8_t * destaddr, uint8_t * key, uint8_t ctrl, ...) {
	va_list ap;
	union bilink_packet * bpkt = (union bilink_packet *)buf;
	uint8_t tol_len = 0;
//	uint8_t tol_len = 0;
	int len = 0;
	int type = 0;
	long valaddr;
	char * val = 0;

	bpkt->uc.length = 0;
	memcpy(bpkt->srcaddr, srcaddr, BILINK_PACKECT_SRC_ADDR_SIZE);
	memcpy(bpkt->uc.destaddr, destaddr, BILINK_PACKECT_DEST_ADDR_SIZE);
	bpkt->ctrl.broad = 0;
	bpkt->ctrl.comm = ctrl & BILINK_PACKECT_CTRL_COMM_MASK;
	va_start(ap, ctrl);
	while (bpkt->uc.length < BILINK_PACKECT_MAX_UNICAST_MSG_SIZE) {
		if ((type = va_arg(ap, int)) != -1) {
			if ((len = va_arg(ap, int)) != -1) {
				if (len == 0) {
					bpkt->uc.msg[bpkt->uc.length++] = (uint8_t)type;
					bpkt->uc.msg[bpkt->uc.length++] = (uint8_t)len;
				} else {
					if ((valaddr = va_arg(ap, long)) != -1) {
						val = (char *)valaddr;
						bpkt->uc.msg[bpkt->uc.length++] = (uint8_t)type;
						bpkt->uc.msg[bpkt->uc.length++] = (uint8_t)len;
						memcpy(bpkt->uc.msg+bpkt->uc.length, val, len);
						bpkt->uc.length += len;
					}
				}
			}
		} else {
			break;
		}
	}
	authorization_key(bpkt->uc.msg+bpkt->uc.length, srcaddr, key);

	va_end(ap);
	tol_len = bpkt->uc.length + BILINK_PACKECT_MAX_UNICAST_HEAD_SIZE;
	return tol_len;
}
/*--------------------------------------------------------------------------------*/



/*--------------------------------------------------------------------------------*/
#if BILINK_TEST
int main (int argc, char * argv[]) {
	int i;

	union bilink_packet pkt;

	int len = 0;
//	* ((uint64_t *)pkt.uc.srcaddr) = 0x0123456789ABCDEF;
	uint64_to_addr(0x0123456789ABCDEF, pkt.uc.srcaddr);
	pkt.uc.ctrl.broad = 1;
	pkt.uc.ctrl.comm = 1;
//	* ((uint64_t *)pkt.uc.destaddr) = 0x8080808080808080;
	uint64_to_addr(0x8080808080808080, pkt.uc.destaddr);
	pkt.uc.length = 1;
	pkt.uc.msg[0] = 'A';
	len = 30;
	printf("pkt val (hex): ");
	for (i=0; i<len; i++) {
		printf("%02X ", *(pkt.buf+i));
	}
	printf("\n");


	return 0;
}
#endif
/*--------------------------------------------------------------------------------*/
