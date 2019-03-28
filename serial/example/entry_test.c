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
	int idx;
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
    	if (0 == entry_open()) {
    		log_info("entry successfully opened.");
    	} else {
    		log_error("entry opened failed.");
    		return -1;
    	}

    } else {
    	log_notice("open %s need to select mode T/R", argv[0]);
        return -1;
    }
#else
    mode = 'T';
    entry_open();
#endif
    log_debug("serial test start...");

#if 1
    for(;count<10000;) {
        if (mode == 'T') {
            count++;
            gettimeofday(&now, NULL);
            usleep(8000 + rand()%50);  //8000
            for (idx=0; idx<256; idx++) {
            	payload[idx] = idx;
            }
//            size = snprintf(payload, 256, "%5ld:%4ld 11111111112222222222333333333344444444445555555555", count, now.tv_usec/100);
//            size = snprintf(payload, 256, "%5ld:%4ld", count, now.tv_usec/100);
//            size = snprintf(payload, 256, "T");
            gettimeofday(&now1, NULL);
            rts_send_ctrl(&tEntry.bhdr.rts, SIMPLE_PAYLOAD_RESE_MODE, 20);
            for (idx=0; idx<256; idx+=32) {
            	log_debug("send %d bytes.", 32);
                entry_send_data(payload+idx, 32);
//                size = 32;
                timer_wait_ms(10);
            }

//            ret = entry_send_data(payload, size);
            gettimeofday(&now2, NULL);
            if (ret == 0) {
            	tmp1 = (int) (now.tv_usec/100);
            	tmp2 = (int) (now1.tv_usec/100);
            	tmp3 = (int) (now2.tv_usec/100);
//                PRINTF("%ld,%d,%d,", count, size, tmp1);
//                PRINTF("%d,", tmp2);
//                PRINTF("%d,", (tmp2<=tmp3) ? (tmp3-tmp2) : (tmp3+10000-tmp2) );
//                PRINTF("%d\n",tmp3);
            }

//            // sleep(1);
//            log_debug("send %d : %s.", size, (payload));

            timer_wait_ms(10);
            sleep(1);

        } else if (mode == 'R') {
//        	int tmp;
        	sleep(1);
//			for(tmp=0; tmp<200; tmp++ ){
//				        if (0 == rts_send_ctrl(&tEntry.bhdr.rts, SIMPLE_PAYLOAD_STATUS, 500)) {
//				        	log_info("got MCU status.");
//				        	if (0 != rts_send_ctrl(&tEntry.bhdr.rts, SIMPLE_PAYLOAD_RESE_MODE, 500)) {
//				        		log_error("Radio change to RESE mode error.");
//				        	}
//				        } else {
//				        	log_error("The MCU status is not OK.");
//				//        	return -1;
//				        }
//						rts_send_ctrl(&tEntry.bhdr.rts, 0x04, 200);
//				        sleep(1);
//			}
            // buffer.size = handler->recv(handler, buffer.buf, 100);
//        	size = entry_receive_data(payload, 20);
//
//             if (size) {
//            	 payload[size] = '\0';
//                 log_debug("recv: %s.", payload);
//             }

        }
    }
#endif
    sleep(1);

	return 0;
}
