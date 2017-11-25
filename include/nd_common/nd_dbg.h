/* file : nd_dbg.h
 * define function used in debug 
 *
 * version 1.0
 * neil duan
 * all right reserved 2007 by neil!
 * 2007-10-5 
 */

#ifndef _NDDBG_H_
#define _NDDBG_H_
#include "nd_common/nd_comcfg.h"
#include "nd_common/ndchar.h"

typedef int (*nd_out_func)(FILE *outfile,const char *stm,...) ;
/* 
pro compile out put message 
   #pragma ndpreMSG(need to do someting)

it outputs a line like this:

  x:\yourdirect\xx.cpp:need to do someting

*/
#define ndpre_str2(x)	   #x
#define ndpre_str(x)	ndpre_str2(x)
#define ndpre_msg(desc) message(__FILE__ "(" ndpre_str(__LINE__) "):" #desc)

typedef enum _error_ID{
	ND_MSG	= 0,			//正常信息
	ND_MSGDEBUG	,			//记录调试信息
	ND_WARN	,				//警告信息
	ND_ERROR ,				//错误信息
	ND_FATAL_ERR 			//严重错误
}edg_ID ;

typedef int (*nd_log_entry)(const char* text) ;

ND_COMMON_API nd_log_entry nd_setlog_func(nd_log_entry f);
ND_COMMON_API int nd_log_no_file(int without_file);
ND_COMMON_API int nd_log_no_time(int without_time);
ND_COMMON_API int nd_log_no_date(int without_date);
ND_COMMON_API void nd_log_close_screen(int flag);
ND_COMMON_API void set_log_file(const char *file_name)  ;
#define nd_log_set_file set_log_file
ND_COMMON_API int nd_output(const char *text)  ;	//screen output
ND_COMMON_API int nd_default_filelog(const char* text) ; //default log function , write to *.log file
ND_COMMON_API NDUINT32 nd_setlog_maxsize(NDUINT32 perfile_size); // set log file size , if write-len > size , generate a *.log.1 ...
ND_COMMON_API int nd_set_logpath_without_date(int without_date); //set log files with data 


//log file operate

ND_COMMON_API int nd_logtext(const char *text);//nd module call this function when write log
ND_COMMON_API int _logmsg_screen(const char *filePath, int line, const char *stm, ...) ;
ND_COMMON_API int _logmsg(const char *func, const char *file, int line, int level, const char *stm, ...) ;

ND_COMMON_API void nd_log_open(edg_ID logType);
ND_COMMON_API void nd_log_close(edg_ID logType);
ND_COMMON_API int nd_log_check(edg_ID logType);


#define AND ,

//////////////////////////////////////////////////////////////////////////
#if defined(__ND_ANDROID__)
#include <android/log.h>
#define NDLOG_TAG    "ndnet-log"
#define nd_logdebug(...)  __android_log_print(ANDROID_LOG_DEBUG,NDLOG_TAG,__VA_ARGS__)
#define nd_logmsg(...)  __android_log_print(ANDROID_LOG_INFO,NDLOG_TAG,__VA_ARGS__)
#define nd_logwarn(...)  __android_log_print(ANDROID_LOG_WARN,NDLOG_TAG,__VA_ARGS__)
#define nd_logerror(...)  __android_log_print(ANDROID_LOG_ERROR,NDLOG_TAG,__VA_ARGS__)
#define nd_logfatal(...)  __android_log_print(ANDROID_LOG_FATAL,NDLOG_TAG,__VA_ARGS__)
#define nd_log_screen(...)  __android_log_print(ANDROID_LOG_INFO,NDLOG_TAG,__VA_ARGS__)

#elif defined(_MSC_VER)

#pragma  warning(disable: 4002)

#ifdef ND_OPEN_LOG_COMMON
#define nd_logmsg(...) _logmsg(__FUNC__ , __FILE__ , __LINE__ , ND_MSG  AND __VA_ARGS__)
#else 
	#define nd_logmsg(msg) //(void) 0
#endif

#ifdef ND_OPEN_LOG_DEBUG
#define nd_logdebug(...) _logmsg(__FUNC__ , __FILE__ , __LINE__  , ND_MSGDEBUG  AND __VA_ARGS__)
#else 
#define nd_logdebug(msg) //(void)0
#endif

#ifdef ND_OPEN_LOG_WARN
#define nd_logwarn(...) _logmsg(__FUNC__ , __FILE__ , __LINE__ ,  ND_WARN  AND __VA_ARGS__)
#else 
#define nd_logwarn(msg) //(void) 0
#endif

#ifdef ND_OPEN_LOG_ERROR
#define nd_logerror(...) _logmsg(__FUNC__ , __FILE__ , __LINE__ , ND_ERROR AND __VA_ARGS__)
#else 
#define nd_logerror(msg) //(void) 0
#endif

#ifdef ND_OPEN_LOG_FATAL
#define nd_logfatal(...) _logmsg(__FUNC__ , __FILE__ , __LINE__ ,  ND_FATAL_ERR  AND __VA_ARGS__)
#else 
#define nd_logfatal(msg) //(void) 0
#endif

#if defined(ND_OUT_LOG_2CTRL) && defined(ND_DEBUG) 
#define  nd_log_screen(...) _logmsg_screen(__FILE__ , __LINE__ AND __VA_ARGS__)
#else 
#define  nd_log_screen //(void)0
#endif 

//////////////////////////////////////////////////////////////////////
//unix 
// 
 #else

 #define LOG(format, ...) fprintf(stdout, format, __VA_ARGS__)

 #ifdef ND_OPEN_LOG_COMMON
 #define nd_logmsg(fmt,arg...) _logmsg(__FUNC__,__FILE__, __LINE__, ND_MSG ,fmt, ##arg)
 #else
 #define nd_logmsg(fmt,arg...) (void) 0
 #endif

 #ifdef ND_OPEN_LOG_DEBUG
 #define nd_logdebug(fmt,arg...) _logmsg(__FUNC__,__FILE__, __LINE__, ND_MSGDEBUG ,fmt, ##arg)
 #else
 #define nd_logdebug(fmt,arg...) (void) 0
 #endif

 #ifdef ND_OPEN_LOG_WARN
 #define nd_logwarn(fmt,arg...) _logmsg(__FUNC__,__FILE__, __LINE__, ND_WARN ,fmt, ##arg)
 #else
 #define nd_logwarn(fmt,arg...) (void) 0
 #endif

 #ifdef ND_OPEN_LOG_ERROR
 #define nd_logerror(fmt,arg...) _logmsg(__FUNC__,__FILE__, __LINE__, ND_ERROR ,fmt,##arg)
 #else
 #define nd_logerror(fmt,arg...) (void) 0
 #endif

 #ifdef ND_OPEN_LOG_FATAL
 #define nd_logfatal(fmt,arg...) _logmsg(__FUNC__,__FILE__, __LINE__, ND_FATAL_ERR ,fmt,##arg)
 #else
 #define nd_logfatal(fmt,arg...) (void) 0
 #endif

 #if defined(ND_OUT_LOG_2CTRL) && defined(ND_DEBUG)
 #define  nd_log_screen(fmt,arg...)  _logmsg_screen(__FILE__, __LINE__, fmt,##arg)
 #else
 #define  nd_log_screen(fmt,arg...) (void)0
 #endif

#endif		//end win32



//记录资源的使用情况
//在程序退出以后可以确定那些资源没有释放
// 
// nd_source_log 记录资源source 被函数operate函数获得 给出使用情况msg
// int nd_sourcelog(void *source, char *operate, char *msg) ;
// 释放资源source
// it nd_source_release(void *source) ;
// 程序退出,dump 出未释放的资源,不需要手动释放
// void nd_sourcelog_dump() ; 


#if defined(ND_FILE_TRACE) && defined(ND_SOURCE_TRACE)
	ND_COMMON_API FILE *nd_fopen_dbg( const char *filename, const char *mode ,const char *file, int line);
	ND_COMMON_API void nd_fclose_dbg(FILE *fp);	
#else 
#endif

#ifdef ND_SOURCE_TRACE 
ND_COMMON_API int nd_sourcelog_init() ;
ND_COMMON_API int _source_log(void *p ,const char *operate,const char *msg,const char *file, int line) ;
ND_COMMON_API int _source_release(void *source) ;
CPPAPI void nd_sourcelog_dump() ;
#define nd_sourcelog(source, operatename,msg) \
	_source_log(source,operatename,msg,__FILE__,__LINE__)
#define nd_source_release(source) 	_source_release(source) 
//define file open

#else
#define nd_sourcelog(source, operatename,msg) (void)0
#define nd_source_release(source) (void)0
#define nd_sourcelog_dump()	  (void)0
#define nd_sourcelog_init()	  (void)0

#endif 



#ifdef ND_DEBUG
#define ND_RUN_HERE()	printf("[%s : %d] run here...\n" AND __FILE__, __LINE__)
#define printf_dbg		printf
#define ND_PRINT_STACK() nd_show_cur_stack(fprintf, stderr) 

#else 
#define printf_dbg //
#define ND_RUN_HERE()	//

#define ND_PRINT_STACK() nd_show_cur_stack(fprintf, stderr) 
//#define nd_
#endif	//ND_DEBUG

//function call stack trace
ND_COMMON_API int nd_callstack_init(const char *filename) ;
ND_COMMON_API int nd_callstack_end() ;
ND_COMMON_API int nd_callstack_monitor_init(const char *filename) ;
ND_COMMON_API int nd_callstack_monitor_end() ;
ND_COMMON_API int push_func(const char *funcname);
ND_COMMON_API void pop_func(const char *funcname);
ND_COMMON_API char *nd_get_callstack_desc(char *buf, size_t size) ;


ND_COMMON_API int nd_callstack_monitor_dump(nd_out_func func,FILE *outfile);

ND_COMMON_API int nd_show_cur_stack(nd_out_func func, FILE *outfile);


#ifdef ND_CALLSTACK_TRACE

#define CALLSTACK_INIT(name)	nd_callstack_init(name)
#define CALLSTACK_DESTROY()	nd_callstack_end()
#define ENTER_FUNC()			int __push_func_return_value = push_func(__FUNC__) ;
#define LEAVE_FUNC()			do {if(0==__push_func_return_value) pop_func(__FUNC__) ;}while(0)

#else 

static int __INLINE__ CALLSTACK_INIT(const char *filename)
{
	return 0 ;
};
#define CALLSTACK_DESTROY()	(void)0
#define ENTER_FUNC()		//
#define LEAVE_FUNC()		//

#endif		//ND_CALLSTACK_TRACE


ND_COMMON_API int nd_sys_callstack_dump(nd_out_func func,FILE *outfile);
#endif
