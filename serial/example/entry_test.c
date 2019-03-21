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
	int ret = -1;
	int tmp1,tmp2,tmp3;
    char mode;
    long int count = 0;
    struct timeval now;
    struct timeval now1;
    struct timeval now2;
    char payload[256];
    int size;

    srand(49);
#if 1
    if (argc > 0) {
    	mode = *(argv[1]);
    	entry_open();
    } else {
    	PRINTF("need select mode T/R\n");
        return -1;
    }
#else
    mode = 'T';
    entry_open();
#endif
    printf("serial test\n");
//    count,before,after,end
//    1,2,3,4
//    6,7,8,9
#if 1
    for(;count<10000;) {
        if (mode == 'T') {
            count++;
            gettimeofday(&now, NULL);
            usleep(8000 + rand()%50);  //8000
            size = snprintf(payload, 256, "%5ld:%4ld 11111111112222222222333333333344444444445555555555", count, now.tv_usec/100);
            gettimeofday(&now1, NULL);

            ret = entry_send_data(payload, size);

            gettimeofday(&now2, NULL);

            if (ret == 0) {
            	tmp1 = (int) (now.tv_usec/100);
            	tmp2 = (int) (now1.tv_usec/100);
            	tmp3 = (int) (now2.tv_usec/100);
                PRINTF("%ld,%d,%d,", count, size, tmp1);
                PRINTF("%d,", tmp2);
                PRINTF("%d,", (tmp2<=tmp3) ? (tmp3-tmp2) : (tmp3+10000-tmp2) );
                PRINTF("%d\n",tmp3);
            }

//            // sleep(1);
            PRINTF("send:%s\n", (payload));

//            timer_wait_ms(10);
            sleep(1);

        } else if (mode == 'R') {

            // buffer.size = handler->recv(handler, buffer.buf, 100);
        	size = entry_receive_data(payload, 20);

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
