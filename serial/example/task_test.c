/*
 * task_test.c
 *
 *  Created on: Apr 16, 2019
 *      Author: user
 */

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
#include "../source/timer/timer.h"
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
int task_test (int argc, char * argv[]) {
	int idx;
	int ret = -1;
	long tmp1,tmp2,tmp3;
    char mode;
    long int count = 0;
    struct timeval now;
    struct timeval now1;
//    struct timeval now2;
    char payload[256];
    char load[70];
    int size;
    time_t localtime;
    char tmpstr[120];

    srand(49);

    if (argc > 0) {
    	mode = *(argv[1]);
    	if (0 == entry_open()) {
    		log_info("entry successfully opened.");
    	} else {
    		log_error("entry opened failed.");
    		return -1;
    	}
//    	PRINTF("%ld,%d,%ld,", count, size, tmp1);
//        PRINTF("%ld,", tmp2);
//        PRINTF("%ld,", (tmp2<=tmp3) ? (tmp3-tmp2) : (tmp3+10000-tmp2) );
//        PRINTF("%ld\n",tmp3);

    } else {
    	log_notice("open %s need to select mode T/R", argv[0]);
        return -1;
    }


//    log_debug("serial test start...");
    time(&localtime);
    log_info("current time is : %s", time_to_string(tmpstr, localtime));

    if (mode == 'T') {
    	ret = rts_send_ctrl(&tEntry.bhdr.rts, SIMPLE_PAYLOAD_SEND_MODE, 20);
		for (idx=0; idx<256; idx++) {
			payload[idx] = 0x66; //255-idx;
		}
    }
    for(;;) {
        if (mode == 'T') {

            gettimeofday(&now, NULL);
//            count++;

            gettimeofday(&now1, NULL);
            if (ret == 0) {
            	tmp1 = (now.tv_usec);
            	tmp2 = (now1.tv_usec);
            	log_debug("CTRL cnt %ld, time %ld", count, (tmp1<=tmp2) ? (tmp2-tmp1) : (tmp2+1000000-tmp1));
            }
            size = 48;
            for (idx=0; idx<192; idx+=size) {
            	gettimeofday(&now, NULL);
            	count++;

            	memcpy(load+4, payload+idx, size);
//            	memcpy(load, (char *)(&count), 4);
            	load[0] = (char )(count>>24);
            	load[1] = (char )(count>>16);
            	load[2] = (char )(count>>8);
            	load[3] = (char )(count);
                ret = entry_send_data(load, size+4);

                gettimeofday(&now1, NULL);
                if (ret == 0) {
                	tmp1 = (now.tv_usec);
                	tmp2 = (now1.tv_usec);
                	log_debug("DATA size %d cnt %ld, time %ld", size, count, (tmp1<=tmp2) ? (tmp2-tmp1) : (tmp2+1000000-tmp1));
                }

                timer_wait_ms(15);
            }

        } else if (mode == 'R') {
        	sleep(1);
        }
    }

    sleep(1);

	return 0;
}
