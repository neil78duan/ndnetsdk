/* file : nd_trace.c
 * trace log function 
 *
 * author : neil duan 
 * 2007-9-27 
 * version : 1.0 
 */

#include "nd_common/nd_comcfg.h"
#include "nd_common/nd_os.h"
#include "nd_common/nd_dbg.h"
#include "nd_common/nddir.h"


#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

static char __log_filename[128] ;
static logfunc __log_func =NULL;
static ndatomic_t __log_write_len = 0 ;
static NDUINT32 __log_file_length = -1 ;

void nd_setlog_func(logfunc f) 
{
	__log_func = f ;
}

NDUINT32 nd_setlog_maxsize(NDUINT32 perfile_size)
{
	NDUINT32 oldval = __log_file_length ;
	__log_file_length = perfile_size ;
	return  oldval ;
}

void set_log_file(const char *file_name)
{
	strncpy(__log_filename, file_name,128) ;
}
char *get_log_file() 
{
	if(__log_filename[0]) 
		return __log_filename ;
	else 
		return "ndlog.log" ;
}

void nd_output(const char *text)  
{
	fprintf(stdout,"%s", text) ;
}


#define ND_LOG_FILE get_log_file()

void nd_default_filelog(const char* text)
{
	int size =  0 ;
	const char *logfile_name = ND_LOG_FILE;
	FILE *log_fp = fopen(logfile_name, "a");
	if(!log_fp) {
		return  ;
	}
	size = fprintf(log_fp,"%s", text) ;
	fclose(log_fp) ;

	if(size <= 0) {
		return ;
	}


	nd_atomic_add(&__log_write_len, size) ;
	if ((NDUINT32)__log_write_len >= __log_file_length) {
		char aimFile[1024] ;
		int i =1 ;
		char *p = aimFile ;

		size = snprintf(p, sizeof(aimFile), "%s.", logfile_name) ;
		p += size ;
		do {
			snprintf(p, sizeof(aimFile) - size, "%d", i) ;
			++i ;
		}while(nd_existfile(aimFile)) ;
		nd_renfile(logfile_name,aimFile) ;
	}

	
}

//ndchar_t *strtowcs(char *src, ndchar_t *desc,int len) ;

static const char *_dg_msg_str[] = 
{"Common message: " , ("Debug: "),("Warn : " ), 
("Error : "),("Fatal error : ")} ;

static __INLINE__ const char *log_level_str(int level)
{
	if((level) >=  ND_MSG && (level) <= ND_FATAL_ERR) {
		return _dg_msg_str[level]  ;
	}
	else {
		return " " ;
	}
}	

/* log_msg mutex */
//static nd_mutex 		log_mutex  ;

//得到字符串形式的时间
const char *nd_get_timestr(void)
{
	static __ndthread  char timebuf[64] ;
	time_t nowtm ;
	struct tm *gtm ;

	time(&nowtm) ;
	gtm = localtime( &nowtm );

	snprintf(timebuf, 64, "%d:%d:%d", gtm->tm_hour,
		gtm->tm_min,gtm->tm_sec) ;
	return (const char *)timebuf ;
}
//得到字符串形式的日期
const char *nd_get_datestr(void)
{
	static __ndthread  char datebuf[64] ;
	time_t nowtm ;
	struct tm *gtm ;

	time(&nowtm) ;
	gtm = localtime( &nowtm );

	snprintf(datebuf, 64, "%d-%d-%d", gtm->tm_year+1900,gtm->tm_mon+1,
		gtm->tm_mday) ;
	return (const char *)datebuf ;
}
//得到字符串形式的时间和日期
const char *nd_get_datetimestr(void)
{
	static __ndthread  char timebuf[64] ;
	time_t nowtm ;
	struct tm *gtm ;

	time(&nowtm) ;
	gtm = localtime( &nowtm );

	snprintf(timebuf, 64, "%d-%d-%d %d:%d:%d", 
		gtm->tm_year+1900,gtm->tm_mon+1,gtm->tm_mday,
		gtm->tm_hour,gtm->tm_min,gtm->tm_sec) ;
	return (const char *)timebuf ;
}

const char * _getfilename(const char *filenamePath) 
{
	const char * ret = filenamePath ;
	const char *p = filenamePath ;
	while ( *p ) {
		if (*p == '/' || *p=='\\') {
			ret = p+1 ;
		}
		++p ;
	}
	
	return ret ;
}

int nd_time_day_interval(time_t end_tm, time_t start_tm) 
{
	int start = (int)(start_tm / (3600 * 24)) ;
	int end = (int)(end_tm / (3600 * 24)) ;
	return end - start ;
}

int _logmsg(const char *func, const char *filePath, int line, int level, const char *stm,...) 
{
	char buf[1024*4] ;
	char *p = buf;
	va_list arg;
	int done;
	const char *file = _getfilename(filePath) ;

#ifdef	ND_LOG_WITH_TIME
	p += snprintf(p, 4096,"%s ", nd_get_datetimestr()) ;
#endif

#ifdef	ND_LOG_WITH_SOURCE
	p += snprintf(p, 4096," %s() [%d:%s] ",  func, line, file) ;
#endif
	p += snprintf(p, 4096,"%s:", log_level_str(level)) ;

	va_start (arg, stm);
	done = vsnprintf (p, sizeof(buf),stm, arg);
	va_end (arg);

#ifdef 	ND_OUT_LOG_2CTRL
	if (level==ND_ERROR || level == ND_FATAL_ERR){
		fprintf(stderr, "%s", buf) ;
	}
	else {
		fprintf(stdout, "%s", buf) ;
	}
#endif

	if (__log_func)	{
		__log_func(buf) ;
	}
	else {
#ifdef ND_OUT_LOG_2FILE
		nd_default_filelog(buf) ;
#endif
	}
	return done ;
}

#ifdef ND_FILE_TRACE

#ifdef _ND_COMMON_H_
#error not include nd_common.h
#endif
FILE *nd_fopen_dbg( const char *filename, const char *mode ,const char *file, int line)
{
	FILE *fp = fopen(filename,mode) ;
	_source_log(fp, "fopen ", "file not closed", file, line) ;
	return fp;
}
void nd_fclose_dbg(FILE *fp)
{	
	nd_assert(fp) ;
	fclose(fp) ;
	nd_source_release(fp) ;
}

#endif
