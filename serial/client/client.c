#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/un.h>

#include <json-c/json.h>
#include "../config.h"
#include "../bilink/bilink.h"
#include "../task/task.h"
#include "../inifun/inirw.h"

#define LOCAL_SERIAL_READ_DEBUG 0
#if LOCAL_SERIAL_READ_DEBUG
#include "../timer/timer.h"
#endif

#define UNIX_NET_ACK "ACK"
#define UNIX_NET_ACK_LEN 3
#define MAX_RECV_LEN 1024

#define DEBUG 1
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

 
static int running = 1;
 
int unix_send_without_recv (struct client_conn * c, const char * src, int size) {
	int ret = -1;

//	int num = 0;
	int sock_fd = -1;
	struct sockaddr_un serv_addr;
	socklen_t serv_addr_len = 0;

	memset(&serv_addr, 0, sizeof(serv_addr));

	sock_fd = socket(PF_UNIX, SOCK_STREAM, 0);
	if (sock_fd < 0) {
		log_error("Fail to socket.");
		return ret;
	}

	serv_addr.sun_family = AF_UNIX;
	strcpy(serv_addr.sun_path, c->epdSockPath);

	serv_addr_len = sizeof(serv_addr);
	if (connect(sock_fd, (struct sockaddr *)&serv_addr, serv_addr_len) < 0)
	{
		log_error("Fail to connect.");
		close(sock_fd);
		return ret;
	}
	ret = write(sock_fd, src, size);
	if (ret < 0) {
		log_error("sock write failed.");
		close(sock_fd);
		return ret;
	}
	if (0) {
		char buf[64];
		if (read(sock_fd, buf, 64) > 0 && 0 == strncmp(buf, UNIX_NET_ACK, UNIX_NET_ACK_LEN)) {
			ret = 0;
		}
	}
	close(sock_fd);
	return ret;
}


int unix_send_with_recv (struct client_conn * c, const char * src, int size, char * dest, int max_size) {
	int ret = -1;

	int sock_fd = -1;
	struct sockaddr_un serv_addr;
	socklen_t serv_addr_len = 0;
 
	memset(&serv_addr, 0, sizeof(serv_addr));
 
	sock_fd = socket(PF_UNIX, SOCK_STREAM, 0);
	if (sock_fd < 0) {
		log_error("Fail to socket.");
		return ret;
	}
 
	serv_addr.sun_family = AF_UNIX;
	strcpy(serv_addr.sun_path, c->epdSockPath);

	serv_addr_len = sizeof(serv_addr);
	if (connect(sock_fd, (struct sockaddr *)&serv_addr, serv_addr_len) < 0)
	{
		log_error("Fail to connect.");
		close(sock_fd);
		return ret;
	}
	ret = write(sock_fd, src, size);
	if (ret < 0) {
		log_error("sock write failed.");
		return ret;
	}
	* dest = '\0';
	ret = read(sock_fd, dest, max_size);

	close(sock_fd);
	return ret;
}

static void * client_thread_exec(void * arg) {
	struct client_conn * c = (struct client_conn *)arg;
#if LOCAL_SERIAL_READ_DEBUG
		int tmp;
		serial_packet_entry_t * newEntry = NULL;
		struct serial_handler * handler = &c->parent->bhdr.rts.ots.pld.serial;
		struct simple_payload_buf p;
		p.payload.data[0] = 0x11;
		p.payload.data[1] = 0x22;
		p.payload.data[2] = 0x33;
		p.payload.data[3] = 0x44;
		p.payload.data[4] = 0x55;
		p.payload.data[5] = 0x66;
		p.payload.data[6] = 0x00;
		p.payload.data[7] = 0x01;

		p.payload.data[8] = 0x80;

		p.payload.data[9] = 0x5A;
		p.payload.data[10] = 0xEF;
		p.size = 11;
//		log_info("create data size %d." (p.size));

//		for (tmp=0; tmp<(ptr->ots.pld.rbuf.size-1); tmp++) {
//			printf("%02X ",  ptr->ots.pld.rbuf.payload.data[tmp]);
//		}
//		printf("\n");


	    pthread_mutex_lock(&handler->readDataMut);
		newEntry = (serial_packet_entry_t *)malloc(sizeof(serial_packet_entry_t));

		newEntry->length = p.size;

		newEntry->data = (uint8_t *)malloc(newEntry->length);
		memcpy(newEntry->data, p.payload.data, newEntry->length);
		if (handler->readListLen < handler->readListMaxLen) {
			SIMPLEQ_INSERT_TAIL(&handler->readList, newEntry, next);
			handler->readListLen++;
			pthread_cond_signal(&handler->readDataCond);
		}
	    pthread_mutex_unlock(&handler->readDataMut);
	    log_info("create a data packet.");

		timer_wait_ms(2000);
#endif
	while (running) {


		parse_serial_data(c->parent);
	}
	return 0;
}

int client_open (struct client_conn * c) {
	int ret;
	if (c == NULL) {
		log_error("open client thread failed.");
		return -1;
	}
	ret = IniReadValue("[client]", "epdSockPath", c->epdSockPath, CONFIG_PATH);
	c->epdSockPath[ret] = '\0';
	//_serial_packet_entryeate write thread
    if (pthread_create(&c->thread, NULL, client_thread_exec, (void *)c) != 0) {
        log_error("create client thread failed.");
//        free(c);
        return -1;
    }
    log_info("client thread created.");
	return 0;
}

int client_close (struct client_conn * c) {
//	pthread_join(c->thread, NULL);
	pthread_cancel(c->thread);
//	free(s);
	log_info("client thread closed.");
	return 0;
}

