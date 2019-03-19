#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <sys/queue.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <json-c/json.h>
#include <signal.h>
#include <sqlite3.h>
#include "../config.h"

#include "../serial/serial.h"
#include "../bilink/bilink.h"
#include "../bilink/bilink_packet.h"
#include "../packet/packet.h"
#include "../payload.h"
#include "../rts/rts.h"
#include "../timer/timer.h"
#include "../task/client.h"
#include "entry.h"

/*--------------------------------------------------------------------------------*/
#define DEBUG_PRINTF 0
#if DEBUG_PRINTF
    #define PRINTF(...) printf(__VA_ARGS__)
#else
    #define PRINTF(...)
#endif
/*--------------------------------------------------------------------------------*/
static struct bilink_conn bilink_conn_obj;
static struct task_entry * task_handler;
/*--------------------------------------------------------------------------------*/
int check_white_list (const char * filename, uint8_t * srcaddr);
int init_task (const char * sql_path, struct task_entry * hdr);
int read_bin_data(char * path, uint8_t * data);
void broadcast_block_data (int signo);

/*--------------------------------------------------------------------------------*/
static void bilink_recv_data (struct bilink_conn *ptr) {

}

static void bilink_sent (struct bilink_conn *ptr, int status, int num_tx) {
//	PRINTF("sent by rts: status=%d num_tx=%d\n", status, num_tx);
}

static struct bilink_callbacks bilink_callback_obj = {
	bilink_recv_data,
	bilink_sent,
};

/*--------------------------------------------------------------------------------*/
#define SRC_INCLUDED      0
#define SRC_NOT_INCLUDED  1
#define CHECK_SRC_ERROR   2
/*--------------------------------------------------------------------------------*/
int check_white_list (const char * filename, uint8_t * srcaddr) {
	  int ret;
	  int nrow=0;
	  int ncolumn = 0;
	  char **result = NULL;
	  char * errmsg;
	  char zsql[256];
	  char addr[20];
	  sqlite3 * pDb = NULL;
	  if ((ret = sqlite3_open(filename, &pDb)) == -1) {
	    perror("sql open");
		sqlite3_free(pDb);
	    return CHECK_SRC_ERROR;
	  }
	  addr_to_string(addr, srcaddr);
	  snprintf(zsql, 256, "SELECT device_id = %s FROM white_list;", addr);

      ret = sqlite3_get_table( pDb , zsql , &result , &nrow , &ncolumn , &errmsg );

      printf( "row:%d column=%d\n " , nrow , ncolumn );

	  if(errmsg != NULL) {
	    printf("%s\n", errmsg);
	    sqlite3_free(errmsg);
	    ret = CHECK_SRC_ERROR;
	  }

	  sqlite3_free_table(result);
	  sqlite3_close(pDb);

	return (nrow > 0 ? SRC_INCLUDED : SRC_NOT_INCLUDED);
}
/*--------------------------------------------------------------------------------*/
int init_task (const char * sql_path, struct task_entry * hdr) {
	int ret;
	int nrow=0;
	int ncolumn = 0;
	char **result = NULL;
	char * errmsg;
	char zsql[256];
	char task_id[20];
	sqlite3 * pDb = NULL;
	if ((ret = sqlite3_open(sql_path, &pDb)) == -1) {
	perror("sql open");
	sqlite3_free(pDb);
	return CHECK_SRC_ERROR;
	}
	memcpy(task_id, hdr->task_id, 16);
	task_id[16] = '\0';
	snprintf(zsql, 256, "SELECT * FROM task WHERE task_id = %s;", task_id);

	ret = sqlite3_get_table( pDb , zsql , &result , &nrow , &ncolumn , &errmsg );
//      +------------+------------+----------+-------------------------+-------------------------+---------+
//      | task_id    | data_id    | data_url | start_time              | end_time                | status  |
//      +------------+------------+----------+-------------------------+-------------------------+---------+
//      | String(16) | String(16) | String   | Time(%y-%M-%d %h:%m:%s) | Time(%y-%M-%d %h:%m:%s) | Integer |
//      +------------+------------+----------+-------------------------+-------------------------+---------+
//      | primary_key|            |          |                         |                         |         |
//      +------------+------------+----------+-------------------------+-------------------------+---------+
	printf( "row:%d column=%d\n " , nrow , ncolumn );

	if(errmsg != NULL) {
	printf("%s\n", errmsg);
	sqlite3_free(errmsg);
	ret = CHECK_SRC_ERROR;
	}

	memcpy (hdr->data_id, result[ncolumn+1], 16);
	strcpy (hdr->data_url, result[ncolumn+2]);
	strcpy (hdr->data_path, DATA_BIN_PATH);
	string_to_system_time(&hdr->start_time, result[ncolumn+3]);
	string_to_system_time(&hdr->end_time, result[ncolumn+4]);
	hdr->status = atoi(result[ncolumn+5]);
	sqlite3_free_table(result);
	sqlite3_close(pDb);


	task_handler->packets_num = 0;
	task_handler->packet_status = NULL;
	task_handler->status = STATUS_NONE;
	// read bin information
	task_handler->data_size =  read_bin_data(task_handler->data_path, NULL);
	task_handler->packets_num = (task_handler->data_size + BILINK_PACKECT_BROADCAST_MSG_SIZE - 1) / BILINK_PACKECT_BROADCAST_MSG_SIZE;

	task_handler->packet_status_size = (task_handler->packets_num + 31) / 32;

	if ((task_handler->packet_status = (uint32_t *)malloc(task_handler->packet_status_size)) == NULL) {
		printf("malloc memery for packet_status faild\n");
	}
	memset(task_handler->packet_status, 0, task_handler->packet_status_size<<2);

	  return (nrow > 0 ? SRC_INCLUDED : SRC_NOT_INCLUDED);
}

int read_bin_data(char * path, uint8_t * data) {
	int size = 0;
	FILE *fp;

	if (path == NULL)
		return -1;
	if ((fp = fopen(path, "rb")) == NULL) {
		printf("fopen %s error\n", path);
	}
	// if file opened, seek to the end of the file to get the length of the file.
	fseek(fp, 0, SEEK_END);
	// get the length
	size = ftell(fp);
	// return to the start
	rewind(fp);
	PRINTF("size of the %s file is %d\n", path, size);

	if (data != NULL) {
		// get the data into the array buffer
		fread(data, 1, size, fp);
	}
	// close the file
	fclose(fp);
	return size;
}

/*--------------------------------------------------------------------------------*/
void broadcast_block_data (int signo) {
//	FILE *fp;
//	FILE *fp2;
//	int size;
	int index = 0;
	int psize;
	int head_len;
	int count = 0;
	uint8_t * array = NULL;
//	char str[64];
	uint8_t word[2];
	struct simple_payload_buf p;

	if (task_handler->task_status == TASK_STATUS_LISTEN_ACCESS) {
		task_handler->task_status = TASK_STATUS_WAIT_SEND;
	} else if (task_handler->task_status == TASK_STATUS_LISTEN_TMP_RESULT) {
		task_handler->task_status = TASK_STATUS_WAIT_REISSUE;
	}

	if ((array = (uint8_t *)malloc(task_handler->data_size)) == NULL) {
		printf("malloc memory for array failed\n");
		return ;
	}
	// get the data into the array buffer
	read_bin_data(task_handler->data_path, array);

	authorization_key(word, bilink_conn_obj.selfaddr, bilink_conn_obj.newkey);
	// start to send data


	if ( task_handler->task_status == TASK_STATUS_WAIT_SEND ) {
		// first time send
		task_handler->task_status = TASK_STATUS_SEND;

		p.size = create_broadcast_packet (p.payload.data,
										  bilink_conn_obj.selfaddr,
										  BILINK_COMM_BROADCAST_DATA_BLOCK_REQ,
										  WITH_CONTENT);
		head_len = p.size;
		count = 0;
		for (index = 0; index < task_handler->data_size; index += BILINK_PACKECT_BROADCAST_MSG_SIZE) {
			p.size = head_len;

			p.payload.data[p.size++] = (uint8_t)((count>>8) & 0x000000FF);
			p.payload.data[p.size++] = (uint8_t)(count & 0x000000FF);

			if (task_handler->data_size > (index + BILINK_PACKECT_BROADCAST_MSG_SIZE)) {
				psize = BILINK_PACKECT_BROADCAST_MSG_SIZE;
			} else {
				psize = task_handler->data_size - index;
			}

			memcpy(p.payload.data+p.size, array+index, psize);
			p.size += psize;

			memcpy(p.payload.data+p.size, word, BILINK_PACKECT_AUTHORIZATION_KEY_SIZE);
			p.size += BILINK_PACKECT_AUTHORIZATION_KEY_SIZE;

			bilink_send_data(&bilink_conn_obj, &p);
			count++;
			timer_wait_ms(5);
		}
		task_handler->task_status = TASK_STATUS_SEND_OVER;
		p.size = create_broadcast_packet (p.payload.data,
										  bilink_conn_obj.selfaddr,
										  BILINK_COMM_UPDATE_RESULT_REQ,
										  WITH_CONTENT);
		p.size = packet_add_content (p.payload.data,
									 p.size,
									 BILINK_COMM_UPDATE_RESULT_SUB_AUTO_RESP,
									 BILINK_COMM_UPDATE_RESULT_SUB_AUTO_RESP_LEN,
									 NULL);
		memcpy(p.payload.data+p.size, word, BILINK_PACKECT_AUTHORIZATION_KEY_SIZE);
		p.size += BILINK_PACKECT_AUTHORIZATION_KEY_SIZE;

		bilink_send_data(&bilink_conn_obj, &p);

		task_handler->task_status = TASK_STATUS_LISTEN_TMP_RESULT;

	} else if (task_handler->task_status == TASK_STATUS_WAIT_REISSUE) {
		// second time send
		task_handler->task_status = TASK_STATUS_REISSUE;

		p.size = create_broadcast_packet (p.payload.data,
										  bilink_conn_obj.selfaddr,
										  BILINK_COMM_REISSUE_DATA_BLOCK_REQ,
										  WITH_CONTENT);
		head_len = p.size;
		count = 0;
		for (index = 0; index < task_handler->data_size; index += BILINK_PACKECT_BROADCAST_MSG_SIZE) {
			if (IS_BIT_OF_INT32_BUF(task_handler->packet_status, count)) {
				p.size = head_len;

				p.payload.data[p.size++] = (uint8_t)((count>>8) & 0x000000FF);
				p.payload.data[p.size++] = (uint8_t)(count & 0x000000FF);

				if (task_handler->data_size > (index + BILINK_PACKECT_BROADCAST_MSG_SIZE)) {
					psize = BILINK_PACKECT_BROADCAST_MSG_SIZE;
				} else {
					psize = task_handler->data_size - index;
				}

				memcpy(p.payload.data+p.size, array+index, psize);
				p.size += psize;

				memcpy(p.payload.data+p.size, word, BILINK_PACKECT_AUTHORIZATION_KEY_SIZE);
				p.size += BILINK_PACKECT_AUTHORIZATION_KEY_SIZE;

				bilink_send_data(&bilink_conn_obj, &p);
				timer_wait_ms(5);
			}
			count++;
		}
		task_handler->task_status = TASK_STATUS_REISSUE_OVER;
		memset(task_handler->packet_status, 0, task_handler->packet_status_size<<2);
		task_handler->task_status = TASK_STATUS_SEND_OVER;
		p.size = create_broadcast_packet (p.payload.data,
										  bilink_conn_obj.selfaddr,
										  BILINK_COMM_UPDATE_RESULT_REQ,
										  WITH_CONTENT);
		p.size = packet_add_content (p.payload.data,
									 p.size,
									 BILINK_COMM_UPDATE_RESULT_SUB_AUTO_RESP,
									 BILINK_COMM_UPDATE_RESULT_SUB_AUTO_RESP_LEN,
									 NULL);
		memcpy(p.payload.data+p.size, word, BILINK_PACKECT_AUTHORIZATION_KEY_SIZE);
		p.size += BILINK_PACKECT_AUTHORIZATION_KEY_SIZE;

		bilink_send_data(&bilink_conn_obj, &p);

		task_handler->task_status = TASK_STATUS_LISTEN_RESULT;
	}

	free(array);
}

/*--------------------------------------------------------------------------------*/
void communication_app (void) {


	signal(SIGALRM, broadcast_block_data);

}

/*--------------------------------------------------------------------------------*/
int parse_json_cmd (const char * str, int size) {
	struct json_object * recv_obj;
	struct json_object * key_obj;
	recv_obj = json_tokener_parse(str);
    if (TRUE == json_object_object_get_ex(recv_obj, "cmd", &key_obj)) {
    	if (0 == strncmp(json_object_to_json_string(key_obj), "task", 4)) {
    	    if (TRUE == json_object_object_get_ex(recv_obj, "method", &key_obj)) {
    	    	if (0 == strncmp(json_object_to_json_string(key_obj), "create", 6)) {
    	    		// creste
    	    		if (TRUE == json_object_object_get_ex(recv_obj, "task_id", &key_obj)) {
//    	    			int packet_status_size;
    	    			// create task
    	    			if ((task_handler = (struct task_entry *)malloc(sizeof(struct task_entry))) == NULL) {
    	    				printf("malloc for task_entry failed\n");
    	    				return -1;
    	    			}
    	    			printf("recv create task\n");
    	    			task_handler->task_status = TASK_STATUS_IDLE;
    	    			// read sql and initialize task handler
    	    			init_task(SQL_TABLE_PATH, task_handler);
    	    			// open entry and start to rTASK_STATUS_NONEeceive
    	    			entry_open();
    	    			// initialize key
    	    			task_handler->task_status = TASK_STATUS_LISTEN_ACCESS;
    	    			// set timer signal
    	    			signal(SIGALRM, broadcast_block_data);

    	    			alarm(1);

    	    			//

    	    		} else {
    	    	    	printf("there is no key named \"task_id\".\n");
    	    		}
    	    	} else if (0 == strncmp(json_object_to_json_string(key_obj), "cancel", 6)) {
    	    		// cancel task
    	    		printf("recv cancel task\n");
    	    		// close entry

    	    		// delete task

    	    	}

    	    } else {
    	    	printf("there is no key named \"method\".\n");
    	    }

    	} else if (0 == strncmp(json_object_to_json_string(key_obj), "restart", 7)) {
    		// send reset to mcu

    		// wait a moment
    		sleep(1);
    		// reset linux
    		printf("reboot system\n");
    		// system("reboot\n");
    	}
    } else {
    	printf("there is no key named \"cmd\".\n");
    }


	return 0;
}



int send_success_register_client_to_server (uint8_t * device_id, uint8_t * auth_key, uint8_t * msg) {
    struct json_object * new_obj = NULL;
//	struct json_object * key_obj = NULL;
	char tmp_str[20];
    char * send_buf;
    char * recv_buf;
    int tmp;

    if ( (recv_buf = (char *)malloc(UNIX_DOMAINRECV_BUFFER_SIZE) ) == NULL ) {
        printf("malloc recv_buf failed.\n");
        return -1;
    }

    if ((new_obj = json_object_new_object()) == NULL) {
        printf("new json object failed.\n");
        free(recv_buf);
        return -1;
    }
    json_object_object_add(new_obj, "cmd", json_object_new_string("register"));
    addr_to_string(tmp_str, device_id);
    json_object_object_add(new_obj, "device_id", json_object_new_string(tmp_str));
    tmp = auth_key[0];
    tmp <<= 8;
    tmp += auth_key[1];
    json_object_object_add(new_obj, "auth_key", json_object_new_int(tmp));

    send_buf = (char *)json_object_to_json_string(new_obj);
    unix_send_without_recv (UNIX_DOMAIN_CLIENT, send_buf, strlen(send_buf));
//    tmp = unix_send_with_recv (UNIX_DOMAIN_CLIENT,
//    								send_buf,
//									strlen(send_buf),
//									recv_buf,
//									UNIX_DOMAINRECV_BUFFER_SIZE);
    json_object_put(new_obj);
    free(recv_buf);

	return 0;
}

int send_network_access_to_server(uint8_t * device_id, uint8_t * data_id, int battery, int status, uint8_t * msg) {
    struct json_object * new_obj = NULL;
//	struct json_object * key_obj = NULL;
	char tmp_str[20];
    char * send_buf;
    char * recv_buf;
//    int recv_size;

    if ( (recv_buf = (char *)malloc(UNIX_DOMAINRECV_BUFFER_SIZE) ) == NULL ) {
        printf("malloc recv_buf failed.\n");
        return -1;
    }

    if ((new_obj = json_object_new_object()) == NULL) {
        printf("new json object failed.\n");
        free(recv_buf);
        return -1;
    }
    json_object_object_add(new_obj, "cmd", json_object_new_string("heart"));
    addr_to_string(tmp_str, device_id);
    json_object_object_add(new_obj, "device_id", json_object_new_string(tmp_str));
    data_id_to_string(tmp_str, data_id);
    json_object_object_add(new_obj, "data_id", json_object_new_string(tmp_str));
    json_object_object_add(new_obj, "battery", json_object_new_int(battery));
    send_buf = (char *)json_object_to_json_string(new_obj);
    unix_send_without_recv (UNIX_DOMAIN_CLIENT, send_buf, strlen(send_buf));
//    recv_size = unix_send_with_recv (UNIX_DOMAIN_CLIENT,
//    								send_buf,
//									strlen(send_buf),
//									recv_buf,
//									UNIX_DOMAINRECV_BUFFER_SIZE);
    json_object_put(new_obj);
    free(recv_buf);
	return 0;
}

int send_update_schedule_to_server(uint8_t * srcaddr) {

	return 0;
}

int send_update_result_to_server(uint8_t * srcaddr) {

	return 0;
}

/*--------------------------------------------------------------------------------*/
int entry_open (void) {
	int ret;
    ret = bilink_open(&bilink_conn_obj, &bilink_callback_obj);
    printf("bilink opened\n");

    return ret;
}

int entry_send_data (const char * data, int size) {
	int ret;
	if (data == NULL || size > SIMPLE_PAYLOAD_LENGTH_MAX) {
		return -1;
	}
	if (size <= 0) {
		return -1;
	}
	struct simple_payload_buf payload;
	payload.size = size;
	memcpy(payload.buf+1, data, size);
	payload.buf[0] = 0x80;
	payload.size += 1;
    ret = bilink_send_data (&bilink_conn_obj, &payload);
	return ret;
}

void entry_close (void) {
	bilink_close(&bilink_conn_obj);
}
int entry_receive_data (char * data, int msec) {
	int ret = 0;

	ret = serial_receive_from_list (&bilink_conn_obj.rts.ots.pld.serial, (uint8_t *)data, msec);

	return ret;
}


int parse_serial_data (void) {
	union bilink_packet b;
	struct simple_payload_buf p;
	int size;
	size = serial_receive_from_list (&bilink_conn_obj.rts.ots.pld.serial, b.buf, 20);

	if (size > 0) {
		if (b.ctrl.broad) {
			switch (b.ctrl.comm) {
				case BILINK_COMM_BEAT_REQ :
					if (1 || SRC_INCLUDED == check_white_list(SQL_TABLE_PATH, b.srcaddr)) {

						uint8_t word[2];
						authorization_key(word, b.srcaddr, bilink_conn_obj.newkey);
						if (0 == memcmp(word, b.bc.akey,
								BILINK_PACKECT_AUTHORIZATION_KEY_SIZE)) {

							p.size = create_unicast_packet(p.payload.data,
									bilink_conn_obj.selfaddr,
									BILINK_COMM_BEAT_RESP,
									b.srcaddr,
									WITHOUT_CONTENT);
							p.size = packet_add_autherization_key(p.payload.data,
									p.size,
									bilink_conn_obj.newkey);

							bilink_send_data(&bilink_conn_obj, &p);

						}
					}
					break;
				case BILINK_COMM_REGISTER_REQ :
					if (SRC_INCLUDED == check_white_list(SQL_TABLE_PATH, b.srcaddr)) {
						uint8_t word[2];
						authorization_key(word, b.srcaddr, bilink_conn_obj.initkey);
						if (0 == memcmp(word, b.bc.akey,
								BILINK_PACKECT_AUTHORIZATION_KEY_SIZE)) {
//							struct simple_payload_buf p;
							p.size = create_unicast_packet(p.payload.data,
									bilink_conn_obj.selfaddr,
									BILINK_COMM_UPDATE_ACCESS_KEY_REQ,
									b.srcaddr,
									WITH_CONTENT);
							p.size = packet_add_content(p.payload.data,
									p.size,
									BILINK_COMM_UPDATE_ACCESS_KEY_SUB_NEW_KEY,
									BILINK_COMM_UPDATE_ACCESS_KEY_SUB_NEW_KEY_LEN,
									bilink_conn_obj.newkey);
							p.size = packet_add_autherization_key(p.payload.data,
									p.size,
									bilink_conn_obj.initkey);

							bilink_send_data(&bilink_conn_obj, &p);

						}
					}
					break;
				default  :

					break;
			}
		} else {
			uint8_t type;
			uint8_t length;
			uint8_t index = 0;
			uint8_t word[2];
			// destination matching
			if (memcmp(b.uc.destaddr, bilink_conn_obj.selfaddr, BILINK_PACKECT_DEST_ADDR_SIZE) != 0)
				return 0;
			// create and match A-key
			authorization_key(word, b.srcaddr, bilink_conn_obj.newkey);
			if (0 != memcmp(word, b.bc.akey, BILINK_PACKECT_AUTHORIZATION_KEY_SIZE))
				return 0;
			// select
			switch (b.ctrl.comm) {
				case BILINK_COMM_NETWORK_ACCESS_REQ :
					index = 0;
					uint8_t data_id[2];
					uint8_t msg[20];
					int battery;
					while (index < b.uc.length) {
						type = b.uc.msg[index++];
						length = b.uc.msg[index++];
						switch (type) {
							case BILINK_COMM_NETWORK_ACCESS_SUB_POWER :
								battery = b.uc.msg[index];
								break;
							case BILINK_COMM_NETWORK_ACCESS_SUB_DATA_VERSION :
								memcpy(data_id, b.uc.msg+index, BILINK_COMM_NETWORK_ACCESS_SUB_DATA_VERSION_LEN);
								break;
							case BILINK_COMM_NETWORK_ACCESS_SUB_LAST_UPDATE_TIME :
								memcpy(msg, b.uc.msg+index, BILINK_COMM_NETWORK_ACCESS_SUB_LAST_UPDATE_TIME_LEN);
								break;
							default :
								break;
						}
						index += length;
					}

					send_network_access_to_server(b.srcaddr, data_id, battery, 0, msg);
					break;
				case BILINK_COMM_APPOINT_UPDATE_SCHEDULE_RESP :
					index = 0;
					while (index < b.uc.length) {
						type = b.uc.msg[index++];
						length = b.uc.msg[index++];
						switch (type) {
							case BILINK_COMM_APPOINT_UPDATE_SCHEDULE_SUB_WHETHER_TO_UPDATE :

								break;

							default :
								break;
						}
						index += length;
					}
//					send_update_schedule_to_server(b.srcaddr);

					break;
				case BILINK_COMM_UPDATE_RESULT_RESP :
					index = 0;
					while (index < b.uc.length) {
						type = b.uc.msg[index++];
						length = b.uc.msg[index++];
						switch (type) {
							case BILINK_COMM_UPDATE_RESULT_SUB_TOTAL_LOST_NUM :

								break;
							case BILINK_COMM_UPDATE_RESULT_SUB_LOST_PKT_ELEMT :

								break;
							default :
								break;
						}
						index += length;
					}
//					send_update_result_to_server(b.srcaddr);

					break;
				case BILINK_COMM_REISSUE_DATA_BLOCK_RESP :

					break;
				case BILINK_COMM_UPDATE_ACCESS_KEY_RESP :
					if (SRC_INCLUDED == check_white_list(SQL_TABLE_PATH, b.srcaddr)) {
						uint8_t word[2];
						authorization_key(word, b.srcaddr, bilink_conn_obj.newkey);
						if (0 == memcmp(word, b.uc.akey,
								BILINK_PACKECT_AUTHORIZATION_KEY_SIZE)) {
							send_success_register_client_to_server (b.srcaddr, bilink_conn_obj.newkey, NULL);
						}
					}
					break;
				default  :
					break;
			}
		}


	} else {

	}
	return 0;
}

