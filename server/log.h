#ifndef __LOG_H__
#define __LOG_H__

//#define LOG_LEVEL_INDISPENSABLE	(0x01)	//To display several indispensable information.
//#define LOG_LEVEL_FATAL			(0x02)
//#define LOG_LEVEL_ERROR			(0x03)
//#define LOG_LEVEL_WARNING		(0x04)
//#define LOG_LEVEL_INFO			(0x05)

enum{
 LOG_LEVEL_INDISPENSABLE	=(0x01),	//To display several indispensable information.
 LOG_LEVEL_FATAL,			//(0x02)
 LOG_LEVEL_ERROR,			//(0x03)
 LOG_LEVEL_WARNING,			//(0x04)
 LOG_LEVEL_DEBUG,           //(0x05)
 LOG_LEVEL_INFO,			//(0x06)
};

#define LOG_STR_BUF_LEN		512//256

//#define LOG_INFO(a,b)


#define LOG_INFO(lvl, fmt, arg...) \
{\
log_preamble(lvl, "%s %s(%d) ", __FILE__, __FUNCTION__, __LINE__); \
log_s(lvl, fmt, ##arg); \
}


#if 0

#define LOG_INFO(lvl, fmt, arg...)	\
		log_s(lvl, "%s %s(%d) "#fmt, __FILE__, __FUNCTION__, __LINE__, ##arg)

#define LOG_INFO(a, b)	\
	{\
		char log_str[LOG_STR_BUF_LEN];\
		snprintf(log_str, LOG_STR_BUF_LEN, "%s %s(%d) %s", __FILE__, __FUNCTION__, __LINE__, b);\
		log_s(a, log_str);\
	}
#endif

void set_log_file_name(char *file_name);
int log_init();
void log_set_level(int level);
int log_preamble(int level, const char *fmt, ...);
int log_s(int level, const char *fmt, ...);

int log_close(void);

#endif /* LOG_H_ */

