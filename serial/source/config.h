#ifndef __CONFIG_H__
#define __CONFIG_H__

/*--------------------------------------------------------------------------------*/

#define OTS_EXAMPLE        2 //
#define RTS_EXAMPLE        3 //
#define ENTRY_EXAMPLE      4 //
#define MANAGE_EXAMPLE     5 //
#define EXAMPLE  ENTRY_EXAMPLE

#if ( EXAMPLE == OTS_EXAMPLE )
#include "../example/ots_test.h"
#elif ( EXAMPLE == RTS_EXAMPLE )
#include "../example/rts_test.h"
#elif ( EXAMPLE == ENTRY_EXAMPLE )
#include "../example/entry_test.h"
#elif ( EXAMPLE == MANAGE_EXAMPLE )
#include "task/task.h"
#endif

/*--------------------------------------------------------------------------------*/


#ifndef SERIAL_CONFIG_PATH
//#define SERIAL_CONFIG_PATH  "/home/user/eclipse-workspace/serialapp/Debug/config/serial.ini"
#define SERIAL_CONFIG_PATH  "./config/serial.ini"
#endif

#define CONFIG_SECTION_SERIAL          "[serial]"
#define CONFIG_SECTION_BILINK          "[bilink]"
#define CONFIG_SECTION_UNIXNET         "[unixnet]"
#define CONFIG_SECTION_RADIO           "[radio]"
#define CONFIG_SECTION_NETWORK         "[network]"

// key of section [serial]
#define CONFIG_KEY_DEVICE              "device"
#define CONFIG_KEY_BAUDRATE            "baudrate"
#define CONFIG_KEY_PARITY              "parity"
#define CONFIG_KEY_DATABITS            "databits"
#define CONFIG_KEY_STOPBITS            "stopbits"
#define CONFIG_KEY_READ_LIST_MAX_LEN   "readListMaxLen"
#define CONFIG_KEY_WRITE_LIST_MAX_LEN  "writeListMaxLen"
// value of section [serial]
#define CONFIG_DEFAULT_VAL_DEVICE              "/dev/ttyS0"
#define CONFIG_DEFAULT_VAL_BAUDRATE            115200
#define CONFIG_DEFAULT_VAL_PARITY              'N'
#define CONFIG_DEFAULT_VAL_DATABITS            8
#define CONFIG_DEFAULT_VAL_STOPBITS            1
#define CONFIG_DEFAULT_VAL_READ_LIST_MAX_LEN   30
#define CONFIG_DEFAULT_VAL_WRITE_LIST_MAX_LEN  30

//  unix socket config
#define UNIX_DOMAIN "/var/run/gateway/serialserver.sock"
//#define UNIX_DOMAIN "./serialserver.sock"
#define UNIX_DOMAIN_SIZE 64
#define UNIX_DOMAINRECV_BUFFER_SIZE   1024
#define MAX_CLIENT_NUM  10

#define UNIX_DOMAIN_CLIENT "/var/run/gateway/epdserver.sock"
#define SELECT_TIMEOUT_S 5

#define SQL_TABLE_PATH  "/var/run/gateway/sqlite3/sqlite3.sock"
#define DATA_BIN_PATH  "/var/run/gateway/test_bin"
//
#define RTS_BUSY_TIME_MS  20
#define RTS_ACK_TIME_MS   20

#endif
