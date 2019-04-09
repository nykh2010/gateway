#ifndef __BI_SERVER_H__
#define __BI_SERVER_H__


#include <pthread.h>

#include "../entrydef.h"




int local_net_manage_task(int argc, char * argv[]);

int server_open (struct server_conn * e);

int server_close (struct server_conn * e);

#endif /*__BI_SERVER_H__*/
