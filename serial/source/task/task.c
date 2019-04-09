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

#define LOCAL_DEBUG 1
#if LOCAL_DEBUG

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
int read_bin_size(char * path) {
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

	rewind(fp);

//	log_debug("size of the %s file is %d\n", path, size);

	// close the file
	fclose(fp);
	return size;
}
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

//	log_debug("size of the %s file is %d\n", path, size);
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
	uint8_t temp_array[20];

	t->task_status = TASK_STATUS_LISTEN_ACCESS;
	log_info("task %d status : listen access.", t->task_id);
	// ==============================================================================
	time(&timer);

#if LOCAL_DEBUG
	timer_wait_ms((t->start_time - timer)*1000);
//	timer_wait_ms(1000);
#elif
	timer_wait_ms((t->start_time - timer)*1000);
#endif


	// change status to wait for send
	if (t->task_status == TASK_STATUS_LISTEN_ACCESS) {
		t->task_status = TASK_STATUS_WAIT_SEND;
	} else if (t->task_status == TASK_STATUS_LISTEN_TMP_RESULT) {
		t->task_status = TASK_STATUS_WAIT_REISSUE;
	}
	log_info("task %d status : wait send.", t->task_id);
	// get the data into the array buffer
	if ((t->data_size = read_bin_data(t->data_path, &array)) == 0) {
		log_error("read bin size failed.");
		return NULL;
	}
	// calculate key
	authorization_key(word, t->parent->bhdr.selfaddr, t->parent->bhdr.newkey);
	// start to send data


	if ( t->task_status == TASK_STATUS_WAIT_SEND ) {
		// first time send
		t->task_status = TASK_STATUS_SEND;
		log_info("task %d status : send.", t->task_id);

		if (0 != rts_send_ctrl(&t->parent->bhdr.rts, SIMPLE_PAYLOAD_SEND_MODE, 20)) {
			log_error("Radio change to SEND mode error.");
		}
		// create control packet, broadcast,
		p.size = create_broadcast_packet (p.payload.data,
										  t->parent->bhdr.selfaddr,
										  BILINK_CONTROL_REQ,
										  WITH_CONTENT);
		// add data version
		uint64_to_array((uint64_t)t->data_id, temp_array);
		p.size = packet_add_content (p.payload.data,
									 p.size,
									 BILINK_CONTROL_REQ_SUB_KEY_VERSION,
									 BILINK_CONTROL_REQ_SUB_LEN_VERSION,
									 temp_array);
		// add data packet number
		temp_array[0] = (uint8_t)(t->packets_num >> 8);
		temp_array[1] = (uint8_t)(t->packets_num);
		p.size = packet_add_content (p.payload.data,
									 p.size,
									 BILINK_CONTROL_REQ_SUB_KEY_PKT_NUM,
									 BILINK_CONTROL_REQ_SUB_LEN_PKT_NUM,
									 temp_array);
		// add data next flush time, reissue time
//		uint32_to_array(t->start_time, temp_array);
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
		log_info("task %d status : send over.", t->task_id);

		// create query packet
		p.size = create_broadcast_packet (p.payload.data,
										  t->parent->bhdr.selfaddr,
										  BILINK_QUERY_REQ,
										  WITH_CONTENT);
		// add reissue time
		uint32_to_array((t->end_time>>1) + (t->start_time>>1), temp_array);

		p.size = packet_add_content (p.payload.data,
									 p.size,
									 BILINK_QUERY_REQ_SUB_KEY_REISSUE_TIME,
									 BILINK_QUERY_REQ_SUB_LEN_REISSUE_TIME,
									 temp_array);
		// add author key
		p.size = packet_add_key (p.payload.data, p.size, t->auth_key);
		// send a packet
		bilink_send_data(&t->parent->bhdr, &p);
		// wait 8ms at least
		t->task_status = TASK_STATUS_LISTEN_TMP_RESULT;
		log_info("task %d status : listen temp result.", t->task_id);

		if (0 != rts_send_ctrl(&t->parent->bhdr.rts, SIMPLE_PAYLOAD_RECV_MODE, 20)) {
			log_error("Radio change to RECV mode error.");
		}
		// ==============================================================================
		// set timer signal
		time(&timer);
#if LOCAL_DEBUG
		timer_wait_ms(((t->end_time>>1) - (t->start_time>>1))*1000);
#elif
		timer_wait_ms(((t->end_time>>1) + (t->start_time>>1) - timer)*1000);
#endif

	}
	t->task_status = TASK_STATUS_WAIT_REISSUE;
	if (t->task_status == TASK_STATUS_WAIT_REISSUE) {
		// second time send
		t->task_status = TASK_STATUS_REISSUE;
		log_info("task %d status : reissue.", t->task_id);

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
		log_info("task %d status : reissue over.", t->task_id);

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
		log_info("task %d status : listen result.", t->task_id);

		if (0 != rts_send_ctrl(&t->parent->bhdr.rts, SIMPLE_PAYLOAD_RECV_MODE, 20)) {
			log_error("Radio change to RECV mode error.");
		}
		//==================================================================================
		time(&timer);
#if LOCAL_DEBUG
		timer_wait_ms(((t->end_time>>1) - (t->start_time>>1))*1000);
#elif
		timer_wait_ms((t->end_time - timer)*1000);
#endif
	}
	free(array);
	t->task_status = TASK_STATUS_FINISH;
	log_info("task %d status : finish.", t->task_id);

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
	int tmp;
	tmp = IniReadValue("[task]", "binFilePath", t->data_path, CONFIG_PATH);
	t->data_path[tmp] = '\0';
	// creat tmp exec list
	tmp = IniReadValue("[task]", "ExecOkListPath", t->execlist_ok_path, CONFIG_PATH);
	t->execlist_ok_path[tmp] = '\0';
	tmp = IniReadValue("[task]", "ExecFailedListPath", t->execlist_failed_path, CONFIG_PATH);
	t->execlist_failed_path[tmp] = '\0';

	// read bin data size
	t->data_size = read_bin_size(t->data_path);
	log_info("size of bin is %d", t->data_size );
	// init packet number, calculate packet number
	t->packets_num = (t->data_size + BILINK_PACKECT_BROADCAST_MSG_SIZE - 1) / BILINK_PACKECT_BROADCAST_MSG_SIZE;

	// init packet status array
	t->packet_status_size = (t->packets_num + 31) / 32;
	if (t->packet_status != NULL) {
		free(t->packet_status);
	}
	if ((t->packet_status = (uint32_t *)malloc((t->packet_status_size<<2))) == NULL) {
		log_error("malloc memery for packet_status faild.");
		ret = -1;
	}
	memset(t->packet_status, 0, t->packet_status_size*4);

	t->task_status = TASK_STATUS_LISTEN_ACCESS;
	// create thread
	if (pthread_create(&t->thread, NULL, task_send_thread_exec, (void *)t) != 0) {
		log_error("create task thread failed\n");
		return -1;
	} else {
		log_info("create task thread successfully.");
	}

	return ret;
}
/*--------------------------------------------------------------------------------*/
int task_close (struct task_entry * t) {
	free(t->packet_status);
	return 0;
}


/*--------------------------------------------------------------------------------*/
int parse_json_cmd (struct entry * e, const char * str, int size) {
	int ret = 0;
	struct json_object * recv_obj;
	struct json_object * key_obj;
	char tmp_str[20];
	recv_obj = json_tokener_parse(str);
//	log_debug("recv json : %s", str);
	log_debug("recv json cmd from web server.");

    if (TRUE == json_object_object_get_ex(recv_obj, "cmd", &key_obj)) {
//    	log_debug("cmd   : %s.", json_object_get_string(key_obj));

    	if (0 == strncmp(json_object_get_string(key_obj), "task", 4)) {
    		log_debug("cmd        : %s", json_object_get_string(key_obj));

    	    if (TRUE == json_object_object_get_ex(recv_obj, "method", &key_obj)) {
//    	    	log_debug("parse find method.");

    	    	if (0 == strncmp(json_object_get_string(key_obj), "create", 6)) {
    	    		log_debug("method     : %s", json_object_get_string(key_obj));
	    			// clear task
	    			memset(&e->thdr, 0, sizeof(struct task_entry));
	    			// register parent
	    			e->thdr.parent = e;
	    			//
    	    		if (TRUE == json_object_object_get_ex(recv_obj, "task_id", &key_obj)) {
    	    			// copy task id
//    	    			strncpy(e->thdr.task_id, json_object_get_string(key_obj), 16);
    	    			e->thdr.task_id = json_object_get_int(key_obj);
    	    			log_debug("task_id    : %d", e->thdr.task_id);
    	    		} else {
    	    	    	log_error("no key named \"task_id\".");
    	    	    	ret = -1;
    	    		}
    	    		// start time
    	    		if (TRUE == json_object_object_get_ex(recv_obj, "start_time", &key_obj)) {
    	    			// copy start time
    	    			strncpy(tmp_str, json_object_get_string(key_obj), 16);
    	    			tmp_str[16] = '\0';
    	    			e->thdr.start_time = string_to_time(tmp_str);
    	    			log_debug("start_time : %s", json_object_get_string(key_obj));
    	    		} else {
    	    	    	log_error("there is no key named \"start_time\".");
    	    	    	ret = -1;
    	    		}
    	    		// end time
    	    		if (TRUE == json_object_object_get_ex(recv_obj, "end_time", &key_obj)) {
    	    			// copy end time
    	    			strncpy(tmp_str, json_object_get_string(key_obj), 16);
    	    			tmp_str[16] = '\0';
    	    			e->thdr.end_time = string_to_time(tmp_str);
    	    			log_debug("end_time   : %s.", json_object_get_string(key_obj));
    	    		} else {
    	    	    	log_error("no key named \"end_time\".");
    	    	    	ret = -1;
    	    		}
    	    		// data version or data_id
    	    		if (TRUE == json_object_object_get_ex(recv_obj, "data_id", &key_obj)) {
    	    			// copy end time
    	    			e->thdr.data_id = json_object_get_int(key_obj);
    	    			log_debug("data_id    : %d.", e->thdr.data_id );
    	    		} else {
    	    	    	log_error("no key named \"data_id\".");
    	    	    	ret = -1;
    	    		}

    	    		if (ret == 0) {
    	    			// init and open task
    	    			log_info("recv start from epd server.");
    	    			task_open(&e->thdr);
    	    		}


    	    	} else if (0 == strncmp(json_object_get_string(key_obj), "cancel", 6)) {
    	    		// cancel task
    	    		log_info("recv cancel task\n");
    	    		// close task

    	    		// delete task

    	    	}

    	    } else {
    	    	printf("there is no key named \"method\".\n");
    	    }

    	} else if (0 == strncmp(json_object_get_string(key_obj), "restart", 7)) {
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
        log_error("malloc recv_buf failed.\n");
        return -1;
    }

    if ((new_obj = json_object_new_object()) == NULL) {
    	log_error("new json object failed.\n");
        free(recv_buf);
        return -1;
    }
    json_object_object_add(new_obj, "cmd", json_object_new_string("register"));
    hex_to_str(tmp_str, device_id, BILINK_PACKECT_SRC_ADDR_SIZE);
    json_object_object_add(new_obj, "device_id", json_object_new_string(tmp_str));
    hex_to_str(tmp_str, auth_key, BILINK_PACKECT_AUTHORIZATION_KEY_SIZE);
    json_object_object_add(new_obj, "auth_key", json_object_new_string(tmp_str));

    send_buf = (char *)json_object_to_json_string(new_obj);
    log_debug("send json : %s", send_buf);
    if ( unix_send_with_recv (c, send_buf, strlen(send_buf), recv_buf, UNIX_DOMAINRECV_BUFFER_SIZE) < 0) {
    	log_error("have not receive respond from epd server.");
    	return -1;
    }

    json_object_put(new_obj);
    new_obj = json_tokener_parse(recv_buf);
    log_debug("recv json : %s", recv_buf);

    if (TRUE == json_object_object_get_ex(new_obj, "status", &key_obj)) {
    	if (0 == strncmp(json_object_to_json_string(key_obj), "ok", 2)) {
    		log_debug("recv ok.");
    		ret = 0;
    	} else {
    		log_error("have not recv ok.");
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
    json_object_object_add(new_obj, "data_id", json_object_new_int((int)array_to_uint64(data_id)));
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
	char tmp_str[64];
	uint8_t last_flush_time[4];
	uint8_t str_time[4];
	time_t cur_time;
	size = serial_receive_from_list (&e->bhdr.rts.ots.pld.serial, b.buf, 20);

	if (size > 0) {
		log_debug("get a new data packet.");
		if (b.ctrl.broad) {
			switch (b.ctrl.comm) {
				case BILINK_BEAT_REQ :
					// test , without check
//					if (0 || SRC_INCLUDED != check_execute_list(SQL_TABLE_PATH, b.srcaddr)) {
//						break;
//					}
					log_debug("get a new beat req.");
					authorization_key(word, b.srcaddr, e->bhdr.newkey);
					// check a-key
					if ( 0 && 0 != memcmp(word, b.bc.msg+b.bc.length, BILINK_PACKECT_AUTHORIZATION_KEY_SIZE)) {
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
					if (1 || 0 == send_network_access_to_epd(&e->chdr, b.srcaddr, msg, battery)) {
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
						log_info("send beat request.");
						// ==========================================================
						for (length=0; length<p.size; length++) {
							printf("%02X ",  p.payload.data[length]);
						}
						printf("\n");
						// ==========================================================
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
					log_debug("get a new register beat req.");
					authorization_key(word, b.srcaddr, e->bhdr.initkey);
					if (1 || 0 == memcmp(word, b.bc.akey,
							BILINK_PACKECT_AUTHORIZATION_KEY_SIZE)) {
						//==========================================================
						log_info("source addr : %s", hex_to_str(tmp_str, b.srcaddr, 8));
						log_info("self addr : %s", hex_to_str(tmp_str, e->bhdr.selfaddr, 8));
						//
						if (1 || 0 != send_register_to_epd (&e->chdr, b.srcaddr, word)) {
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
							log_info("send register requst.");
							// ==========================================================
							for (length=0; length<p.size; length++) {
								printf("%02X ",  p.payload.data[length]);
							}
							printf("\n");
							// ==========================================================
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
							log_info("send register refused.");

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
							default :
								break;
						}
						index += length;
					}
					log_debug("recv access respond.");
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
									pkt_num = b.uc.msg[index];log_info("send register request.");
									pkt_num <<= 8;
									pkt_num += b.uc.msg[index+1];
									if (pkt_num == 0) {
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
					log_info("recv register respond.");
					break;

				default  :
					break;
			}
		}


	} else {

	}
	return 0;
}
