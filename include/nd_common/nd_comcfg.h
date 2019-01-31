/* file nd_comcfg.h
 *
 * config nd common
 *
 * 2009-12-6 22:15
 */

#ifndef _ND_COMCFG_H_
#define _ND_COMCFG_H_

#define ND_VERSION "3.0"

#define E_SRC_CODE_ANSI  	0
#define E_SRC_CODE_GBK		1
#define E_SRC_CODE_UTF_8	2

#define E_SRC_CODE_UTF8		E_SRC_CODE_UTF_8

#define ND_ENCODE_UTF8		E_SRC_CODE_UTF_8
#define ND_ENCODE_GBK		E_SRC_CODE_GBK
#define ND_ENCODE_ANSI		E_SRC_CODE_ANSI

#define ND_FILE_PATH_SIZE	1024
#define NOT_SUPPORT_THIS_FUNCTION 0			// old linux version
#define ND_MULTI_THREADED 1				// must be define 
#define ND_MAX_THREAD_NUM 16			//for server

#define X86_64 1						//default is 64bits

#ifdef _MSC_VER
#define ND_ENCODE_TYPE E_SRC_CODE_GBK

#ifndef __ND_WIN__ 
#define __ND_WIN__ 1 
#endif  //_MSC_VER

#else
#define ND_ENCODE_TYPE E_SRC_CODE_UTF_8
#define ND_UNIX			//unix like system
#endif
			

//---------------------------------------------------------begin platform define
 //source code compiled encode
 //define platform
 /*
 __ND_MAC__		//MAC_OS
 __ND_LINUX__	//linux
 __ND_IOS__		//iOS
 __ND_ANDROID__ // android
 __ND_WIN__		//WIN32 WIN64
 __ND_BSD__		//freebsd bsd...

 #define ND_OPEN_TRACE		1
 #define ND_OPEN_LOG_COMMON 	1
 #define ND_OPEN_LOG_DEBUG	1
 #define ND_OPEN_LOG_WARN	1
 #define ND_OPEN_LOG_ERROR	1
 #define ND_OPEN_LOG_FATAL	1
 #define ND_SOURCE_TRACE		1
 #define ND_FILE_TRACE		1
 #define ND_OUT_LOG_2CTRL	1
 #define ND_OUT_LOG_2FILE	1
 #define ND_MEM_CHECK		1
 #define ND_USE_MSGBOX		1
 #define ND_LOG_WITH_SOURCE	1
 #define ND_LOG_WITH_TIME	1
 #define ND_MEM_STATICS		1		// statics memory used info and check mm-leak
 #define ND_UNUSE_STDC_ALLOC 1		// use nd_alloc replace
 #define ND_OVER_RIDE_NEW	1		// use myself new/delete
 #define ND_CALLSTACK_TRACE	1		//trace function call
 #define ND_LOG_WITH_ND_MARK 1 		//log with nd-log different other module
 #define WITHOUT_ICONV		1		// do not compile iconv-functions

 */

#if defined(__ND_IOS__)	

#define ND_PLATFORM "ARM"
#define WITHOUT_ICONV		1

#elif defined(__ND_ANDROID__)	// define IOS Android

#define ND_PLATFORM "ARM"

#define WITHOUT_ICONV		1
#define ND_LOG_WITH_ND_MARK 1 			//log with nd-log different other module

#define ND_OPEN_LOG_COMMON 	1
//#define ND_OPEN_LOG_DEBUG	1
#define ND_OPEN_LOG_WARN	1
#define ND_OPEN_LOG_ERROR	1
#define ND_OPEN_LOG_FATAL	1

#else  	

#define ND_PLATFORM "x86"

#define ND_OPEN_LOG_COMMON 	1
#define ND_OPEN_LOG_WARN	1
#define ND_OPEN_LOG_ERROR	1
#define ND_OPEN_LOG_FATAL	1
#define ND_OUT_LOG_2FILE	1
#define ND_LOG_WITH_SOURCE	1
#define ND_LOG_WITH_TIME	1
#define ND_LOG_WITH_DATE	1
#define ND_CALLSTACK_TRACE	1			//trace function call


#ifdef ND_DEBUG					//---------------------------define debug--------------

#define ND_OUT_LOG_2CTRL	1
#define ND_OPEN_TRACE		1		//vs TRACE
#define ND_OPEN_LOG_DEBUG	1
#define ND_SOURCE_TRACE		1
#define ND_FILE_TRACE		1
#define ND_TRACE_MESSAGE	1
#define ND_MEM_CHECK		1
#define ND_USE_MSGBOX		1
#define ND_MEM_STATICS		1
#define ND_UNUSE_STDC_ALLOC 1
#define ND_OVER_RIDE_NEW	1

#else //-------------------------------------release -----------------------------

#if defined(__ND_WIN__) 

#define ND_OUT_LOG_2CTRL	1
#define ND_UNUSE_STDC_ALLOC 1
#define ND_OVER_RIDE_NEW	1

#elif defined(__ND_LINUX__)
#define ND_LOG_PATH_WITH_DATE 1			//output log one day one direct
#elif defined(__ND_MAC__)
#else 
#error unknown platform !!!!!!!!!!!!!!!!!
#endif 

#endif		//---------------------------------debug --------------------------


#endif 	//-----------------------------platform----------------------


#endif
