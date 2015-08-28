/* file nd_comcfg.h
 *
 * config nd common
 *
 * 2009-12-6 22:15
 */

#ifndef _ND_COMCFG_H_
#define _ND_COMCFG_H_

#define ND_PLATFORM "x86"
#define ND_VERSION "3.0"

#define E_SRC_CODE_ANSI  	0
#define E_SRC_CODE_GBK		1
#define E_SRC_CODE_UTF_8	2

//source code compiled encode

#ifdef _MSC_VER
#define ND_ENCODE_TYPE E_SRC_CODE_GBK
#else
#define ND_ENCODE_TYPE E_SRC_CODE_UTF_8
#endif



//#define ND_USE_GPERF		1		//使用GOOGLE的内存优化库

#ifdef ND_DEBUG

#ifdef _MSC_VER
#define ND_USE_VLD		1		//使用VDL查找内存泄露
#endif

//#define ND_OVER_RIDE_NEW	1

#if defined(ND_USE_GPERF )||defined(ND_USE_VLD)
#else
#define ND_UNUSE_STDC_ALLOC 1		//不使用标准C的alloc
#define ND_MEM_CHECK		1		//内存访问越界检查
//#define ND_MEM_STATICS		1		//内测申请统计
//#define ND_SOURCE_TRACE		1		//记录每个申请的资源(打开的文件和申请的内存,还有系统资源)
//#define ND_FILE_TRACE		1		//记录每个打开的文件
#endif

//#define ND_OPEN_TRACE 		1		//打开VC调试窗口输出功能
#define ND_OPEN_LOG_COMMON 	1		//打开正常log输出
#define ND_OPEN_LOG_DEBUG	1		//打开debug log输出
#define ND_OPEN_LOG_WARN	1		//打开警告 log
#define ND_OPEN_LOG_ERROR	1		//打开错误log
#define ND_OPEN_LOG_FATAL	1		//打开严重错误输出
#define ND_OUT_LOG_2CTRL	1		//把日志输出到控制台
#define ND_OUT_LOG_2FILE	1		//把日志输出到文件
#define ND_USE_MSGBOX		1		//使用messagebox
#define ND_LOG_WITH_SOURCE	1		//日志输出带文件名
#define ND_LOG_WITH_TIME	1		//日志输出带时间
#else

//#define ND_USE_GPERF		1		//使用GOOGLE的内存优化库
#if defined(ND_USE_GPERF )||defined(ND_USE_VLD)
#else
#define ND_UNUSE_STDC_ALLOC 1		//不使用标准C的alloc
//#define ND_MEM_CHECK		1		//内存访问越界检查
//#define ND_MEM_STATICS		1		//内测申请统计
#endif
//#define ND_OPEN_TRACE		1		//打开VC调试窗口输出功能
#define ND_OPEN_LOG_COMMON 	1		//打开正常log输出
//#define ND_OPEN_LOG_DEBUG	1		//打开debug log输出
//#define ND_OPEN_LOG_WARN	1		//打开警告 log
#define ND_OPEN_LOG_ERROR	1		//打开错误log
#define ND_OPEN_LOG_FATAL	1		//打开严重错误输出
//#define ND_SOURCE_TRACE	1		//记录每个申请的资源(打开的文件和申请的内存,还有系统资源)
//#define ND_FILE_TRACE		1		//记录每个打开的文件
#define ND_OUT_LOG_2CTRL	1		//把日志输出到控制台
#define ND_OUT_LOG_2FILE	1		//把日志输出到文件
//#define ND_MEM_CHECK		1		//内存访问越界检查
#define ND_USE_MSGBOX		1		//使用messagebox
#define ND_LOG_WITH_SOURCE	1		//日志输出带文件名
#define ND_LOG_WITH_TIME	1		//日志输出带时间
//#define ND_MEM_STATICS		1		//内测申请统计

#endif			//end debug

#define ND_BUFSIZE 4096						//默认缓冲大小(临时缓冲,不要修改)

#define ND_FILE_PATH_SIZE	256

#define NOT_SUPPORT_THIS_FUNCTION 0			//在assert中提示不支持的功能

#define ND_MULTI_THREADED 1				//使用多线程(必须)

#define ND_MAX_THREAD_NUM 16

#define ND_CALLSTACK_TRACE	1			//跟踪函数调用堆栈

#ifndef BUILD_AS_STATIC_LIB
#define BUILD_AS_STATIC_LIB
#endif
// 
// #ifdef BUILD_AS_STATIC_LIB
// #else
// #define ND_COMPILE_AS_DLL	1			//编译成动态库
// #endif

//不支持unicode
//#define ND_UNICODE			1

#endif
