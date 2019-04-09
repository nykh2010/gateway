#ifndef __SIMPLE_ENTRY_H__
#define __SIMPLE_ENTRY_H__


#include "../timer/timer.h"
#include "../serial/serial.h"
#include "../bilink/bilink.h"
#include "../bilink/bilink_packet.h"
#include "../entrydef.h"
#include "../packet/packet.h"
#include "../payload.h"
#include "../rts/rts.h"
#include "../timer/timer.h"
#include "../bi_client/bi_client.h"
#include "../bi_server/bi_server.h"


extern struct entry tEntry;

//int entry_open (struct entry * e);
int entry_open (void);

void entry_close (struct entry * e);

int entry_send_data (const char * data, int size);

int entry_receive_data (char * data, int msec) ;


#endif /*__SIMPLE_ENTRY_H__*/
