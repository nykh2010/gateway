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



#define DEBUG 1
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

extern struct entry tEntry;

/*--------------------------------------------------------------------------------*/
#define SRC_INCLUDED      0
#define SRC_NOT_INCLUDED  1
#define CHECK_SRC_ERROR   2
//static struct task_entry * task_handler;
/*--------------------------------------------------------------------------------*/
int check_white_list (const char * filename, uint8_t * srcaddr);
int read_bin_data(char * path, uint8_t * data);
void broadcast_block_data (int signo);
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
	  snprintf(zsql, 256, "SELECT * FROM white_list WHERE device_id = %s;", addr);

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
	    perror("sql open");
		sqlite3_free(pDb);
	    return CHECK_SRC_ERROR;
	  }
	  addr_to_string(addr, srcaddr);
	  snprintf(zsql, 256, "SELECT * FROM execute WHERE device_id = %s;", addr);

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
int task_open (struct task_entry * t, const char * sql_path) {
	int ret;
	int nrow=0;
	int ncolumn = 0;
	char **result = NULL;
	char * errmsg;
	char zsql[256];
	char task_id[20];
	sqlite3 * pDb = NULL;
//	if ((t = (struct task_entry *)malloc(sizeof(struct task_entry))) == NULL) {
//		printf("malloc for task_entry failed\n");
//		return -1;
//	}
//	printf("recv create task\n");

	t->task_status = TASK_STATUS_IDLE;
	if ((ret = sqlite3_open(sql_path, &pDb)) == -1) {
	perror("sql open");
		sqlite3_free(pDb);
		return CHECK_SRC_ERROR;
	}
	memcpy(task_id, t->task_id, 16);
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

	memcpy (t->data_id, result[ncolumn+1], 16);
	strcpy (t->data_path, DATA_BIN_PATH);
	t->start_time = string_to_time(result[ncolumn+3]);
	t->end_time = string_to_time(result[ncolumn+4]);
	t->status = atoi(result[ncolumn+5]);
	sqlite3_free_table(result);
	sqlite3_close(pDb);

	t->packets_num = 0;
	t->packet_status = NULL;
	t->status = STATUS_NONE;
	// read bin information
	t->data_size =  read_bin_data(t->data_path, NULL);
	t->packets_num = (t->data_size + BILINK_PACKECT_BROADCAST_MSG_SIZE - 1) / BILINK_PACKECT_BROADCAST_MSG_SIZE;

	t->packet_status_size = (t->packets_num + 31) / 32;

	if ((t->packet_status = (uint32_t *)malloc(t->packet_status_size)) == NULL) {
		printf("malloc memery for packet_status faild\n");
	}

	memset(t->packet_status, 0, t->packet_status_size*4);
	// initialize key
	t->task_status = TASK_STATUS_LISTEN_ACCESS;
	return (nrow > 0 ? SRC_INCLUDED : SRC_NOT_INCLUDED);
}

int task_close (struct task_entry * t) {
	free(t->packet_status);
	return 0;
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
	struct entry * e = &tEntry;
	int index = 0;
	int psize;
	int head_len;
	int count = 0;
	uint8_t * array = NULL;
//	char str[64];
	uint8_t word[2];
	struct simple_payload_buf p;

	if (e->thdr.task_status == TASK_STATUS_LISTEN_ACCESS) {
		e->thdr.task_status = TASK_STATUS_WAIT_SEND;
	} else if (e->thdr.task_status == TASK_STATUS_LISTEN_TMP_RESULT) {
		e->thdr.task_status = TASK_STATUS_WAIT_REISSUE;
	}

	if ((array = (uint8_t *)malloc(e->thdr.data_size)) == NULL) {
		printf("malloc memory for array failed\n");
		return ;
	}
	// get the data into the array buffer
	read_bin_data(e->thdr.data_path, array);

	authorization_key(word, e->bhdr.selfaddr, e->bhdr.newkey);
	// start to send data


	if ( e->thdr.task_status == TASK_STATUS_WAIT_SEND ) {
		// first time send
		e->thdr.task_status = TASK_STATUS_SEND;
		// create control packet, broadcast,
		p.size = create_broadcast_packet (p.payload.data,
										  e->bhdr.selfaddr,
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
		p.size = packet_add_key (p.payload.data, p.size, e->thdr.auth_key);
		// send a packet
		bilink_send_data(&e->bhdr, &p);
		// wait 8ms at least
		timer_wait_ms(8);
		// create data packet
		p.size = create_broadcast_packet (p.payload.data,
										  e->bhdr.selfaddr,
										  BILINK_DATA_REQ,
										  WITH_CONTENT);
		// store head byte size
		head_len = p.size;
		// sequeue count
		count = 0;
		for (index = 0; index < e->thdr.data_size; index += BILINK_PACKECT_BROADCAST_MSG_SIZE) {
			p.size = head_len;
			// data sequece number
			p.payload.data[p.size++] = (uint8_t)((count>>8) & 0x000000FF);
			p.payload.data[p.size++] = (uint8_t)(count & 0x000000FF);
			// data length
			if (e->thdr.data_size > (index + BILINK_PACKECT_BROADCAST_MSG_SIZE)) {
				psize = BILINK_PACKECT_BROADCAST_MSG_SIZE;
			} else {
				psize = e->thdr.data_size - index;
			}
			// copy data
			memcpy(p.payload.data+p.size, array+index, psize);
			// add data length
			p.size += psize;
			// add author key
			p.size = packet_add_key (p.payload.data, p.size, e->thdr.auth_key);
			// send a packet
			bilink_send_data(&e->bhdr, &p);
			// wait 8ms at least
			timer_wait_ms(8);
			count++;
		}
		e->thdr.task_status = TASK_STATUS_SEND_OVER;
		// create query packet
		p.size = create_broadcast_packet (p.payload.data,
				                          e->bhdr.selfaddr,
										  BILINK_QUERY_REQ,
										  WITH_CONTENT);
		// add reissue time
		p.size = packet_add_content (p.payload.data,
									 p.size,
									 BILINK_QUERY_REQ_SUB_KEY_REISSUE_TIME,
									 BILINK_QUERY_REQ_SUB_LEN_REISSUE_TIME,
									 NULL);
		// add author key
		p.size = packet_add_key (p.payload.data, p.size, e->thdr.auth_key);
		// send a packet
		bilink_send_data(&e->bhdr, &p);
		// wait 8ms at least
		e->thdr.task_status = TASK_STATUS_LISTEN_TMP_RESULT;
		// set timer signal
		signal(SIGALRM, broadcast_block_data);
		alarm(1800);

	} else if (e->thdr.task_status == TASK_STATUS_WAIT_REISSUE) {
		// second time send
		e->thdr.task_status = TASK_STATUS_REISSUE;
		// create data packet
		p.size = create_broadcast_packet (p.payload.data,
										  e->bhdr.selfaddr,
										  BILINK_REISSUE_REQ,
										  WITH_CONTENT);
		// store head byte size
		head_len = p.size;
		count = 0;
		for (index = 0; index < e->thdr.data_size; index += BILINK_PACKECT_BROADCAST_MSG_SIZE) {
			if (IS_BIT_OF_INT32_BUF(e->thdr.packet_status, count)) {
				p.size = head_len;
				// data sequece number
				p.payload.data[p.size++] = (uint8_t)((count>>8) & 0x000000FF);
				p.payload.data[p.size++] = (uint8_t)(count & 0x000000FF);
				// data length
				if (e->thdr.data_size > (index + BILINK_PACKECT_BROADCAST_MSG_SIZE)) {
					psize = BILINK_PACKECT_BROADCAST_MSG_SIZE;
				} else {
					psize = e->thdr.data_size - index;
				}
				// copy data
				memcpy(p.payload.data+p.size, array+index, psize);
				// add data length
				p.size += psize;
				// add author key
				p.size = packet_add_key (p.payload.data, p.size, e->thdr.auth_key);
				// send a packet
				bilink_send_data(&e->bhdr, &p);
				// wait 8ms at least
				timer_wait_ms(8);
			}
			count++;
		}
		e->thdr.task_status = TASK_STATUS_REISSUE_OVER;
		// clear status
		memset(e->thdr.packet_status, 0, e->thdr.packet_status_size<<2);
		// create query packet
		p.size = create_broadcast_packet (p.payload.data,
										  e->bhdr.selfaddr,
										  BILINK_QUERY_REQ,
										  WITH_CONTENT);
		// add reissue time
		// p.size = packet_add_content (p.payload.data,
		// 							 p.size,
		// 							 BILINK_QUERY_REQ_SUB_KEY_REISSUE_TIME,
		// 							 BILINK_QUERY_REQ_SUB_LEN_REISSUE_TIME,
		// 							 NULL);
		// add author key
		p.size = packet_add_key (p.payload.data, p.size, e->thdr.auth_key);
		// send a packet
		bilink_send_data(&e->bhdr, &p);
		e->thdr.task_status = TASK_STATUS_LISTEN_RESULT;
	}

	free(array);
}

/*--------------------------------------------------------------------------------*/
void communication_app (void) {


	signal(SIGALRM, broadcast_block_data);

}

/*--------------------------------------------------------------------------------*/
int parse_json_cmd (struct entry * e, const char * str, int size) {
	struct json_object * recv_obj;
	struct json_object * key_obj;
	recv_obj = json_tokener_parse(str);
    if (TRUE == json_object_object_get_ex(recv_obj, "cmd", &key_obj)) {
    	if (0 == strncmp(json_object_to_json_string(key_obj), "task", 4)) {
    	    if (TRUE == json_object_object_get_ex(recv_obj, "method", &key_obj)) {
    	    	if (0 == strncmp(json_object_to_json_string(key_obj), "create", 6)) {
    	    		// creste
    	    		if (TRUE == json_object_object_get_ex(recv_obj, "task_id", &key_obj)) {
    	    			// create task
    	    			time_t now;

    	    			task_open(&e->thdr, SQL_TABLE_PATH);
    	    			e->thdr.parent = e;
    	    			// set timer signal
    	    			signal(SIGALRM, broadcast_block_data);
    	    			time(&now);

    	    			if (e->thdr.start_time > now) {
        	    			alarm(1);
    //    	    			alarm(start_time - now);
    	    			}

    	    			//

    	    		} else {
    	    	    	printf("there is no key named \"task_id\".\n");
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
    		// reset linux
    		printf("reboot system\n");
    		// system("reboot\n");
    	}
    } else {
    	printf("there is no key named \"cmd\".\n");
    }


	return 0;
}



int send_register_success_to_epd (uint8_t * device_id, uint8_t * auth_key, uint8_t * msg) {
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
    hex_to_str(tmp_str, device_id, BILINK_PACKECT_SRC_ADDR_SIZE);
    json_object_object_add(new_obj, "device_id", json_object_new_string(tmp_str));
    tmp = auth_key[0];
    tmp <<= 8;
    tmp += auth_key[1];
    json_object_object_add(new_obj, "auth_key", json_object_new_int(tmp));

    send_buf = (char *)json_object_to_json_string(new_obj);
    unix_send_without_recv (UNIX_DOMAIN_CLIENT, send_buf, strlen(send_buf));

    json_object_put(new_obj);
    free(recv_buf);

	return 0;
}

int send_network_access_to_epd (uint8_t * device_id, uint8_t * data_id, int battery, int status, uint8_t * msg) {
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
    hex_to_str(tmp_str, device_id, BILINK_PACKECT_SRC_ADDR_SIZE);
    json_object_object_add(new_obj, "device_id", json_object_new_string(tmp_str));
    hex_to_str(tmp_str, data_id, BILINK_ACCESS_RESP_SUB_LEN_VERSION);
    json_object_object_add(new_obj, "data_id", json_object_new_string(tmp_str));
    json_object_object_add(new_obj, "battery", json_object_new_int(battery));
    send_buf = (char *)json_object_to_json_string(new_obj);
    unix_send_without_recv (UNIX_DOMAIN_CLIENT, send_buf, strlen(send_buf));

    json_object_put(new_obj);
    free(recv_buf);
	return 0;
}


int send_update_result_to_epd(uint8_t * srcaddr) {

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
	// to store a-key
	uint8_t word[2];
	uint8_t msg[20];
	char tmp_str[20];
//	char tmp_str2[20];
	uint8_t str_time[4];
	time_t cur_time;
	size = serial_receive_from_list (&e->bhdr.rts.ots.pld.serial, b.buf, 20);

	if (size > 0) {
		if (b.ctrl.broad) {
			switch (b.ctrl.comm) {
				case BILINK_BEAT_REQ :
					// test , without check
					if (0 || SRC_INCLUDED != check_execute_list(SQL_TABLE_PATH, b.srcaddr)) {
						break;
					}
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
					// if data version in the same
					if (0 == memcmp(hex_to_str(tmp_str, msg, BILINK_BEAT_REQ_SUB_LEN_VERSION),
									e->thdr.data_id,
									BILINK_BEAT_REQ_SUB_KEY_VERSION) ) {
						break;
					}
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


					break;
				case BILINK_REGISTER_BEAT_REQ :
					if (SRC_INCLUDED == check_white_list(SQL_TABLE_PATH, b.srcaddr)) {
						uint8_t word[2];
						authorization_key(word, b.srcaddr, e->bhdr.initkey);
						if (0 == memcmp(word, b.bc.akey,
								BILINK_PACKECT_AUTHORIZATION_KEY_SIZE)) {
//							struct simple_payload_buf p;
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
					uint8_t last_flush_time[4];
					index = 0;
					while (index < b.uc.length) {
						type = b.uc.msg[index++];
						length = b.uc.msg[index++];
						switch (type) {
							case BILINK_ACCESS_RESP_SUB_KEY_POWER :
								battery = b.uc.msg[index];
								break;
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
					send_network_access_to_epd (b.srcaddr, msg, battery, 0, NULL);
					break;
				case BILINK_QUERY_RESP :
					int seq;
					int pkt_num = 0;
					int num = 0;
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

					break;
				case BILINK_REGISTER_RESP :
					send_register_success_to_epd (b.srcaddr, e->bhdr.newkey, NULL);
					break;

				default  :
					break;
			}
		}


	} else {

	}
	return 0;
}
