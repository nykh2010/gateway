/*
 * main.c
 *
 *  Created on: Feb 18, 2019
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

#include "config.h"


/*--------------------------------------------------------------------------------*/
#define DEBUG_PRINTF 1
#if DEBUG_PRINTF
    #define PRINTF(...) printf(__VA_ARGS__)
#else
    #define PRINTF(...)
#endif
/*--------------------------------------------------------------------------------*/

int main (int argc, char * argv[]) {
	int ret;
	log_init(ZLOG_CONF_PATH, ZLOG_CATEGORY);

#if ( EXAMPLE == OTS_EXAMPLE )
	log_info("ots test start.");
	ret = ots_test(argc, argv);
#elif ( EXAMPLE == RTS_EXAMPLE )
	log_info("rts test start.");
	ret = rts_test(argc, argv);
#elif ( EXAMPLE == ENTRY_EXAMPLE )
	log_info("entry test start.");
	ret = entry_test(argc, argv);
#elif ( EXAMPLE == MANAGE_EXAMPLE )
	ret = local_net_manage_task(argc, argv);
#endif
	log_fini();
	return ret;
}



