/*
 * main.c
 *
 *  Created on: Feb 18, 2019
 *      Author: user
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <sys/queue.h>

#include "./serial/serial.h"

#define DEBUG_PRINTF 1
#if DEBUG_PRINTF
    #define PRINTF(...) printf(__VA_ARGS__)
#else
    #define PRINTF(...)
#endif




int main (int argc, char * argv[]) {
    // variables define
	char mode;
	long int count = 0;
    struct timeval now;
    serial_handler_t serialHandler;
    serial_handler_t * handler = &serialHandler;
    serial_buffer_t buffer;
    char str[256];
    int len;

    buffer.data = (uint8_t *)malloc(256);

    if (argc > 0) {
        handler->port = argv[1];
        if (argc > 1) {
        	mode = *(argv[2]);
        } else {
        	PRINTF("need select mode T/R\n");
        	return -1;
        }
    } else {
    	PRINTF("need device name\n");
    	return -1;
    }
    printf("serial test\n");
    // serial_config(handler);
    serial_start(handler);

    for(;;) {
    	if (mode == 'T') {
    		count++;
        	gettimeofday(&now, NULL);
        	// len = snprintf(str, 256, "time sec %ld, usec %ld\n", now.tv_sec, now.tv_usec);
        	len = snprintf(str, 256, "%ld:%ld", count, now.tv_sec);
            serial_send_to_list(handler, (uint8_t *)str, len);
            msleep(10);
            PRINTF("send:%s\n", str);
    	} else if (mode == 'R') {

            if (0 == serial_receive_from_list (handler, &buffer, 100)) {
            	buffer.data[buffer.size] = '\0';
            	PRINTF("recv: %s\n", buffer.data);
            }

    	}
        // serial_send(handler, (uint8_t *)"abc", 3);
        // sleep(1);
    	// gettimeofday(&now, NULL);
    	// len = snprintf(str, 256, "time sec %ld, usec %ld\n", now.tv_sec, now.tv_usec);
        // serial_send_to_list(handler, (uint8_t *)str, len);
        // msleep(10);
        // serial_receive_from_list (handler, buffer, 10);
    }
    // sleep(2);

    return 0;
}







