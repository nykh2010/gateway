/*
 * entrydef.h
 *
 *  Created on: Mar 20, 2019
 *      Author: user
 */

#ifndef SOURCE_ENTRYDEF_H_
#define SOURCE_ENTRYDEF_H_

#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <pthread.h>
#include "config.h"

//#include "../timer/timer.h"
//#include "../serial/serial.h"
//#include "../bilink/bilink.h"
//#include "../bilink/bilink_packet.h"
//#include "../packet/packet.h"
//#include "../payload.h"
//#include "../rts/rts.h"
//#include "../timer/timer.h"
//#include "../bi_client/bi_client.h"
//#include "../bi_server/bi_server.h"

struct entry;

struct server_conn {
	pthread_t thread;
	char serialSockPath[UNIX_DOMAIN_SIZE];
	struct entry * parent;
};

struct client_conn {
	pthread_t thread;
	char epdSockPath[UNIX_DOMAIN_SIZE];
	struct entry * parent;
};

struct task_entry {
	pthread_t thread;
	char task_id[17];
	char data_id[17];
	// char data_url[256];
	char data_path[256];
	char execlist_ok_path[64];
	char execlist_failed_path[64];
//	uint8_t * execlist_cache;
//	int execlist_len;
	// struct tm_utc start_time;
	// struct tm_utc end_time;
	time_t start_time;
	time_t end_time;
	int status;
	uint8_t auth_key[2];
	int packets_num;
	int data_size;
	int task_status;
	int packet_status_size;
	uint32_t * packet_status;
	struct entry * parent;
};

struct entry {
	struct bilink_conn  bhdr;
	struct server_conn  shdr;
	struct client_conn  chdr;
	struct task_entry   thdr;
};

#endif /* SOURCE_ENTRYDEF_H_ */
