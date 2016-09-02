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
//define platform
/*
 __ND_MAC__		//MAC_OS
 __ND_LINUX__	//linux
 __ND_IOS__		//iOS
 __ND_ANDROID__ // android 
 __ND_WIN__		//WIN32 WIN64
 __ND_BSD__		//freebsd bsd...

 
 ND_SERVER //build for server
 ND_CLIENT //build for client 
 WITHOUT_ICONV //without iconv
 _GUI_TOOL_ // for gui tool
 ND_UNIX // UNIX-LIKE platform un-windows
 ND_COMPILE_AS_DLL //build as dll
 ND_UNICODE //inter-char as unicode-16
 BUILD_AS_STATIC_LIB //build as static lib
 
 //��֧��unicode
 //#define 			1
 you need define the macro which you need porting to .
*/

#ifdef _MSC_VER
#define ND_ENCODE_TYPE E_SRC_CODE_GBK

#ifndef __ND_WIN__ 
#define __ND_WIN__ 1 
#endif  //_MSC_VER

#else
#define ND_ENCODE_TYPE E_SRC_CODE_UTF_8
#define ND_UNIX 
#endif

#define BUILD_AS_STATIC_LIB 1 
#define ND_BUFSIZE 4096						
#define ND_FILE_PATH_SIZE	1024
#define NOT_SUPPORT_THIS_FUNCTION 0			// old linux version
#define ND_MULTI_THREADED 1				// must be define 
#define ND_MAX_THREAD_NUM 16			//for server

#if defined(__ND_WIN__)  //--------------------windows platform------------------

#ifdef ND_DEBUG
//#define ND_USE_VLD		1  // use vld to check memory leak
#endif 

#define ND_OPEN_TRACE		1		//vs TRACE 
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
#define ND_MEM_STATICS		1
#define ND_UNUSE_STDC_ALLOC 1
#define ND_OVER_RIDE_NEW	1
#define ND_CALLSTACK_TRACE	1			//trace function call


#elif defined(__ND_LINUX__)		//----------------------linux platform------------------------

//#define ND_OPEN_TRACE		1
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
#define ND_MEM_STATICS		1
#define ND_UNUSE_STDC_ALLOC 1
#define ND_OVER_RIDE_NEW	1			// use myself new/delete
#define ND_CALLSTACK_TRACE	1			//trace function call

#elif defined(__ND_MAC__)		//-----------------------------mac-os-----------------------

#define ND_OPEN_TRACE		1		//vs TRACE 
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
#define ND_MEM_STATICS		1
#define ND_UNUSE_STDC_ALLOC 1
#define ND_OVER_RIDE_NEW	1			// use myself new/delete
#define ND_CALLSTACK_TRACE	1			//trace function call

#elif defined(__ND_BSD__)		//---------------------------- bsd----------------------------

#define ND_OPEN_TRACE		1		//vs TRACE 
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
#define ND_MEM_STATICS		1
#define ND_UNUSE_STDC_ALLOC 1
#define ND_OVER_RIDE_NEW	1			// use myself new/delete
#define ND_CALLSTACK_TRACE	1			//trace function call

#elif defined(__ND_IOS__)		//-----------------------------iOS---------------------------

#define WITHOUT_ICONV		1

//#define ND_OPEN_TRACE		1		//vs TRACE
#define ND_OPEN_LOG_COMMON 	1
#define ND_OPEN_LOG_DEBUG	1
#define ND_OPEN_LOG_WARN	1
#define ND_OPEN_LOG_ERROR	1
#define ND_OPEN_LOG_FATAL	1
//#define ND_SOURCE_TRACE		1
//#define ND_FILE_TRACE		1
#define ND_OUT_LOG_2CTRL	1
#define ND_OUT_LOG_2FILE	1
//#define ND_MEM_CHECK		1
//#define ND_USE_MSGBOX		1
#define ND_LOG_WITH_SOURCE	1
#define ND_LOG_WITH_TIME	1
//#define ND_MEM_STATICS		1
//#define ND_UNUSE_STDC_ALLOC 1
//#define ND_OVER_RIDE_NEW	1			// use myself new/delete
//#define ND_CALLSTACK_TRACE	1			//trace function call
#define ND_LOG_WITH_ND_MARK 1 			//log with nd-log different other module

#elif defined(__ND_ADNROID__)	//-----------------------------android----------------------

#define WITHOUT_ICONV		1

#define ND_OPEN_LOG_COMMON 	1
#define ND_OPEN_LOG_DEBUG	1
#define ND_OPEN_LOG_WARN	1
#define ND_OPEN_LOG_ERROR	1
#define ND_OPEN_LOG_FATAL	1
//#define ND_SOURCE_TRACE		1
//#define ND_FILE_TRACE		1
#define ND_OUT_LOG_2CTRL	1
#define ND_OUT_LOG_2FILE	1
#define ND_MEM_CHECK		1
//#define ND_USE_MSGBOX		1
#define ND_LOG_WITH_SOURCE	1
#define ND_LOG_WITH_TIME	1
//#define ND_MEM_STATICS		1
//#define ND_UNUSE_STDC_ALLOC 1
#define ND_OVER_RIDE_NEW	1			// use myself new/delete
#define ND_CALLSTACK_TRACE	1			//trace function call
#define ND_LOG_WITH_ND_MARK 1 			//log with nd-log different other module

#else						//-------------------------------------UNKNOWN----------------------
#error unknown platform !!!!!!!!!!!!!!!!!
#endif


#endif
