#pragma once


//Commonly used
char * my_strstr(const char * str1, const char * str2);
int my_stricmp(const char *dst, const char *src);
char * my_strchr(const char *str, int ch);
int my_strnicmp(const char *dst, const char *src, int count);
char * DelSpace(char *szData);
int my_sprintf(char* buf, const char* fmt, ...);
double my_ceil(double x);
void * my_memcpy(void * dst, const void * src, size_t count);
int my_strncmp(const char *first, const char *last, int count);
char * my_substr(char   *str, int   istar, int   iend);
char * my_strncpy(char * dest, const char * source, int count);
