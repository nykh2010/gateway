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
//#include "../client/client.h"
//#include "../server/server.h"

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
	int thread_on;
//	char data_id[17];
	int task_id;
	int data_id;
	char data_path[256];
	char execlist_ok_path[128];
	char execlist_failed_path[128];

	time_t start_time;
	time_t end_time;
	int status;
	uint8_t auth_key[2];
	int packets_interval;
	int packets_num;
	int data_size;
	int task_status;
	int packet_status_size;
	uint32_t * packet_status;
	uint8_t * data;
	struct entry * parent;
};

struct entry {
	struct bilink_conn  bhdr;
	struct server_conn  shdr;
	struct client_conn  chdr;
	struct task_entry   thdr;
	int task_on;
};

#endif /* SOURCE_ENTRYDEF_H_ */
