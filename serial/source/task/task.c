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
#include "../entrydef.h"
#include "../bi_client/bi_client.h"
#include "../bi_server/bi_server.h"
#include "task.h"
#include "../inifun/inirw.h"


#define DEBUG 1
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

//extern struct entry tEntry;

/*--------------------------------------------------------------------------------*/
#define SRC_INCLUDED      0
#define SRC_NOT_INCLUDED  1
#define CHECK_SRC_ERROR   2
//static struct task_entry * task_handler;
/*--------------------------------------------------------------------------------*/
//int check_white_list (const char * filename, uint8_t * srcaddr);
int read_bin_data(char * path, uint8_t ** data);
//void signal_handler (int signo);
/*--------------------------------------------------------------------------------*/
int write_string_to_file (const char * filename, char * srcaddr) {
	FILE *fp;

	if (filename == NULL) {
		log_error("A wrong file name or path");
		return -1;
	}

	if ((fp = fopen(filename, "rw")) == NULL) {
		log_error("open %s error.", filename);
		return -1;
	}

	fseek(fp, 0, SEEK_END);

	fputs(srcaddr, fp);

	fclose(fp);

	return 0;
}
/*--------------------------------------------------------------------------------*/
#if 0
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
	    log_error("sql open");
		sqlite3_free(pDb);
	    return CHECK_SRC_ERROR;
	  }
	  addr_to_string(addr, srcaddr);
	  snprintf(zsql, 256, "SELECT * FROM white_list WHERE device_id = %s;", addr);

      ret = sqlite3_get_table( pDb , zsql , &result , &nrow , &ncolumn , &errmsg );

      log_debug( "row:%d column=%d\n " , nrow , ncolumn );

	  if(errmsg != NULL) {
	    log_error("%s\n", errmsg);
	    sqlite3_free(errmsg);
	    ret = CHECK_SRC_ERROR;
	  }

	  sqlite3_free_table(result);
	  sqlite3_close(pDb);

	return (nrow > 0 ? SRC_INCLUDED : SRC_NOT_INCLUDED);
}

/*--------------------------------------------------------------------------------*/
int check_execute_list (const char * filename, uint8_t * srcaddr) {
	  int ret;
	  int nrow=0;
	  int ncolumn = 0;
	  char **result = NULL;
	  char * errmsg;
	  char zsql[256];
	  char addr[20];
	  sqlite3 * pDb = NULL;
	  if ((ret = sqlite3_open(filename, &pDb)) == -1) {
	    log_error("sql open");
		sqlite3_free(pDb);
	    return CHECK_SRC_ERROR;
	  }
	  addr_to_string(addr, srcaddr);
	  snprintf(zsql, 256, "SELECT * FROM execute WHERE device_id = %s;", addr);

      ret = sqlite3_get_table( pDb , zsql , &result , &nrow , &ncolumn , &errmsg );

      log_debug( "row:%d column=%d\n " , nrow , ncolumn );

	  if(errmsg != NULL) {
	    log_error("%s\n", errmsg);
	    sqlite3_free(errmsg);
	    ret = CHECK_SRC_ERROR;
	  }

	  sqlite3_free_table(result);
	  sqlite3_close(pDb);

	return (nrow > 0 ? SRC_INCLUDED : SRC_NOT_INCLUDED);
}
#endif
/*--------------------------------------------------------------------------------*/
int read_bin_data(char * path, uint8_t ** data) {
	int size = 0;
	FILE *fp;

	if (path == NULL) {
		return -1;
	}
	if ((fp = fopen(path, "rb")) == NULL) {
		log_error("fopen %s error.", path);
	}
	// if file opened, seek to the end of the file to get the length of the file.
	fseek(fp, 0, SEEK_END);
	// get the length
	size = ftell(fp);

	log_debug("size of the %s file is %d\n", path, size);
	if (data == NULL) {
		// close the file
		fclose(fp);
		return size;
	}
	// return to the start
	rewind(fp);

	if ((*data = (uint8_t *)malloc(size)) == NULL) {
		log_error("malloc memory for array failed\n");
		// close the file
		fclose(fp);
		return 0;
	}

	// get the data into the array buffer
	fread(*data, 1, size, fp);

	// close the file
	fclose(fp);
	return size;
}
/*--------------------------------------------------------------------------------*/
static void * task_send_thread_exec(void * arg) {
	struct task_entry * t = (struct task_entry *)arg;
	int index = 0;
	int psize;
	int head_len;
	int count = 0;
	time_t timer;
	// to store file
	uint8_t * array = NULL;
	// akey
	uint8_t word[2];
	// send buffer
	struct simple_payload_buf p;

	t->task_status = TASK_STATUS_LISTEN_ACCESS;
	// ==============================================================================
	time(&timer);
	timer_wait_ms((t->start_time - timer)*1000);

	// change status to wait for send
	if (t->task_status == TASK_STATUS_LISTEN_ACCESS) {
		t->task_status = TASK_STATUS_WAIT_SEND;
	} else if (t->task_status == TASK_STATUS_LISTEN_TMP_RESULT) {
		t->task_status = TASK_STATUS_WAIT_REISSUE;
	}

	// get the data into the array buffer
	if ((t->data_size = read_bin_data(t->data_path, &array)) == 0) {
		log_error("read bin size failed.");
		return NULL;
	}
	// calculate key
	authorization_key(word, t->parent->bhdr.selfaddr, t->parent->bhdr.newkey);
	// start to send data
//	switch (t->task_status ) {
//		case TASK_STATUS_WAIT_SEND :
//
//			break;
//		case TASK_STATUS_SEND :
//
//			break;
//		case TASK_STATUS_SEND_OVER :
//
//			break;
//		case TASK_STATUS_LISTEN_TMP_RESULT :
//
//			break;
//		case TASK_STATUS_WAIT_REISSUE :
//
//			break;
//		case TASK_STATUS_REISSUE :
//
//			break;
//		case TASK_STATUS_REISSUE_OVER :
//
//			break;
//		case TASK_STATUS_LISTEN_RESULT :
//
//			break;
//		case TASK_STATUS_FINISH :
//
//			break;
//		case TASK_STATUS_SUSPEND :
//
//			break;
//	}

	if ( t->task_status == TASK_STATUS_WAIT_SEND ) {
		// first time send
		t->task_status = TASK_STATUS_SEND;
		log_debug("task status is TASK_STATUS_SEND.");

		if (0 != rts_send_ctrl(&t->parent->bhdr.rts, SIMPLE_PAYLOAD_SEND_MODE, 20)) {
			log_error("Radio change to SEND mode error.");
		}
		// create control packet, broadcast,
		p.size = create_broadcast_packet (p.payload.data,
										  t->parent->bhdr.selfaddr,
										  BILINK_CONTROL_REQ,
										  WITH_CONTENT);
		// add data version
		p.size = packet_add_content (p.payload.data,
									 p.size,
									 BILINK_CONTROL_REQ_SUB_KEY_VERSION,
									 BILINK_CONTROL_REQ_SUB_LEN_VERSION,
									 NULL);
		// add data packet number
		p.size = packet_add_content (p.payload.data,
									 p.size,
									 BILINK_CONTROL_REQ_SUB_KEY_PKT_NUM,
									 BILINK_CONTROL_REQ_SUB_LEN_PKT_NUM,
									 NULL);
		// add data next flush time, reissue time
		p.size = packet_add_content (p.payload.data,
									 p.size,
									 BILINK_CONTROL_REQ_SUB_KEY_NEXT_FLUSH_TIME,
									 BILINK_CONTROL_REQ_SUB_LEN_NEXT_FLUSH_TIME,
									 NULL);
		// add author key
		p.size = packet_add_key (p.payload.data, p.size, t->auth_key);
		// send a packet
		bilink_send_data(&t->parent->bhdr, &p);
		// wait 8ms at least
		timer_wait_ms(8);
		// create data packet
		p.size = create_broadcast_packet (p.payload.data,
										  t->parent->bhdr.selfaddr,
										  BILINK_DATA_REQ,
										  WITH_CONTENT);
		// store head byte size
		head_len = p.size;
		// sequeue count
		count = 0;
		for (index = 0; index < t->data_size; index += BILINK_PACKECT_BROADCAST_MSG_SIZE) {
			p.size = head_len;
			// data sequece number
			p.payload.data[p.size++] = (uint8_t)((count>>8) & 0x000000FF);
			p.payload.data[p.size++] = (uint8_t)(count & 0x000000FF);
			// data length
			if (t->data_size > (index + BILINK_PACKECT_BROADCAST_MSG_SIZE)) {
				psize = BILINK_PACKECT_BROADCAST_MSG_SIZE;
			} else {
				psize = t->data_size - index;
			}
			// copy data
			memcpy(p.payload.data+p.size, array+index, psize);
			// add data length
			p.size += psize;
			// add author key
			p.size = packet_add_key (p.payload.data, p.size, t->auth_key);
			// send a packet
			bilink_send_data(&t->parent->bhdr, &p);
			// wait 8ms at least
			timer_wait_ms(8);
			count++;
		}
		t->task_status = TASK_STATUS_SEND_OVER;
		log_debug("task status is TASK_STATUS_SEND_OVER.");
		// create query packet
		p.size = create_broadcast_packet (p.payload.data,
										  t->parent->bhdr.selfaddr,
										  BILINK_QUERY_REQ,
										  WITH_CONTENT);
		// add reissue time
		p.size = packet_add_content (p.payload.data,
									 p.size,
									 BILINK_QUERY_REQ_SUB_KEY_REISSUE_TIME,
									 BILINK_QUERY_REQ_SUB_LEN_REISSUE_TIME,
									 NULL);
		// add author key
		p.size = packet_add_key (p.payload.data, p.size, t->auth_key);
		// send a packet
		bilink_send_data(&t->parent->bhdr, &p);
		// wait 8ms at least
		t->task_status = TASK_STATUS_LISTEN_TMP_RESULT;
		log_debug("task status is TASK_STATUS_LISTEN_TMP_RESULT.");

		if (0 != rts_send_ctrl(&t->parent->bhdr.rts, SIMPLE_PAYLOAD_RECV_MODE, 20)) {
			log_error("Radio change to RECV mode error.");
		}
		// ==============================================================================
		// set timer signal
		time(&timer);
		timer_wait_ms(((t->end_time>>1) + (t->start_time>>1) - timer)*1000);

	}
	t->task_status = TASK_STATUS_WAIT_REISSUE;
	if (t->task_status == TASK_STATUS_WAIT_REISSUE) {
		// second time send
		t->task_status = TASK_STATUS_REISSUE;
		log_debug("task status is TASK_STATUS_REISSUE.");

		if (0 != rts_send_ctrl(&t->parent->bhdr.rts, SIMPLE_PAYLOAD_SEND_MODE, 20)) {
			log_error("Radio change to SEND mode error.");
		}
		// create data packet
		p.size = create_broadcast_packet (p.payload.data,
										  t->parent->bhdr.selfaddr,
										  BILINK_REISSUE_REQ,
										  WITH_CONTENT);
		// store head byte size
		head_len = p.size;
		count = 0;
		for (index = 0; index < t->data_size; index += BILINK_PACKECT_BROADCAST_MSG_SIZE) {
			if (IS_BIT_OF_INT32_BUF(t->packet_status, count)) {
				p.size = head_len;
				// data sequece number
				p.payload.data[p.size++] = (uint8_t)((count>>8) & 0x000000FF);
				p.payload.data[p.size++] = (uint8_t)(count & 0x000000FF);
				// data length
				if (t->data_size > (index + BILINK_PACKECT_BROADCAST_MSG_SIZE)) {
					psize = BILINK_PACKECT_BROADCAST_MSG_SIZE;
				} else {
					psize = t->data_size - index;
				}
				// copy data
				memcpy(p.payload.data+p.size, array+index, psize);
				// add data length
				p.size += psize;
				// add author key
				p.size = packet_add_key (p.payload.data, p.size, t->auth_key);
				// send a packet
				bilink_send_data(&t->parent->bhdr, &p);
				// wait 8ms at least
				timer_wait_ms(8);
			}
			count++;
		}
		t->task_status = TASK_STATUS_REISSUE_OVER;
		log_debug("task status is TASK_STATUS_REISSUE_OVER.");

		// clear status
		memset(t->packet_status, 0, t->packet_status_size<<2);
		// create query packet
		p.size = create_broadcast_packet (p.payload.data,
										  t->parent->bhdr.selfaddr,
										  BILINK_QUERY_REQ,
										  WITH_CONTENT);
		// add reissue time
		// p.size = packet_add_content (p.payload.data,
		// 							 p.size,
		// 							 BILINK_QUERY_REQ_SUB_KEY_REISSUE_TIME,
		// 							 BILINK_QUERY_REQ_SUB_LEN_REISSUE_TIME,
		// 							 NULL);
		// add author key
		p.size = packet_add_key (p.payload.data, p.size, t->auth_key);
		// send a packet
		bilink_send_data(&t->parent->bhdr, &p);
		t->task_status = TASK_STATUS_LISTEN_RESULT;
		log_debug("task status is TASK_STATUS_LISTEN_RESULT.");

		if (0 != rts_send_ctrl(&t->parent->bhdr.rts, SIMPLE_PAYLOAD_RECV_MODE, 20)) {
			log_error("Radio change to RECV mode error.");
		}
		//==================================================================================
		time(&timer);
		timer_wait_ms((t->end_time - timer)*1000);
	}
	free(array);
	t->task_status = TASK_STATUS_FINISH;
	log_debug("task status is TASK_STATUS_FINISH.");
//	free(t->execlist_cache);
	if (0 != rts_send_ctrl(&t->parent->bhdr.rts, SIMPLE_PAYLOAD_RESE_MODE, 20)) {
		log_error("Radio change to RESE mode error.");
	}
	free(t->packet_status);
	return NULL;
}
/*--------------------------------------------------------------------------------*/
int task_open (struct task_entry * t) {
	int ret = 0;

	// read bin data size
	t->data_size =  read_bin_data(t->data_path, NULL);
	// init packet number, calculate packet number
	t->packets_num = (t->data_size + BILINK_PACKECT_BROADCAST_MSG_SIZE - 1) / BILINK_PACKECT_BROADCAST_MSG_SIZE;

	// init packet status array
	t->packet_status_size = (t->packets_num + 31) / 32;
	if (t->packet_status != NULL) {
		free(t->packet_status);
	}
	if ((t->packet_status = (uint32_t *)malloc(t->packet_status_size)) == NULL) {
		log_error("malloc memery for packet_status faild.");
		ret = -1;
	}
	memset(t->packet_status, 0, t->packet_status_size*4);

	// creat tmp exec list
	IniReadValue("[task]", "ExecOkListPath", t->execlist_ok_path, CONFIG_PATH);
	IniReadValue("[task]", "ExecFailedListPath", t->execlist_failed_path, CONFIG_PATH);

	t->task_status = TASK_STATUS_LISTEN_ACCESS;
	// create thread
	if (pthread_create(&t->thread, NULL, task_send_thread_exec, (void *)t) != 0) {
		log_error("create server thread failed\n");
		return -1;
	}

	return ret;
}
#if 0
int task_open (struct task_entry * t, const char * sql_path) {
	int ret = -1;
	int nrow=0;
	int idx;
	int ncolumn = 0;
	char **result = NULL;
	char * errmsg;
	char zsql[256];
//	char task_id[20];
	sqlite3 * pDb = NULL;
	t->task_status = TASK_STATUS_IDLE;
	if ((ret = sqlite3_open(sql_path, &pDb)) == -1) {
		log_error("sql open");
		sqlite3_free(pDb);
		return -1;
	}

	snprintf(zsql, 256, "SELECT * FROM task WHERE task_id = %s;", t->task_id);
	sqlite3_get_table( pDb , zsql , &result , &nrow , &ncolumn , &errmsg );
	//      +------------+------------+----------+-------------------------+-------------------------+---------+
	//      | task_id    | data_id    | data_url | start_time              | end_time                | status  |
	//      +------------+------------+----------+-------------------------+-------------------------+---------+
	//      | String(16) | String(16) | String   | Time(%y-%M-%d %h:%m:%s) | Time(%y-%M-%d %h:%m:%s) | Integer |
	//      +------------+------------+----------+-------------------------+-------------------------+---------+
	//      | primary_key|            |          |                         |                         |         |
	//      +------------+------------+----------+-------------------------+-------------------------+---------+
	log_debug( "row:%d column=%d\n " , nrow , ncolumn );

	if(errmsg != NULL) {
		printf("%s\n", errmsg);
		sqlite3_free(errmsg);
		ret = -1;
	}
	if (nrow > 1) {

		// init data id
		strcpy (t->data_id, result[ncolumn+1]);
		// init data path
		strcpy (t->data_path, DATA_BIN_PATH);
		// init start time
		t->start_time = string_to_time(result[ncolumn+3]);
		// init end time
		t->end_time = string_to_time(result[ncolumn+4]);
		// init status
		t->status = atoi(result[ncolumn+5]);
		//	t->status = STATUS_NONE;
		// release space
		sqlite3_free_table(result);
		snprintf(zsql, 256, "SELECT * FROM execute WHERE task_id = %s;", t->task_id);
		sqlite3_get_table( pDb , zsql , &result , &nrow , &ncolumn , &errmsg );
		if(errmsg != NULL) {
			log_error("%s\n", errmsg);
			sqlite3_free(errmsg);
			ret = -1;
		}
		sqlite3_close(pDb);
		// create table
//		sqlite3_open(LOCAL_SQL_TABLE_PATH, &pDb);
//
//		strcpy(zsql,"CREATE TABLE execlist (device_id CHAR(16) PRIMARY KEY, status INT);");
//		sqlite3_exec(pDb, zsql, NULL, NULL, &zErrMsg);
//		if(errmsg != NULL) {
//			PRINTF("%s\n", errmsg);
//			sqlite3_free(errmsg);
//			ret = -1;
//		}

		// create execute cache
		if ((t->execlist_cache = (uint8_t *)malloc( (BILINK_PACKECT_SRC_ADDR_SIZE+1)*nrow)) == NULL)  {
			log_error("malloc for execute list cache failed.");
			return -1;
		}
		t->execlist_len = nrow-1;
		memset(t->execlist_cache, 0, (BILINK_PACKECT_SRC_ADDR_SIZE+1)*nrow);
		for (idx=1; idx<(nrow+1); idx++) {
			str_to_hex((t->execlist_cache+(BILINK_PACKECT_SRC_ADDR_SIZE+1)*(idx-1)), result[idx*ncolumn], 16);
		}


		// read bin data size
		t->data_size =  read_bin_data(t->data_path, NULL);
		// init packet number, calculate packet number
		t->packets_num = (t->data_size + BILINK_PACKECT_BROADCAST_MSG_SIZE - 1) / BILINK_PACKECT_BROADCAST_MSG_SIZE;

		// init packet status array
		t->packet_status_size = (t->packets_num + 31) / 32;
		if (t->packet_status != NULL) {
			free(t->packet_status);
		}
		if ((t->packet_status = (uint32_t *)malloc(t->packet_status_size)) == NULL) {
			log_error("malloc memery for packet_status faild.");
			ret = -1;
		}
		memset(t->packet_status, 0, t->packet_status_size*4);

		// creat tmp exec list
		strcpy(t->execlist_ok_path, LOCAL_SQL_TABLE_PATH);

		// initialize key

		// create thread
	    if (pthread_create(&t->thread, NULL, task_send_thread_exec, (void *)t) != 0) {
	        log_error("create server thread failed\n");
	        return -1;
	    }

		t->task_status = TASK_STATUS_LISTEN_ACCESS;
	}
	return 0;
}
#endif
int task_close (struct task_entry * t) {
	free(t->packet_status);
	return 0;
}


//
///*--------------------------------------------------------------------------------*/
//void signal_handler (int signo) {
//
//}
//
///*--------------------------------------------------------------------------------*/
//void communication_app (void) {
//
//
//	signal(SIGALRM, signal_handler);
//
//}

/*--------------------------------------------------------------------------------*/
int parse_json_cmd (struct entry * e, const char * str, int size) {
	int ret = 0;
	struct json_object * recv_obj;
	struct json_object * key_obj;
	char tmp_str[20];
	recv_obj = json_tokener_parse(str);
    if (TRUE == json_object_object_get_ex(recv_obj, "cmd", &key_obj)) {
    	if (0 == strncmp(json_object_to_json_string(key_obj), "task", 4)) {
    	    if (TRUE == json_object_object_get_ex(recv_obj, "method", &key_obj)) {
    	    	if (0 == strncmp(json_object_to_json_string(key_obj), "create", 6)) {
    	    		// creste
	    			// clear task
	    			memset(&e->thdr, 0, sizeof(struct task_entry));
	    			// register parent
	    			e->thdr.parent = e;
	    			//
    	    		if (TRUE == json_object_object_get_ex(recv_obj, "task_id", &key_obj)) {
    	    			// copy task id
    	    			strncpy(e->thdr.task_id, json_object_to_json_string(key_obj), 16);
    	    		} else {
    	    	    	log_error("there is no key named \"task_id\".");
    	    	    	ret = -1;
    	    		}
    	    		// start time
    	    		if (TRUE == json_object_object_get_ex(recv_obj, "start_time", &key_obj)) {
    	    			// copy start time
    	    			strncpy(tmp_str, json_object_to_json_string(key_obj), 16);
    	    			tmp_str[16] = '\0';
    	    			e->thdr.start_time = string_to_time(tmp_str);
    	    		} else {
    	    	    	log_error("there is no key named \"start_time\".");
    	    	    	ret = -1;
    	    		}
    	    		// end time
    	    		if (TRUE == json_object_object_get_ex(recv_obj, "end_time", &key_obj)) {
    	    			// copy end time
    	    			strncpy(tmp_str, json_object_to_json_string(key_obj), 16);
    	    			tmp_str[16] = '\0';
    	    			e->thdr.end_time = string_to_time(tmp_str);
    	    		} else {
    	    	    	log_error("there is no key named \"end_time\".");
    	    	    	ret = -1;
    	    		}

    	    		if (ret == 0) {
    	    			// init and open task
    	    			task_open(&e->thdr);
    	    		}


    	    	} else if (0 == strncmp(json_object_to_json_string(key_obj), "cancel", 6)) {
    	    		// cancel task
    	    		printf("recv cancel task\n");
    	    		// close task

    	    		// delete task

    	    	}

    	    } else {
    	    	printf("there is no key named \"method\".\n");
    	    }

    	} else if (0 == strncmp(json_object_to_json_string(key_obj), "restart", 7)) {
    		// send reset to mcu

    		// wait a moment
    		sleep(1);
    		// reset linuxs
    		printf("reboot system\n");
    		// system("reboot\n");
    	}
    } else {
    	printf("there is no key named \"cmd\".\n");
    }


	return 0;
}



int send_register_to_epd (struct client_conn * c, uint8_t * device_id, uint8_t * auth_key) {
    int ret = -1;
	struct json_object * new_obj = NULL;
	struct json_object * key_obj = NULL;
	char tmp_str[20];
    char * send_buf;
    char * recv_buf;
//    int tmp;

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
    hex_to_str(tmp_str, device_id, BILINK_PACKECT_SRC_ADDR_SIZE);
    json_object_object_add(new_obj, "device_id", json_object_new_string(tmp_str));
    hex_to_str(tmp_str, auth_key, BILINK_PACKECT_AUTHORIZATION_KEY_SIZE);
    json_object_object_add(new_obj, "auth_key", json_object_new_string(tmp_str));

    send_buf = (char *)json_object_to_json_string(new_obj);

    unix_send_with_recv (c, send_buf, strlen(send_buf), recv_buf, UNIX_DOMAINRECV_BUFFER_SIZE);
    json_object_put(new_obj);
    new_obj = json_tokener_parse(recv_buf);
    if (TRUE == json_object_object_get_ex(new_obj, "status", &key_obj)) {
    	if (0 == strncmp(json_object_to_json_string(key_obj), "ok", 2)) {
    		ret = 0;
    	} else {
    		ret = -1;
    	}
    }
    json_object_put(new_obj);
    free(recv_buf);

	return ret;
}

int send_network_access_to_epd (struct client_conn * c, uint8_t * device_id, uint8_t * data_id, int battery) {
    int ret = -1;
	struct json_object * new_obj = NULL;
    struct json_object * key_obj = NULL;
	char tmp_str[20];
    char * send_buf;
    char * recv_buf;

    if ( (recv_buf = (char *)malloc(UNIX_DOMAINRECV_BUFFER_SIZE) ) == NULL ) {
        printf("malloc recv_buf failed.\n");
        return ret;
    }

    if ((new_obj = json_object_new_object()) == NULL) {
        printf("new json object failed.\n");
        free(recv_buf);
        return ret;
    }
    json_object_object_add(new_obj, "cmd", json_object_new_string("heart"));
    hex_to_str(tmp_str, device_id, BILINK_PACKECT_SRC_ADDR_SIZE);
    json_object_object_add(new_obj, "device_id", json_object_new_string(tmp_str));
    hex_to_str(tmp_str, data_id, BILINK_ACCESS_RESP_SUB_LEN_VERSION);
    json_object_object_add(new_obj, "data_id", json_object_new_string(tmp_str));
    json_object_object_add(new_obj, "battery", json_object_new_int(battery));
    send_buf = (char *)json_object_to_json_string(new_obj);
    json_object_put(new_obj);

    unix_send_with_recv (c, send_buf, strlen(send_buf), recv_buf, UNIX_DOMAINRECV_BUFFER_SIZE);
    new_obj = json_tokener_parse(recv_buf);
    if (TRUE == json_object_object_get_ex(new_obj, "status", &key_obj)) {
    	if (0 == strncmp(json_object_to_json_string(key_obj), "ok", 2)) {
    		ret = 0;
    	} else {
    		ret = -1;
    	}
    }
    json_object_put(new_obj);
    free(recv_buf);
	return ret;
}


int send_update_result_to_epd(void) {

	return 0;
}

int parse_serial_data (struct entry * e) {
	union bilink_packet b;
	struct simple_payload_buf p;
	int size;
	int battery;
	uint8_t type;
	uint8_t length;
	uint8_t index = 0;
	int seq;
	int pkt_num = 0;
	int num = 0;
	// to store a-key
	uint8_t word[2];
	uint8_t msg[20];
	char tmp_str[20];
	uint8_t last_flush_time[4];
	uint8_t str_time[4];
	time_t cur_time;
	size = serial_receive_from_list (&e->bhdr.rts.ots.pld.serial, b.buf, 20);

	if (size > 0) {
		if (b.ctrl.broad) {
			switch (b.ctrl.comm) {
				case BILINK_BEAT_REQ :
					// test , without check
//					if (0 || SRC_INCLUDED != check_execute_list(SQL_TABLE_PATH, b.srcaddr)) {
//						break;
//					}
					authorization_key(word, b.srcaddr, e->bhdr.newkey);
					// check a-key
					if (0 != memcmp(word, b.bc.msg+b.bc.length, BILINK_PACKECT_AUTHORIZATION_KEY_SIZE)) {
						break;
					}
					// read msg
					index = 0;
					while (index < b.bc.length) {
						type = b.bc.msg[index++];
						length = b.bc.msg[index++];
						switch (type) {
							case BILINK_BEAT_REQ_SUB_KEY_POWER :
								battery = b.bc.msg[index];
								break;
							case BILINK_BEAT_REQ_SUB_KEY_VERSION :
								memcpy(msg, b.bc.msg+index, length);
								break;
							default :
								break;
						}
						index += length;
					}
					// check data version in the same
					if (0 == send_network_access_to_epd(&e->chdr, b.srcaddr, msg, battery)) {
						// check execute list ok
						// respond
						p.size = create_unicast_packet(p.payload.data,
								e->bhdr.selfaddr,
								BILINK_ACCESS_REQ,
								b.srcaddr,
								WITH_CONTENT);
						// update time
						uint32_to_array(e->thdr.start_time, str_time);
						p.size = packet_add_content(p.payload.data,
								p.size,
								BILINK_ACCESS_REQ_SUB_KEY_FLUSH_TIME,
								BILINK_ACCESS_REQ_SUB_LEN_FLUSH_TIME,
								(uint8_t *)str_time);
						// cur_time
						time(&cur_time);
						uint32_to_array(cur_time, str_time);
						p.size = packet_add_content(p.payload.data,
								p.size,
								BILINK_ACCESS_REQ_SUB_KEY_UTC_TIME,
								BILINK_ACCESS_REQ_SUB_LEN_UTC_TIME,
								(uint8_t *)str_time);
						// a-key
						p.size = packet_add_key(p.payload.data,
								p.size,
								e->thdr.auth_key);
						// send packet
						bilink_send_data(&e->bhdr, &p);
					} else {
						// check execute list failed
						// respond
						p.size = create_unicast_packet(p.payload.data,
								e->bhdr.selfaddr,
								BILINK_ACCESS_REQ,
								b.srcaddr,
								WITH_CONTENT);
//						// update time
//						uint32_to_array(e->thdr.start_time, str_time);
//						p.size = packet_add_content(p.payload.data,
//								p.size,
//								BILINK_ACCESS_REQ_SUB_KEY_FLUSH_TIME,
//								BILINK_ACCESS_REQ_SUB_LEN_FLUSH_TIME,
//								(uint8_t *)str_time);
//						// cur_time
//						time(&cur_time);
//						uint32_to_array(cur_time, str_time);
//						p.size = packet_add_content(p.payload.data,
//								p.size,
//								BILINK_ACCESS_REQ_SUB_KEY_UTC_TIME,
//								BILINK_ACCESS_REQ_SUB_LEN_UTC_TIME,
//								(uint8_t *)str_time);
						// a-key
						p.size = packet_add_key(p.payload.data,
								p.size,
								e->thdr.auth_key);
						// send packet
						bilink_send_data(&e->bhdr, &p);
					}

					break;
				case BILINK_REGISTER_BEAT_REQ :

//					uint8_t word[2];
					authorization_key(word, b.srcaddr, e->bhdr.initkey);
					if (0 == memcmp(word, b.bc.akey,
							BILINK_PACKECT_AUTHORIZATION_KEY_SIZE)) {

						if (0 != send_register_to_epd (&e->chdr, b.srcaddr, word)) {
							// check ok
							p.size = create_unicast_packet(p.payload.data,
									e->bhdr.selfaddr,
									BILINK_REGISTER_REQ,
									b.srcaddr,
									WITH_CONTENT);
							p.size = packet_add_content(p.payload.data,
									p.size,
									BILINK_REGISTER_REQ_SUB_KEY_NEWKEY,
									BILINK_REGISTER_REQ_SUB_LEN_NEWKEY,
									e->bhdr.newkey);
							p.size = packet_add_autherization_key(p.payload.data,
									p.size,
									e->bhdr.initkey);
							bilink_send_data(&e->bhdr, &p);
						} else {
							// check error
							p.size = create_unicast_packet(p.payload.data,
									e->bhdr.selfaddr,
									BILINK_REGISTER_REQ,
									b.srcaddr,
									WITH_CONTENT);
//							p.size = packet_add_content(p.payload.data,
//									p.size,
//									BILINK_REGISTER_REQ_SUB_KEY_NEWKEY,
//									BILINK_REGISTER_REQ_SUB_LEN_NEWKEY,
//									e->bhdr.newkey);
							p.size = packet_add_autherization_key(p.payload.data,
									p.size,
									e->bhdr.initkey);
							bilink_send_data(&e->bhdr, &p);
						}
					}
					break;
				default  :

					break;
			}
		} else {

			// destination matching
			if (memcmp(b.uc.destaddr, e->bhdr.selfaddr, BILINK_PACKECT_DEST_ADDR_SIZE) != 0)
				return 0;

			// create and match A-key
			authorization_key(word, b.srcaddr, e->bhdr.newkey);
			if (0 != memcmp(word, b.bc.akey, BILINK_PACKECT_AUTHORIZATION_KEY_SIZE))
				return 0;

			// select
			switch (b.ctrl.comm) {
				case BILINK_ACCESS_RESP :
					index = 0;
					while (index < b.uc.length) {
						type = b.uc.msg[index++];
						length = b.uc.msg[index++];
						switch (type) {
//							case BILINK_ACCESS_RESP_SUB_KEY_POWER :
//								battery = b.uc.msg[index];
//								break;
							case BILINK_ACCESS_RESP_SUB_KEY_LAST_FLUSH_TIME :
								memcpy(last_flush_time, b.uc.msg+index, length);
								break;
							case BILINK_ACCESS_RESP_SUB_KEY_VERSION :
								memcpy(msg, b.uc.msg+index, length);
								break;
							default :
								break;
						}
						index += length;
					}
					//
//					send_network_access_to_epd (b.srcaddr, msg, battery);
					break;
				case BILINK_QUERY_RESP :
					if (e->thdr.task_status == TASK_STATUS_LISTEN_ACCESS
						|| e->thdr.task_status == TASK_STATUS_LISTEN_TMP_RESULT ) {

						index = 0;
						while (index < b.uc.length) {
							type = b.uc.msg[index++];
							length = b.uc.msg[index++];
							switch (type) {
								case BILINK_QUERY_RESP_SUB_KEY_LOST_PKT_NUM :
									// big first
									pkt_num = b.uc.msg[index];
									pkt_num <<= 8;
									pkt_num += b.uc.msg[index+1];
									if (pkt_num == 0) {
	//									int idx;
	//									for (idx=0; idx<e->thdr.execlist_len; idx++) {
	//										if (0 == memcpy(e->thdr.execlist_cache+idx*(BILINK_PACKECT_SRC_ADDR_SIZE+1), b.srcaddr, 8)) {
	//											e->thdr.execlist_cache[idx*(BILINK_PACKECT_SRC_ADDR_SIZE+1)+BILINK_PACKECT_SRC_ADDR_SIZE] = 1;
	//											break;
	//										}
	//									}
										write_string_to_file(e->thdr.execlist_ok_path, hex_to_str(tmp_str, b.srcaddr, 8));
									} else {
										write_string_to_file(e->thdr.execlist_failed_path, hex_to_str(tmp_str, b.srcaddr, 8));
									}
									break;
								case BILINK_QUERY_RESP_SUB_KEY_LOST_PKT_SEQ :

									for (num=0; num<pkt_num; num++) {
										seq = b.uc.msg[index+(num<<1)];
										seq <<= 8;
										seq += b.uc.msg[index+(num<<1)+1];
										SET_BIT_OF_INT32_BUF(e->thdr.packet_status, seq);
									}
									break;
								default :
									break;
							}
							index += length;
						}
	//					send_update_schedule_to_server(b.srcaddr);

					}
					break;
				case BILINK_REGISTER_RESP :
//					send_register_success_to_epd (b.srcaddr, e->bhdr.newkey, NULL);
					break;

				default  :
					break;
			}
		}


	} else {

	}
	return 0;
}
