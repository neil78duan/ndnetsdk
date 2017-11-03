/* file nd_comcfg.h
 *
 * config nd common
 *
 * 2009-12-6 22:15
 */

#ifndef _SERVER_ND_COMCFG_H_
#define _SERVER_ND_COMCFG_H_

#define ND_PLATFORM "x86"

//------------debug

#ifdef ND_DEBUG

#define ND_OPEN_TRACE		1		//vs TRACE
#define ND_OPEN_LOG_COMMON 	1
#define ND_OPEN_LOG_DEBUG	1
#define ND_OPEN_LOG_WARN	1
#define ND_OPEN_LOG_ERROR	1
#define ND_OPEN_LOG_FATAL	1
#define ND_SOURCE_TRACE		1
#define ND_FILE_TRACE		1
#define ND_TRACE_MESSAGE	1
#define ND_OUT_LOG_2CTRL	1
#define ND_OUT_LOG_2FILE	1
#define ND_MEM_CHECK		1
#define ND_USE_MSGBOX		1
#define ND_LOG_WITH_SOURCE	1
#define ND_LOG_WITH_TIME	1
#define ND_LOG_WITH_DATE	1
#define ND_MEM_STATICS		1
#define ND_UNUSE_STDC_ALLOC 1
#define ND_OVER_RIDE_NEW	1
#define ND_CALLSTACK_TRACE	1			//trace function call
//#define ND_LOG_PATH_WITH_DATE 1			//output log one day one direct

#else 

//------------
#if defined(__ND_WIN__)  //--------------------windows platform------------------


#define ND_OPEN_LOG_COMMON 	1
#define ND_OPEN_LOG_DEBUG	1
#define ND_OPEN_LOG_WARN	1
#define ND_OPEN_LOG_ERROR	1
#define ND_OPEN_LOG_FATAL	1
#define ND_OUT_LOG_2FILE	1
#define ND_LOG_WITH_SOURCE	1
#define ND_LOG_WITH_TIME	1
#define ND_LOG_WITH_DATE	1
#define ND_CALLSTACK_TRACE	1			//trace function call

#elif defined(__ND_LINUX__)		//----------------------linux platform------------------------

#define ND_OPEN_LOG_COMMON 	1
#define ND_OPEN_LOG_DEBUG	1
#define ND_OPEN_LOG_WARN	1
#define ND_OPEN_LOG_ERROR	1
#define ND_OPEN_LOG_FATAL	1
#define ND_OUT_LOG_2FILE	1
#define ND_LOG_WITH_SOURCE	1
#define ND_LOG_WITH_TIME	1
#define ND_LOG_WITH_DATE	1
#define ND_CALLSTACK_TRACE	1			//trace function call
#define ND_LOG_PATH_WITH_DATE 1			//output log one day one direct

#elif defined(__ND_MAC__)		//-----------------------------mac-os-----------------------

#define ND_OPEN_LOG_COMMON 	1
#define ND_OPEN_LOG_DEBUG	1
#define ND_OPEN_LOG_WARN	1
#define ND_OPEN_LOG_ERROR	1
#define ND_OPEN_LOG_FATAL	1
#define ND_OUT_LOG_2FILE	1
#define ND_LOG_WITH_SOURCE	1
#define ND_LOG_WITH_TIME	1
#define ND_LOG_WITH_DATE	1
#define ND_CALLSTACK_TRACE	1			//trace function call

#elif defined(__ND_IOS__)		//-----------------------------iOS---------------------------


#define ND_OPEN_LOG_COMMON 	1
#define ND_OPEN_LOG_ERROR	1
#define ND_OPEN_LOG_FATAL	1
#define ND_OUT_LOG_2CTRL	1
#define ND_LOG_WITH_SOURCE	1
#define ND_LOG_WITH_TIME	1
#define ND_LOG_WITH_DATE	1

#define WITHOUT_ICONV		1


#elif defined(__ND_ADNROID__)	//-----------------------------android----------------------

#define WITHOUT_ICONV		1
#define ND_LOG_WITH_ND_MARK 1 			//log with nd-log different other module
#define ND_OPEN_LOG_COMMON 	1
#define ND_OPEN_LOG_DEBUG	1
#define ND_OPEN_LOG_WARN	1
#define ND_OPEN_LOG_ERROR	1
#define ND_OPEN_LOG_FATAL	1

#else						//-------------------------------------UNKNOWN----------------------
#error unknown platform !!!!!!!!!!!!!!!!!
#endif

// 
// //#define ND_OPEN_TRACE		1
// #define ND_OPEN_LOG_COMMON 	1
// //#define ND_OPEN_LOG_DEBUG	1
// #define ND_OPEN_LOG_WARN	1
// #define ND_OPEN_LOG_ERROR	1
// #define ND_OPEN_LOG_FATAL	1
// //#define ND_SOURCE_TRACE		1
// //#define ND_FILE_TRACE		1
// //#define ND_OUT_LOG_2CTRL	1
// #define ND_OUT_LOG_2FILE	1
// //#define ND_MEM_CHECK		1
// //#define ND_USE_MSGBOX		1
// #define ND_LOG_WITH_SOURCE	1
// #define ND_LOG_WITH_TIME	1
// //#define ND_MEM_STATICS		1
// //#define ND_UNUSE_STDC_ALLOC 1
// //#define ND_OVER_RIDE_NEW	1			// use myself new/delete
// //#define ND_CALLSTACK_TRACE	1			//trace function call

#endif



#endif
