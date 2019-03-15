#ifndef __CLIENT_H__
#define __CLIENT_H__

int unix_send_without_recv (const char * path, const char * src, int size);

int unix_send_with_recv (const char * path, const char * src, int size, char * dest, int max_size);

#endif
