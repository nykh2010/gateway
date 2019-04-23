#ifndef __BI_CLIENT_H__
#define __BI_CLIENT_H__

#include <pthread.h>
#include "../entrydef.h"


int unix_send_without_recv (struct client_conn * c, const char * src, int size);

int unix_send_with_recv (struct client_conn * c, const char * src, int size, char * dest, int max_size);

int client_open (struct client_conn * c);

int client_close (struct client_conn * c);

#endif /*__BI_CLIENT_H__*/
