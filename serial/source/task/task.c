//server
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <json-c/json.h>
#include <string.h>

#include "../config.h"

#define DEBUG 1
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif



int running = 1;

int local_net_manage_task(int argc, char * argv[]) {

//    socklen_t clt_addr_len;
    int listen_fd;
//    int com_fd;
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
        perror("connect creat communication socket");
    } else {
    	PRINTF("connect creat communication socket\n");
    }



    // set srv_addr param
    srv_addr.sun_family = AF_UNIX;
    // strncpy (srv_addr.sun_path, UNIX_DOMAIN, sizeof(srv_addr.sun_path)-1);
//    strncpy (srv_addr.sun_path, UNIX_DOMAIN, sizeof(UNIX_DOMAIN));
    strncpy (srv_addr.sun_path, UNIX_DOMAIN, UNIX_DOMAIN_SIZE);

    // delete UNIX_DOMAIN
    unlink (UNIX_DOMAIN);

    //bind sockfd&addr
    ret = bind(listen_fd, (struct sockaddr*)&srv_addr, sizeof(srv_addr));
    if (ret < 0) {
        perror("cannot bind server socket");
        close(listen_fd);
        unlink(UNIX_DOMAIN);
        return -1;
    } else {
    	PRINTF("bind server socket\n");
    }

    //listen sockfd
    ret = listen(listen_fd, 10);
    if (ret < 0) {
        perror("cannot listen sockfd");
        close(listen_fd);
        unlink(UNIX_DOMAIN);
        return -1;
    } else {
    	PRINTF("listen sockfd\n");
    }

	max_fd = listen_fd;
	FD_SET(listen_fd, &read_set);



///////////////////////////////////////////////////////////////////////////////////

	while (running)
	{
		timeout.tv_sec = SELECT_TIMEOUT_S;
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
			printf("5 sec timeout\n");
		}
		else if (ret < 0) {
			printf("error occur\n");
		} else {
			if (FD_ISSET(listen_fd, &select_read_set)) {
				printf("new client comes\n");
				len = sizeof(clt_addr);
				new_conn_fd = accept(listen_fd, (struct sockaddr *)(&clt_addr), (socklen_t *)(&len));

				if (new_conn_fd < 0) {
			        perror("cannot accept requst");
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
							printf("Client(%d) left\n", client_fd[i]);
							FD_CLR(client_fd[i], &read_set);
							close(client_fd[i]);
							client_fd[i] = -1;
						} else if (num > 0) {
							recv_buf[num] = '\0';
							printf("Recieve client(%d) data\n", client_fd[i]);
							printf("Data: %s\n\n", recv_buf);

							num = write(client_fd[i], recv_buf, num);

							close(client_fd[i]);
							client_fd[i] = -1;
						} if (num == 0) {
							printf("Client(%d) exit\n", client_fd[i]);
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
    unlink(UNIX_DOMAIN);
    return 0;
}
//
//int main (int argc, char * argv[]) {
//	local_net_manage_task(NULL);
//	return 0;
//}

