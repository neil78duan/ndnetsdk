/* file _source_trace.h
 * 
 * move source tracer function to this file
 *
 * create by duan 
 * 2018.8.30
 */

#ifndef __SOURCE_TRACE_H_
#define __SOURCE_TRACE_H_

#include "nd_common/nd_logger.h"


#ifdef ND_SOURCE_TRACE 

ND_COMMON_API int nd_sourcelog_init(void);
ND_COMMON_API int _source_log(void *p, const char *operate, const char *msg, const char *file, int line);
ND_COMMON_API int _source_release(void *source);
CPPAPI void nd_sourcelog_dump(void);
#define nd_sourcelog(source, operatename,msg) _source_log(source,operatename,msg,__FILE__,__LINE__)
#define nd_source_release(source) 	_source_release(source) 
//define file open

#if defined(ND_FILE_TRACE) 
ND_COMMON_API FILE *nd_fopen_dbg(const char *filename, const char *mode, const char *file, int line);
ND_COMMON_API void nd_fclose_dbg(FILE *fp);
#endif

#else
#define nd_sourcelog(source, operatename,msg) (void)0
#define nd_source_release(source) (void)0
#define nd_sourcelog_dump()	  (void)0
#define nd_sourcelog_init()	  (void)0


static __INLINE__ int _source_log(void *p, const char *operate, const char *msg, const char *file, int line) { return 0; }
static __INLINE__  int _source_release(void *source) { return 0; }

#endif 




//function call stack trace
ND_COMMON_API int nd_callstack_init(const char *filename);
ND_COMMON_API int nd_callstack_end(void);
ND_COMMON_API int nd_callstack_monitor_init(const char *filename);
ND_COMMON_API int nd_callstack_monitor_end(void);
ND_COMMON_API int push_func(const char *funcname);
ND_COMMON_API void pop_func(const char *funcname);
ND_COMMON_API char *nd_get_callstack_desc(char *buf, size_t size);


ND_COMMON_API int nd_callstack_monitor_dump(nd_out_func func, FILE *outfile);

ND_COMMON_API int nd_show_cur_stack(nd_out_func func, FILE *outfile);


#ifdef ND_CALLSTACK_TRACE

#define CALLSTACK_INIT(name)	nd_callstack_init(name)
#define CALLSTACK_DESTROY()	nd_callstack_end()
#define ENTER_FUNC()			int __push_func_return_value = push_func(__FUNC__) ;
#define LEAVE_FUNC()			do {if(0==__push_func_return_value) pop_func(__FUNC__) ;}while(0)

#else 

static int __INLINE__ CALLSTACK_INIT(const char *filename)
{
	return 0;
};
#define CALLSTACK_DESTROY()	(void)0
#define ENTER_FUNC()		//
#define LEAVE_FUNC()		//

#endif		//ND_CALLSTACK_TRACE


ND_COMMON_API int nd_sys_callstack_dump(nd_out_func func, FILE *outfile);

#endif 
