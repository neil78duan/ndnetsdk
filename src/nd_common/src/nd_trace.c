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
#include "nd_common/nd_str.h"


#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include "nd_common/nd_time.h"

static char __log_filename[256] ;
static logfunc __log_func =NULL;
static ndatomic_t __log_write_len = 0 ;
static NDUINT32 __log_file_length = -1 ;
static NDUINT8 __close_screen_log = 0 ;

static NDUINT8 __without_file_info = 0;
static NDUINT8 __without_file_time = 0;
static NDUINT8 __witout_date = 0;



int nd_log_no_file(int without_file)
{
	int ret = __without_file_info;
	__without_file_info = without_file;
	return ret;
}
int nd_log_no_time(int without_time)
{
	int ret = __without_file_time;
	__without_file_time = without_time;
	return ret;

}
int nd_log_no_date(int without_date)
{
	int ret = __witout_date;
	__witout_date = without_date;
	return ret;

	
}
logfunc nd_setlog_func(logfunc f)
{
	logfunc ret = __log_func;
	__log_func = f ;
	return ret;
}
void nd_log_close_screen(int flag)
{
	__close_screen_log = flag ? 1 : 0 ;
}

NDUINT32 nd_setlog_maxsize(NDUINT32 perfile_size)
{
	NDUINT32 oldval = __log_file_length ;
	__log_file_length = perfile_size ;
	return  oldval ;
}

void set_log_file(const char *file_name)
{
	strncpy(__log_filename, file_name,sizeof(__log_filename)) ;
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
		
		nd_atomic_set(&__log_write_len, 0) ;
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

int _logmsg_screen(const char *filePath, int line, const char *stm,...) 
{
	char buf[1024*4] ;
	char *p = buf;
	va_list arg;
	int done;
	const char *file ;
	
	if (__close_screen_log) {
		return 0;
	}
	
	file = _getfilename(filePath) ;
		
#ifdef	ND_LOG_WITH_DATE
	p += snprintf(p, 4096 ,"%s ", nd_get_datestr()) ;
#endif

#ifdef	ND_LOG_WITH_TIME
	p += snprintf(p, 4096 - (p - buf), "%s ", nd_get_timestr());
#endif

#ifdef	ND_LOG_WITH_SOURCE
	p += snprintf(p, 4096- (p-buf),"[%d:%s] ",   line, file) ;
#endif
	
	va_start (arg, stm);
	done = vsnprintf (p, sizeof(buf) - (p-buf),stm, arg);
	va_end (arg);
	
	fprintf(stdout, "%s", buf) ;
	
	return done ;
}

int nd_logtext(const char *buf)
{
	int ret = 0;
	
	
#if defined(ND_OUT_LOG_2CTRL)
	if (__close_screen_log == 0) {
		ret = fprintf(stderr, "%s", buf);
	}
#endif

	if (__log_func)	{
		__log_func(buf);
	}
	else {
#ifdef ND_OUT_LOG_2FILE
		nd_default_filelog(buf);
#endif
	}
	
	return ret;
}

int _logmsg(const char *func, const char *filePath, int line, int level, const char *stm,...) 
{
	size_t size;
	char buf[1024*4] ;
	char *p = buf;
	va_list arg;
	int done;
	const char *file = _getfilename(filePath) ;
	
	size = sizeof(buf);
#ifdef ND_LOG_WITH_ND_MARK
	
	strncpy(p, "[nd-log]  ",sizeof(buf)) ;
	p += 9 ;
	size = sizeof(buf) - (p - buf);
	
#endif
	
#ifdef ND_LOG_WITH_DATE
	if (__witout_date == 0) {
		p += snprintf(p, size, "%s ", nd_get_datestr());
		size = sizeof(buf) - (p - buf);
	}
#endif

#ifdef	ND_LOG_WITH_TIME
	if (__without_file_time==0) {
		p += snprintf(p, size, " %s ", nd_get_timestr());
		size = sizeof(buf) - (p - buf);
	}
#endif

#ifdef	ND_LOG_WITH_SOURCE
	if (__without_file_info==0){
		p += snprintf(p, size, " %s() [%d:%s] ", func, line, file);
		size = sizeof(buf) - (p - buf);
	}
#endif
	p += snprintf(p, size,"%s:", log_level_str(level)) ;
	size = sizeof(buf) - (p - buf);

	va_start (arg, stm);
	done = vsnprintf (p, size,stm, arg);
	size -= done;
	va_end (arg);

	nd_logtext(buf);
	return sizeof(buf) - size ;
}

#if defined(ND_FILE_TRACE) && defined(ND_SOURCE_TRACE)

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
