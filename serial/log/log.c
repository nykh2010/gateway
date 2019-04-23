/*
 * log.c
 *
 *  Created on: Mar 26, 2019
 *      Author: user
 */
#include "log.h"

zlog_category_t * zlog_category = NULL;

// ===  FUNCTION  ======================================================================
//         Name:  log_init
//  Description:
//  @param category [in]:
// =====================================================================================
int log_init(const char * conf, const char * category) {
	if (zlog_init(conf)) {
		printf("zlog init failed\n");
		return -1;
	}
	zlog_category = zlog_get_category(category);
	if (!zlog_category) {
		printf("zlog get category fail\n");
		zlog_fini();
		return -2;
	}
    return 0 ;
}

// ===  FUNCTION  ======================================================================
//         Name:  log_fini
//  Description:
// =====================================================================================
void log_fini() {
    zlog_fini();
}
