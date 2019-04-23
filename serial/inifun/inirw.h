#ifndef __INI_RW_H__
#define __INI_RW_H__

//#define READ_STR_OK   0
#define READ_STR_ERR  -1




int IniReadValue(const char* section, const char* key, char* val, const char* file);

//int readStringValue(const char* section, char* key, char* val, const char* file);

int readIntValue(const char* section, const char* key, const char* file);

void IniWriteValue(const char* section, char* key, char* val, const char* file);

int writeStringValue(const char* section, char* key, char* val, const char* file);

int writeIntValue(const char* section, char* key, int val, const char* file);


#endif /*__INI_RW_H__*/
