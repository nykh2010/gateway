 
#include "crc.h"



static void InvertUint8(unsigned char *dBuf,unsigned char *srcBuf)  
{  
        int i;  
        unsigned char tmp[4];  
        tmp[0] = 0;  
        for(i=0;i< 8;i++)  
        {  
          if(srcBuf[0]& (1 << i))  
            tmp[0]|=1<<(7-i);  
        }  
        dBuf[0] = tmp[0];  
          
}  
static void InvertUint16(unsigned short *dBuf,unsigned short *srcBuf)  
{  
        int i;  
        unsigned short tmp[4];  
        tmp[0] = 0;  
        for(i=0;i< 16;i++)  
        {  
          if(srcBuf[0]& (1 << i))  
            tmp[0]|=1<<(15 - i);  
        }  
        dBuf[0] = tmp[0];  
}  
#if defined USE_CRC16_CCITT
static unsigned short CRC16_CCITT(unsigned char *puchMsg, unsigned int usDataLen)  
{  
      unsigned short wCRCin = 0x0000;  
      unsigned short wCPoly = 0x1021;  
      unsigned char wChar = 0;  
        
      while (usDataLen--)     
      {  
            wChar = *(puchMsg++);  
            InvertUint8(&wChar,&wChar);  
            wCRCin ^= (wChar << 8);  
            for(int i = 0;i < 8;i++)  
            {  
              if(wCRCin & 0x8000)  
                wCRCin = (wCRCin << 1) ^ wCPoly;  
              else  
                wCRCin = wCRCin << 1;  
            }  
      }  
      InvertUint16(&wCRCin,&wCRCin);  
      return (wCRCin) ;  
}  
#elif defined USE_CRC16_CCITT_FALSE
static unsigned short CRC16_CCITT_FALSE(unsigned char *puchMsg, unsigned int usDataLen)  
{  
      unsigned short wCRCin = 0xFFFF;  
      unsigned short wCPoly = 0x1021;  
      unsigned char wChar = 0;  
        
      while (usDataLen--)     
      {  
            wChar = *(puchMsg++);  
            wCRCin ^= (wChar << 8);  
            for(int i = 0;i < 8;i++)  
            {  
              if(wCRCin & 0x8000)  
                wCRCin = (wCRCin << 1) ^ wCPoly;  
              else  
                wCRCin = wCRCin << 1;  
            }  
      }  
      return (wCRCin) ;  
} 
#elif defined USE_CRC16_XMODEM
static unsigned short CRC16_XMODEM(unsigned char *puchMsg, unsigned int usDataLen)  
{  
      unsigned short wCRCin = 0x0000;  
      unsigned short wCPoly = 0x1021;  
      unsigned char wChar = 0;  
        
      while (usDataLen--)     
      {  
            wChar = *(puchMsg++);  
            wCRCin ^= (wChar << 8);  
            for(int i = 0;i < 8;i++)  
            {  
              if(wCRCin & 0x8000)  
                wCRCin = (wCRCin << 1) ^ wCPoly;  
              else  
                wCRCin = wCRCin << 1;  
            }  
      }  
      return (wCRCin) ;  
}  
#elif defined USE_CRC16_X25      
static unsigned short CRC16_X25(unsigned char *puchMsg, unsigned int usDataLen)  
{  
      unsigned short wCRCin = 0xFFFF;  
      unsigned short wCPoly = 0x1021;  
      unsigned char wChar = 0;  
        
      while (usDataLen--)     
      {  
            wChar = *(puchMsg++);  
            InvertUint8(&wChar,&wChar);  
            wCRCin ^= (wChar << 8);  
            for(int i = 0;i < 8;i++)  
            {  
              if(wCRCin & 0x8000)  
                wCRCin = (wCRCin << 1) ^ wCPoly;  
              else  
                wCRCin = wCRCin << 1;  
            }  
      }  
      InvertUint16(&wCRCin,&wCRCin);  
      return (wCRCin^0xFFFF) ;  
}  
defined USE_CRC16_MODBUS      
static unsigned short CRC16_MODBUS(unsigned char *puchMsg, unsigned int usDataLen)  
{  
      unsigned short wCRCin = 0xFFFF;  
      unsigned short wCPoly = 0x8005;  
      unsigned char wChar = 0;  
        
      while (usDataLen--)     
      {  
            wChar = *(puchMsg++);  
            InvertUint8(&wChar,&wChar);  
            wCRCin ^= (wChar << 8);  
            for(int i = 0;i < 8;i++)  
            {  
              if(wCRCin & 0x8000)  
                wCRCin = (wCRCin << 1) ^ wCPoly;  
              else  
                wCRCin = wCRCin << 1;  
            }  
      }  
      InvertUint16(&wCRCin,&wCRCin);  
      return (wCRCin) ;  
} 
#elif defined USE_CRC16_IBM
static unsigned short CRC16_IBM(unsigned char *puchMsg, unsigned int usDataLen)  
    {  
      unsigned short wCRCin = 0x0000;  
      unsigned short wCPoly = 0x8005;  
      unsigned char wChar = 0;  
        
      while (usDataLen--)     
      {  
            wChar = *(puchMsg++);  
            InvertUint8(&wChar,&wChar);  
            wCRCin ^= (wChar << 8);  
            for(int i = 0;i < 8;i++)  
            {  
              if(wCRCin & 0x8000)  
                wCRCin = (wCRCin << 1) ^ wCPoly;  
              else  
                wCRCin = wCRCin << 1;  
            }  
      }  
      InvertUint16(&wCRCin,&wCRCin);  
      return (wCRCin) ;  
} 
#elif defined USE_CRC16_MAXIM
static unsigned short CRC16_MAXIM(unsigned char *puchMsg, unsigned int usDataLen)  
{  
      unsigned short wCRCin = 0x0000;  
      unsigned short wCPoly = 0x8005;  
      unsigned char wChar = 0;  
        
      while (usDataLen--)     
      {  
            wChar = *(puchMsg++);  
            InvertUint8(&wChar,&wChar);  
            wCRCin ^= (wChar << 8);  
            for(int i = 0;i < 8;i++)  
            {  
              if(wCRCin & 0x8000)  
                wCRCin = (wCRCin << 1) ^ wCPoly;  
              else  
                wCRCin = wCRCin << 1;  
            }  
      }  
      InvertUint16(&wCRCin,&wCRCin);  
      return (wCRCin^0xFFFF) ;  
}
#elif defined USE_CRC16_USB
static unsigned short CRC16_USB(unsigned char *puchMsg, unsigned int usDataLen)  
{  
      unsigned short wCRCin = 0xFFFF;  
      unsigned short wCPoly = 0x8005;  
      unsigned char wChar = 0;  
        
      while (usDataLen--)     
      {  
            wChar = *(puchMsg++);  
            InvertUint8(&wChar,&wChar);  
            wCRCin ^= (wChar << 8);  
            for(int i = 0;i < 8;i++)  
            {  
              if(wCRCin & 0x8000)  
                wCRCin = (wCRCin << 1) ^ wCPoly;  
              else  
                wCRCin = wCRCin << 1;  
            }  
      }  
      InvertUint16(&wCRCin,&wCRCin);  
      return (wCRCin^0xFFFF) ;  
}  
#endif
unsigned short CRC16Calc(unsigned char *puchMsg, unsigned int usDataLen) 
{
#if defined USE_CRC16_CCITT
  return CRC16_CCITT(puchMsg, usDataLen);
#elif defined USE_CRC16_CCITT_FALSE
  return CRC16_CCITT_FALSE(puchMsg, usDataLen);
#elif defined USE_CRC16_XMODEM
  return CRC16_XMODEM(puchMsg, usDataLen);
#elif defined USE_CRC16_X25
  return CRC16_X25(puchMsg, usDataLen);
#elif defined USE_CRC16_MODBUS
  return CRC16_MODBUS(puchMsg, usDataLen);
#elif defined USE_CRC16_IBM
  return CRC16_IBM(puchMsg, usDataLen);
#elif defined USE_CRC16_MAXIM
  return CRC16_MAXIM(puchMsg, usDataLen);
#elif defined USE_CRC16_USB
  return CRC16_USB(puchMsg, usDataLen);
#else
  #error "CRC_TYPE_NEED_DEFINE"
#endif
}









