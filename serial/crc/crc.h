#ifndef __CRC_H__
#define __CRC_H__

//#define USE_CRC16_CCITT
//#define USE_CRC16_CCITT_FALSE
//#define USE_CRC16_XMODEM
//#define USE_CRC16_X25
//#define USE_CRC16_MODBUS
#define USE_CRC16_IBM
//#define USE_CRC16_MAXIM
//#define USE_CRC16_USB

unsigned short CRC16Calc(unsigned char *puchMsg, unsigned int usDataLen);



#endif