#ifndef __SIMPLE_ENTRY_H__
#define __SIMPLE_ENTRY_H__


#include "../timer/timer.h"
#include "../serialp/serial.h"
#include "../bilink/bilink.h"
#include "../bilink/bilink_packet.h"
#include "../entrydef.h"
#include "../packet/packet.h"
#include "../payload.h"
#include "../rts/rts.h"
#include "../timer/timer.h"
#include "../client/client.h"
#include "../server/server.h"


extern struct entry tEntry;

//int entry_open (struct entry * e);
int entry_open (void);

void entry_close (struct entry * e);

int entry_send_data (const char * data, int size);

int entry_receive_data (char * data, int msec) ;


#endif /*__SIMPLE_ENTRY_H__*/
