/*
 * log.c
 *
 *  Created on: 2013-5-7
 *      Author: Fred
 */

#include "g_net_global.h"
#include "log.h"

#define MAX_LOG_LINE_NUM 500000
#define TIME_STAMP_BUF_LEN 128

static FILE *log_file_handle = NULL;
static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;
static int def_log_level = LOG_LEVEL_INDISPENSABLE;
static char last_log_str[LOG_STR_BUF_LEN];
static char cur_ts_str[TIME_STAMP_BUF_LEN];

static char log_file_name[128] = {0};

void set_log_file_name(char *file_name)
{
	sprintf(log_file_name, "%s", file_name);
}

static int log_create_current_time_stamp(void)
{
	struct tm *time_stamp;
	time_t cur_time;

	cur_time = time(NULL);
	time_stamp = localtime(&cur_time);

	snprintf(cur_ts_str, TIME_STAMP_BUF_LEN, "%02d/%02d/%02d %02d:%02d:%02d", time_stamp->tm_mday,
			time_stamp->tm_mon + 1, time_stamp->tm_year % 100, time_stamp->tm_hour, time_stamp->tm_min, time_stamp->tm_sec);
	return 0;
}

int log_init()
{
	log_file_handle = fopen(log_file_name, "w");
	if (log_file_handle == NULL)
	{
		printf("[%s %s %d] Can't create log file.\n", __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}

	memset(last_log_str, 0, LOG_STR_BUF_LEN);

	log_s(LOG_LEVEL_INDISPENSABLE, "LOG START\n");
	return 0;
}

void log_set_level(int level)
{
	def_log_level = level;
}
/*******************************
int printlog(const char *fmt, ...)
{
	char sBuffer[1024] = { 0 };
	va_list argp;
	va_start(argp, fmt); // 将可变长参数转换为va_list 
	// 将va_list传递给子函数 
	int iRLen = vsnprintf(sBuffer, 1024, fmt, argp);
	va_end(argp);

}
*/

char *preamble_logstr[]={
	" ",
	"FATAL",
	"ERROR",
	"WARNING",
	"INFO",};
	
int log_preamble(int level, const char *fmt, ...)
{
		static int count = 0;
		int need_log_str = 0;
		int iRLen = 0;
		char sBuffer[512] = { 0 };
		va_list argp;
	
		if(log_file_handle == NULL ) return -1;
		
		pthread_mutex_lock(&log_mutex);
	
		va_start(argp, fmt); /* 将可变长参数转换为va_list */
		/* 将va_list传递给子函数 */
		iRLen = vsnprintf(sBuffer, 512, fmt, argp);
		va_end(argp);
		log_create_current_time_stamp();
	
		if (++count >= MAX_LOG_LINE_NUM)
		{
			count = 0;
			fclose(log_file_handle);
			log_file_handle = fopen(log_file_name, "w");
		}
	
		// only log the string in case it is different then the last one
		if (strcmp(sBuffer, last_log_str))
		{
			if(level <= def_log_level) {
				
				fprintf(log_file_handle, "[%s][%s] %s", cur_ts_str, preamble_logstr[level], sBuffer);
				memset(last_log_str, 0, LOG_STR_BUF_LEN);
				strcpy(last_log_str, sBuffer);
			}
		}
		fflush(log_file_handle);
		pthread_mutex_unlock(&log_mutex);
		return 0;
}

int log_s(int level, const char *fmt, ...)
{
	static int count = 0;
	int need_log_str = 0;
	int iRLen = 0;
	char sBuffer[512] = { 0 };
	va_list argp;

	if(log_file_handle == NULL ) return -1;
	
	pthread_mutex_lock(&log_mutex);

	va_start(argp, fmt); /* 将可变长参数转换为va_list */
	/* 将va_list传递给子函数 */
	iRLen = vsnprintf(sBuffer, 512, fmt, argp);
	va_end(argp);
	log_create_current_time_stamp();

//	if (++count >= MAX_LOG_LINE_NUM)
//	{
//		count = 0;
//		fclose(log_file_handle);
//		log_file_handle = fopen(log_file_name, "w");
//	}

	// only log the string in case it is different then the last one
	if (strcmp(sBuffer, last_log_str))
	{
		if(level <= def_log_level) {
			
			//fprintf(log_file_handle, "[%s][%s] ", cur_ts_str, preamble_logstr[level]);
			fprintf(log_file_handle, sBuffer);
			memset(last_log_str, 0, LOG_STR_BUF_LEN);
			strcpy(last_log_str, sBuffer);
		}
	}
	fflush(log_file_handle);
	pthread_mutex_unlock(&log_mutex);
	return 0;
}



int log_close(void)
{
	if (log_file_handle != NULL)
	{
		log_s(LOG_LEVEL_INDISPENSABLE, "LOG END\n");
		fclose(log_file_handle);
	}
	return 0;
}

