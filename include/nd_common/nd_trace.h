/* file : nd_trace.h
 * define function  trace some source used info
 *
 * version 1.0
 * neil duan
 * all right reserved 2007 by neil!
 * 2007-10-5 
 */

#ifndef _ND_TRACE_H_
#define _ND_TRACE_H_

#include <stdio.h>
#include <stdlib.h>

#include "nd_common/nd_comcfg.h"


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
#undef  fopen
#undef  fclose
#define fopen(filename, mod) nd_fopen_dbg(filename, mod,__FILE__,__LINE__)
#define fclose(fp)			nd_fclose_dbg(fp)
#endif


#endif
