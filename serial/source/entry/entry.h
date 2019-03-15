#ifndef __SIMPLE_ENTRY_H__
#define __SIMPLE_ENTRY_H__

#include "../timer/timer.h"

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

struct task_entry {
	char task_id[16];
	char data_id[16];
	char data_url[256];
	char data_path[256];
	struct tm_utc start_time;
	struct tm_utc end_time;
	int status;
	char auth_key[2];
	int packets_num;
	int data_size;
	int task_status;
	int packet_status_size;
	uint32_t * packet_status;
};

#define IS_BIT_OF_INT32_BUF(buf, idx)     ((*((buf) + (idx) / 32)) & (0x00000001 << ((idx) % 32)))
#define SET_BIT_OF_INT32_BUF(buf, idx)    (*((buf) + (idx) / 32)) |= (0x00000001 << ((idx) % 32))
#define RESET_BIT_OF_INT32_BUF(buf, idx)  (*((buf) + (idx) / 32)) &= ~(0x00000001 << ((idx) % 32))

int entry_open (char * dev);

int entry_send_data (const char * data, int size);

int entry_receive_data (char * data, int msec);

void entry_close (void);

int parse_json_cmd (const char * str, int size);

#endif /*__SIMPLE_ENTRY_H__*/
