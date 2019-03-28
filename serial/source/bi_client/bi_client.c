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
	while (running) {
		parse_serial_data(c->parent);
	}
	return 0;
}

int client_open (struct client_conn * c) {
	if (c == NULL) {
//		if ((c = (struct client_conn *)malloc(sizeof(struct client_conn))) == NULL) {
//			printf("bilink client open failed\n");
			return -1;
//		} else {
//		    printf("client opened\n");
//		}
	}
	IniReadValue("[client]", "epdSockPath", c->epdSockPath, CONFIG_PATH);
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
//
//{
//
//	char buf[MAX_RECV_LEN];
//	int num = 0;
//	int sock_fd = -1;
//	struct sockaddr_un serv_addr;
//
//	socklen_t serv_addr_len = 0;
//
//	memset(&serv_addr, 0, sizeof(serv_addr));
//
//
//	sock_fd = socket(PF_UNIX,SOCK_STREAM,0);
//	if (sock_fd < 0)
//	{
//		perror("Fail to socket");
//		exit(1);
//	}
//
//    serv_addr.sun_family = AF_UNIX;
//    strcpy(serv_addr.sun_path, UNIX_DOMAIN);
//
//	serv_addr_len = sizeof(serv_addr);
//	if (connect(sock_fd, (struct sockaddr *)&serv_addr, serv_addr_len) < 0)
//	{
//		perror("Fail to connect");
//		exit(1);
//	}
//
//
//	while (running)
//	{
//		num = read(STDIN_FILENO, buf, MAX_RECV_LEN);
//		if (num > 0)
//		{
//			buf[num] = '\0';
//			printf("buf: %s\n", buf);
//			num = write(sock_fd, buf, num);
//			if (num < 0)
//			{
//				printf("write failed\n");
//				exit(1);
//			}
//			if (num > 0) {
//				memset(buf, 0, MAX_RECV_LEN);
//				num = read(sock_fd, buf, MAX_RECV_LEN);
//				buf[num] = '\0';
//				printf("read: %s\n", buf);
//			}
//
//
//			if (strncmp(buf, "exit", strlen("exit")) == 0)
//			{
//				printf("Client exit\n");
//				close(sock_fd);
//				return 0;
//			}
//		}
//	}
//	return 0;
//}
