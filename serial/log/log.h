/*
 * log.h
 *
 *  Created on: Mar 26, 2019
 *      Author: user
 */

#ifndef SOURCE_LOG_LOG_H_
#define SOURCE_LOG_LOG_H_

#include <string.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C"
{
#endif

#include <zlog.h>

#ifdef __cplusplus
}
#endif

extern zlog_category_t * zlog_category;

extern int log_init(const char * conf, const char * category);
extern void log_fini(void);

/* log macros */
#define log_fatal(...) \
	zlog(zlog_category, __FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, \
	ZLOG_LEVEL_FATAL, __VA_ARGS__)
#define log_error(...) \
	zlog(zlog_category, __FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, \
	ZLOG_LEVEL_ERROR, __VA_ARGS__)
#define log_warn(...) \
	zlog(zlog_category, __FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, \
	ZLOG_LEVEL_WARN, __VA_ARGS__)
#define log_notice(...) \
	zlog(zlog_category, __FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, \
	ZLOG_LEVEL_NOTICE, __VA_ARGS__)
#define log_info(...) \
	zlog(zlog_category, __FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, \
	ZLOG_LEVEL_INFO, __VA_ARGS__)
#define log_debug(...) \
	zlog(zlog_category, __FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, \
	ZLOG_LEVEL_DEBUG, __VA_ARGS__)

//#ifndef LOG_DEBUG
//#define LOG_DEBUG log_debug
//#endif
//#ifndef LOG_FATAL
//#define LOG_FATAL log_fatal
//#endif
//#ifndef LOG_ERROR
//#define LOG_ERROR log_error
//#endif
//#ifndef LOG_WARN
//#define LOG_WARN log_warn
//#endif
//#ifndef LOG_NOTICE
//#define LOG_NOTICE log_notice
//#endif
//#ifndef LOG_INFO
//#define LOG_INFO log_info
//#endif





#endif /* SOURCE_LOG_LOG_H_ */
