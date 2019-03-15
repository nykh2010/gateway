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


#define UNIX_NET_ACK "ACK"
#define UNIX_NET_ACK_LEN 3
#define MAX_RECV_LEN 1024

 
//static int running = 1;
 
int unix_send_without_recv (const char * path, const char * src, int size) {
	int ret = -1;

//	int num = 0;
	int sock_fd = -1;
	struct sockaddr_un serv_addr;
	socklen_t serv_addr_len = 0;

	memset(&serv_addr, 0, sizeof(serv_addr));

	sock_fd = socket(PF_UNIX, SOCK_STREAM, 0);
	if (sock_fd < 0) {
		perror("Fail to socket");
		return ret;
	}

	serv_addr.sun_family = AF_UNIX;
	strcpy(serv_addr.sun_path, path);

	serv_addr_len = sizeof(serv_addr);
	if (connect(sock_fd, (struct sockaddr *)&serv_addr, serv_addr_len) < 0)
	{
		perror("Fail to connect");
		close(sock_fd);
		return ret;
	}
	ret = write(sock_fd, src, size);
	if (ret < 0) {
		printf("write failed\n");
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


int unix_send_with_recv (const char * path, const char * src, int size, char * dest, int max_size) {
	int ret = -1;

	int sock_fd = -1;
	struct sockaddr_un serv_addr;
	socklen_t serv_addr_len = 0;
 
	memset(&serv_addr, 0, sizeof(serv_addr));
 
	sock_fd = socket(PF_UNIX, SOCK_STREAM, 0);
	if (sock_fd < 0) {
		perror("Fail to socket");
		return ret;
	}
 
	serv_addr.sun_family = AF_UNIX;
	strcpy(serv_addr.sun_path, path);

	serv_addr_len = sizeof(serv_addr);
	if (connect(sock_fd, (struct sockaddr *)&serv_addr, serv_addr_len) < 0)
	{
		perror("Fail to connect");
		close(sock_fd);
		return ret;
	}
	ret = write(sock_fd, src, size);
	if (ret < 0) {
		printf("write failed\n");
		return ret;
	}
	* dest = '\0';
	ret = read(sock_fd, dest, max_size);

	close(sock_fd);
	return ret;
}


//
//int client_main(int argc, char *argv[])
//{
//	char buf[MAX_RECV_LEN];
//	int num = 0;
//	int sock_fd = -1;
//	struct sockaddr_un serv_addr;
//	// struct sockaddr_un cli_addr;
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
//		// close(sock_fd);
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
