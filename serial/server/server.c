//server
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <json-c/json.h>
#include <string.h>
#include <pthread.h>

#include "../bilink/bilink.h"
#include "../entry/entry.h"
#include "../config.h"
#include "../task/task.h"
#include "../entrydef.h"
#include "../inifun/inirw.h"

#define DEBUG 1
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#define LOCAL_TEST 0
#if LOCAL_TEST

#endif

int running = 1;



static void * server_thread_exec (void * arg) {

	struct server_conn * s = (struct server_conn *)arg;
	int listen_fd;
	int ret;
	int i;
	static char recv_buf[UNIX_DOMAINRECV_BUFFER_SIZE];
	int client_fd[MAX_CLIENT_NUM];
	socklen_t len;
	struct sockaddr_un clt_addr;
	struct sockaddr_un srv_addr;

	int new_conn_fd = -1;
	int max_fd = -1;
	int num = -1;
	struct timeval timeout;
	fd_set read_set;
	fd_set write_set;
	fd_set select_read_set;

	FD_ZERO(&read_set);
	FD_ZERO(&write_set);
	FD_ZERO(&select_read_set);

	memset(&clt_addr, 0, sizeof(clt_addr));
	memset(&srv_addr, 0, sizeof(srv_addr));

	for (i = 0; i < MAX_CLIENT_NUM; i++) {
		client_fd[i] = -1;
	}

	listen_fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (listen_fd < 0) {
		log_error("connect creat communication socket.");
	} else {
		log_info("connect creat communication socket.");
	}

	// set srv_addr param
	srv_addr.sun_family = AF_UNIX;
	log_debug("server sock path %s", s->serialSockPath );
	strncpy(srv_addr.sun_path, s->serialSockPath, UNIX_DOMAIN_SIZE);

	// delete UNIX_DOMAIN
	unlink(s->serialSockPath);

	// bind sockfd&addr
	ret = bind(listen_fd, (struct sockaddr*)&srv_addr, sizeof(srv_addr));
	if (ret < 0) {
		log_error("cannot bind server socket.");
		close(listen_fd);
		unlink(s->serialSockPath);
		pthread_exit(NULL);
		return NULL;
	} else {
		log_info("bind server socket.");
	}

	// listen sockfd
	ret = listen(listen_fd, 10);
	if (ret < 0) {
		log_error("cannot listen sockfd.");
		close(listen_fd);
		unlink(s->serialSockPath);
		pthread_exit(NULL);
		return NULL;
	} else {
		log_info("listen sockfd.");
	}

	max_fd = listen_fd;
	FD_SET(listen_fd, &read_set);

	while (running)
	{
#if LOCAL_TEST
//		char * test = "{\"cmd\":\"task\",\"method\":\"create\", \"task_id\":\"1111111111111111\",\"start_time\":\"2019-04-08 14:20:20\",\"end_time\":\"2019-04-08 19:20:20\"}";
		FILE * pf;
		pf = fopen("./config/test_task.json", "r");
		if(pf == NULL) {
			log_error("no json.");
		}
		fseek(pf, 0, SEEK_END);
		num = ftell(pf);
		rewind(pf);
		fread(recv_buf, 1, num, pf);
		fclose(pf);
//		log_info("json task : size %d string %s", num, recv_buf);
		recv_buf[num++] = '\0';
		parse_json_cmd(s->parent, recv_buf, num);
#endif
		timeout.tv_sec = 65535000;
		timeout.tv_usec = 0;
		max_fd = listen_fd;
		for (i = 0; i < MAX_CLIENT_NUM; i++) {
			if (max_fd < client_fd[i]) {
				max_fd = client_fd[i];
			}
		}

		select_read_set = read_set;

		ret = select(max_fd + 1, &select_read_set, NULL, NULL, &timeout);

		if (ret == 0) {
			log_debug("server select timeout.");
		}
		else if (ret < 0) {
			log_error("server select error occur.");
		} else {
			if (FD_ISSET(listen_fd, &select_read_set)) {
				log_info("new client comes.");
				len = sizeof(clt_addr);
				new_conn_fd = accept(listen_fd, (struct sockaddr *)(&clt_addr), (socklen_t *)(&len));

				if (new_conn_fd < 0) {
					log_error("cannot accept new client requst.");
					exit(1);
				} else {
					for (i = 0; i < MAX_CLIENT_NUM; i++) {
						if (client_fd[i] == -1) {
							client_fd[i] = new_conn_fd;
							FD_SET(new_conn_fd, &read_set);
							break;
						}
						if (max_fd < new_conn_fd) {
							max_fd = new_conn_fd;
						}
					}
				}
			} else {
				for (i = 0; i < MAX_CLIENT_NUM; i++) {
					if (-1 == client_fd[i]) {
						continue;
					}

					memset(recv_buf, 0, UNIX_DOMAINRECV_BUFFER_SIZE);
					if (FD_ISSET(client_fd[i], &select_read_set))
					{
						num = read(client_fd[i], recv_buf, UNIX_DOMAINRECV_BUFFER_SIZE);

						if (num < 0) {
							log_debug("Client(%d) left.", client_fd[i]);
							FD_CLR(client_fd[i], &read_set);
							close(client_fd[i]);
							client_fd[i] = -1;
						} else if (num > 0) {
							write(client_fd[i], "{\"status\":\"ok\"}", 15);
//							log_debug("Recieve client(%d) data.", client_fd[i]);
							recv_buf[num] = '\0';
							log_debug("Data: %s.", recv_buf);
							parse_json_cmd(s->parent, recv_buf, num);

							close(client_fd[i]);
							client_fd[i] = -1;
						} if (num == 0) {
							log_debug("Client(%d) exit.", client_fd[i]);
							FD_CLR(client_fd[i], &read_set);
							close(client_fd[i]);
							client_fd[i] = -1;
						}
					}
				}
			}
		}
	}

	close(listen_fd);
	unlink(s->serialSockPath);
	pthread_exit(NULL);
	return NULL;
}

int server_open (struct server_conn * s) {
	int ret;
	if (s == NULL) {
		log_error("open server thread failed.");
		return -1;
	}
	ret = IniReadValue("[server]", "serialSockPath", s->serialSockPath, CONFIG_PATH);
	s->serialSockPath[ret] = '\0';
	log_info("size of sock path : %ld, string : %s", strlen(s->serialSockPath), s->serialSockPath);

    // create write thread
    if (pthread_create(&s->thread, NULL, server_thread_exec, (void *)s) != 0) {
        log_error("create server thread failed.");
        return -1;
    }
    log_info("server thread created.");
	return 0;
}

int server_close (struct server_conn * s) {
//	pthread_join(s->thread, NULL);
	pthread_cancel(s->thread);
	log_info("server thread closed.");
	return 0;
}

