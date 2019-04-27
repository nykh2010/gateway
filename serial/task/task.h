#ifndef __TASK_H__
#define __TASK_H__

#include "../entrydef.h"

enum status_enum {
	STATUS_NONE = 0,
	STATUS_SLEEP,
	STATUS_READY,
	STATUS_RUN,
	STATUS_FINISH,
	STATUS_SUSPEND,
};
typedef enum status_enum status_t;

enum task_status_enum {
	TASK_STATUS_IDLE = 0,
	TASK_STATUS_LISTEN_ACCESS,
	TASK_STATUS_WAIT_SEND,
	TASK_STATUS_SEND,
	TASK_STATUS_SEND_OVER,
	TASK_STATUS_LISTEN_TMP_RESULT,
	TASK_STATUS_WAIT_REISSUE,
	TASK_STATUS_REISSUE,
	TASK_STATUS_REISSUE_OVER,
	TASK_STATUS_LISTEN_RESULT,
	TASK_STATUS_FINISH,
	TASK_STATUS_SUSPEND,
};
typedef enum task_status_enum task_status_t;


#define IS_BIT_OF_INT32_BUF(buf, idx)     ((*((buf) + (idx) / 32)) & (0x00000001 << ((idx) % 32)))
#define SET_BIT_OF_INT32_BUF(buf, idx)    (*((buf) + (idx) / 32)) |= (0x00000001 << ((idx) % 32))
#define RESET_BIT_OF_INT32_BUF(buf, idx)  (*((buf) + (idx) / 32)) &= ~(0x00000001 << ((idx) % 32))


int parse_json_cmd (struct entry * e, const char * str, int size);

int parse_serial_data (struct entry * e);


#endif /*__TASK_H__*/
