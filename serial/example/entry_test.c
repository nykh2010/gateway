#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#include <sys/queue.h>
#include <string.h>

#include "../source/config.h"
#include "../source/payload.h"

#include "../source/serial/serial.h"
#include "../source/packet/packet.h"
#include "../source/timer/timer.h"
#include "../source/rts/rts.h"
#include "../source/entry/entry.h"
/*--------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------*/
#define DEBUG_PRINTF 1
#if DEBUG_PRINTF
    #define PRINTF(...) printf(__VA_ARGS__)
#else
    #define PRINTF(...)
#endif

/*--------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------*/
int entry_test (int argc, char * argv[]) {

    // variables define
    char mode;
    long int count = 0;
    struct timeval now;
    char payload[256];
    int size;

    srand(49);

    if (argc > 0) {
//    	entry_open(argv[1]);

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

#if 1
    for(;;) {
        if (mode == 'T') {
            count++;
            gettimeofday(&now, NULL);

            size = snprintf(payload, 256, "%ld:%ld", count, now.tv_sec);
//            serial_bus_send_data(payload, size);
//            usleep(100000 + rand()%50);
//            // sleep(1);
//            PRINTF("send:%s\n", (payload));

//            timer_wait_ms(10);
            sleep(1);
//            PRINTF("send:%s\n", SIMPLE_PAYLOAD_SEND_DATA_ADDR(payload));
        } else if (mode == 'R') {

            // buffer.size = handler->recv(handler, buffer.buf, 100);
//        	size = serial_bus_receive_data(payload, 20);

             if (size) {
            	 payload[size] = '\0';
                 PRINTF("recv: %s\n", payload);
             }

        }
    }
#endif
    sleep(1);

	return 0;
}
