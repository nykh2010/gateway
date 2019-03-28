// #define _PARAM_GLOBALS_



#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "../config.h"
#include "../inifun/inirw.h"

#define SECTION_MAX_LEN       256
#define STRVALUE_MAX_LEN      256
#define LINE_CONTENT_MAX_LEN  256

// read value from .ini

//int ini_read_value (const char * file) {
//
//	return 0;
//}



int IniReadValue(const char* section, const char* key, char* val, const char* file)
{
    FILE* fp;
    int ret = READ_STR_ERR;
    int i = 0;
    int lineContentLen = 0;
    int position = 0;
    char lineContent[LINE_CONTENT_MAX_LEN];
    bool bFoundSection = false;
    bool bFoundKey = false;
    fp = fopen(file, "r");
    if(fp == NULL)
    {
        log_error("%s: Opent file %s failed.\n", __FILE__, file);
        return READ_STR_ERR;
    }
    while(feof(fp) == 0)
    {
        memset(lineContent, 0, LINE_CONTENT_MAX_LEN);
        fgets(lineContent, LINE_CONTENT_MAX_LEN, fp);
        if((lineContent[0] == ';') || (lineContent[0] == '\0') || (lineContent[0] == '\r') || (lineContent[0] == '\n'))
        {
            continue;
        }

        //check section
        if(strncmp(lineContent, section, strlen(section)) == 0)
        {
            bFoundSection = true;
            //printf("Found section = %s\n", lineContent);
            while(feof(fp) == 0)
            {
                memset(lineContent, 0, LINE_CONTENT_MAX_LEN);
                fgets(lineContent, LINE_CONTENT_MAX_LEN, fp);
                //check key
                if(strncmp(lineContent, key, strlen(key)) == 0)
                {
                    bFoundKey = true;
                    lineContentLen = strlen(lineContent);
                    //find value
                    for(i = strlen(key); i < lineContentLen; i++)
                    {
                        if(lineContent[i] == '=')
                        {
                            position = i + 1;
                            while (lineContent[position] == ' ') {
                            	position += 1;
                            }
                            break;
                        }
                    }
                    if(i >= lineContentLen)
                    	break;
                    for(i = position; i < lineContentLen; i++)
                    {
                        if((lineContent[i] == '\0') || (lineContent[i] == '\r') || (lineContent[i] == '\n') || (lineContent[i] == ' ') )
                        {
                            val[i] = '\0';
                            break;
                        }
                    }  
//                    strncpy(val, lineContent + position, strlen(lineContent + position));
//                    strncpy(val, lineContent + position, i-position+1);
                    strncpy(val, lineContent + position, i-position+1);
//                    lineContentLen = strlen(val);
                    ret = i-position;

                }
                else if(lineContent[0] == '[') 
                {
                    break;
                }
            }
            break;
        }
    }
    if(!bFoundSection){
    	log_error("No section = %s\n", section);
    }
    else if(!bFoundKey){
    	log_error("No key = %s\n", key);
    }
    fclose(fp);
    return ret;
}

//int readStringValue(const char* section, char* key, char* val, const char* file)
//{
//    char sect[SECTION_MAX_LEN];
//    //printf("section = %s, key = %s, file = %s\n", section, key, file);
//    if (section == NULL || key == NULL || val == NULL || file == NULL)
//    {
//        printf("%s: input parameter(s) is NULL!\n", __func__);
//        return READ_STR_ERR;
//    }
//
//    memset(sect, 0, SECTION_MAX_LEN);
//    sprintf(sect, "[%s]", section);
//    //printf("reading value...\n");
//    IniReadValue(sect, key, val, file);
//
//    return READ_STR_OK;
//}

int readIntValue(const char* section, const char* key, const char* file)
{
    char strValue[STRVALUE_MAX_LEN];
    memset(strValue, '\0', STRVALUE_MAX_LEN);
    if(IniReadValue(section, key, strValue, file) == READ_STR_ERR)
    {
        log_error("%s: error", __func__);
        return 0;
    }
    return (atoi(strValue));
}

void IniWriteValue(const char* section, char* key, char* val, const char* file)
{
    FILE* fp;
    // int i = 0;
    // int n = 0; 
    int err = 0;
    // int lineContentLen = 0;
    // int position = 0;
    char lineContent[LINE_CONTENT_MAX_LEN];
    char strWrite[LINE_CONTENT_MAX_LEN];
    bool bFoundSection = false;
    bool bFoundKey = false;
    
    memset(lineContent, '\0', LINE_CONTENT_MAX_LEN);
    memset(strWrite, '\0', LINE_CONTENT_MAX_LEN);
    // n = sprintf(strWrite, "%s=%s\n", key, val);
    sprintf(strWrite, "%s=%s.", key, val);
    fp = fopen(file, "r+");
    if(fp == NULL)
    {
        printf("%s: Opent file %s failed.", __FILE__, file);
        return;
    }
    while(feof(fp) == 0)
    {
        memset(lineContent, 0, LINE_CONTENT_MAX_LEN);
        fgets(lineContent, LINE_CONTENT_MAX_LEN, fp);
        if((lineContent[0] == ';') || (lineContent[0] == '\0') || (lineContent[0] == '\r') || (lineContent[0] == '\n'))
        {
            continue;
        }
        //check section
        if(strncmp(lineContent, section, strlen(section)) == 0)
        {
            bFoundSection = true;
            while(feof(fp) == 0)
            {
                memset(lineContent, 0, LINE_CONTENT_MAX_LEN);
                fgets(lineContent, LINE_CONTENT_MAX_LEN, fp);
                //check key
                if(strncmp(lineContent, key, strlen(key)) == 0)
                {
                    bFoundKey = true;
                    log_debug("%s: %s=%s.", __func__, key, val);
                    fseek(fp, (0-strlen(lineContent)),SEEK_CUR);
                    err = fputs(strWrite, fp);
                    if(err < 0){
                    	log_error("err.");
                    }
                    break; 
                }
                else if(lineContent[0] == '[') 
                {
                    break;
                }
            }
            break;
        }
    }
    if(!bFoundSection){
    	log_error("No section = %s\n", section);
    }
    else if(!bFoundKey){
    	log_error("No key = %s\n", key);
    }
    fclose(fp);
}

int writeStringValue(const char* section, char* key, char* val, const char* file)
{
    char sect[SECTION_MAX_LEN];
    //printf("section = %s, key = %s, file = %s\n", section, key, file);
    if (section == NULL || key == NULL || val == NULL || file == NULL)
    {
        log_error("%s: input parameter(s) is NULL!\n", __func__);
        return READ_STR_ERR;
    }
    memset(sect, '\0', SECTION_MAX_LEN);
    sprintf(sect, "[%s]", section);
    IniWriteValue(sect, key, val, file);
    return 0;
}

int writeIntValue(const char* section, char* key, int val, const char* file)
{
    char strValue[STRVALUE_MAX_LEN];
    memset(strValue, '\0', STRVALUE_MAX_LEN);
    sprintf(strValue, "%-4d", val);
    
    writeStringValue(section, key, strValue, file);
    return 0;
}
