/* file nd_win.h
 * define windows type, function etc..
 * version  1.0 
 *
 * author : neil duan 
 * all right reserved  2007-9-27 
 */

#ifndef _ND_WIN_H_
#define _ND_WIN_H_

#ifndef _UNNEED_INCLUDE_WINDOWS_SYSTEM
#ifndef _WINSOCK2API_
// Prevent inclusion of winsock.h
#ifdef _WINSOCKAPI_
#error Header winsock.h is included unexpectedly.
#endif

// NOTE: If you use Windows Platform SDK, you should enable following definition:
// #define USING_WIN_PSDK

#if !defined(WIN32_LEAN_AND_MEAN) && (_WIN32_WINNT >= 0x0400) && !defined(USING_WIN_PSDK)
#include <windows.h>
#else
#include <winsock2.h>
#endif
#endif//_WINSOCK2API_
#endif //_UNNEED_INCLUDE_WINDOWS_SYSTEM

#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <errno.h>
#include <string.h>

#include "nd_common/nd_comcfg.h"
#include "nd_common/nd_export_def.h"


#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

//#define __INLINE__			__inline	


#define __ndthread  __declspec(thread)

ND_COMMON_API const char *nd_process_name();
ND_COMMON_API int nd_arg(int argc, const char *argv[]);


// compatible for unix
//#define ndsnprintf _snprintf
#ifndef ndsnprintf
#define ndsnprintf  sprintf_s
#endif

#define bzero(pstr,size) 	memset((pstr),0,(size))		//define bzero in windows

//define assert
//only for x86
// #ifdef WIN64
// #define ND_DebugBreak()    				__debugbreak
// #else 
// #endif

#ifdef _DEBUG
#define ND_DebugBreak()    				__debugbreak() //_asm { int 3 }
#else  
#define ND_DebugBreak()    				(void)0
#endif

__INLINE__ void ND_MsgBox(LPCSTR s) {
	char buf[1024];
	ndsnprintf(buf, 1024, ("%s ASSERT FAILED"), nd_process_name());
	MessageBoxA(GetActiveWindow(), s, buf, MB_OK);
}

__INLINE__ void ND_FAILED(LPCSTR szMsg) {
	ND_MsgBox(szMsg);
	ND_DebugBreak();
}
// Put up an assertion failure message box.
__INLINE__ void ND_ASSERTFAIL(LPCSTR file, int line, PCSTR expr) {
	char sz[1024];
	ndsnprintf(sz, 1024, "ASSERT failed in\nFile %s, line %d \n"
		"nd_assert(%s)", file, line, expr);
	ND_FAILED(sz);
}

// Put up a message box if an assertion fails in a debug build.

#ifdef _DEBUG
#define nd_assert(x) if (!(x)) ND_ASSERTFAIL(__FILE__, __LINE__, #x)
#else
#define nd_assert(x)
#endif


//define sem wait return value
#define NDSEM_SUCCESS		WAIT_OBJECT_0
#define NDSEM_ERROR			WAIT_ABANDONED
#define NDSEM_TIMEOUT		WAIT_TIMEOUT

//single operate
typedef HANDLE 				ndsem_t ;		//sem tyep
static __INLINE__ int _init_event(HANDLE *h)
{
	*h = CreateEvent(NULL,FALSE,FALSE,NULL) ;
	return (*h) ? 0 : -1 ;
}

static __INLINE__ HANDLE _sem_init_nanme(const char *name)
{
	return CreateEventA(NULL, FALSE, FALSE, name);
}

#define nd_sem_wait(s,t)	WaitForSingleObject(s,t)			//wait sem
#define nd_sem_post(s)		SetEvent(s)							//
#define nd_sem_init(s)		_init_event(&(s))  //(s)=CreateEvent(NULL,FALSE,FALSE,NULL)	//initilize semahpore resource
#define nd_sem_destroy(s)   CloseHandle(s) 						//destroy semahpore resource
#define nd_sem_open(name)  _sem_init_nanme(name) 

#define nd_sleep(ms)	Sleep(ms) 			// sleep 1/1000 second

//thread operate
#define nd_thread_self()	GetCurrentThreadId()		
#define nd_processid()		GetCurrentProcessId()	

ND_COMMON_API  DWORD _ErrBox(char *file, int line) ;
//last error 
#define nd_last_errno() GetLastError() 
ND_COMMON_API const char *nd_last_error() ;				// get system last error id
ND_COMMON_API const char *nd_str_error(int errcode) ; //get system error descript 
#define nd_showerror()	_ErrBox(__FILE__,__LINE__)		//error message box 

#ifndef __FUNC__
#ifndef  __FUNCTION__ 
#define __FUNC__ 	"unknow_function"
#else 
#define __FUNC__ 	__FUNCTION__
#endif 
#endif

// windows messagebox
#define nd_msgbox(msg,title) MessageBoxA(GetActiveWindow(), msg, title, MB_OK)

//vs TRACK function
#ifdef ND_OPEN_TRACE
ND_COMMON_API int MyDbgReport(const char *file, int line, const char *stm, ...);
#define NDTRACF(msg,...)  MyDbgReport(__FILE__, __LINE__,msg,__VA_ARGS__) 
#define NDTRAC(msg,...)  MyDbgReport(NULL, 0 ,msg,__VA_ARGS__) 

#else 
#define MyDbgReport 
#define NDTRACF(msg,...) (void) 0
#define NDTRAC(msg,...) (void) 0

#endif	//_DEBUG

#ifdef ND_USE_MSGBOX
	#define nd_msgbox_dg(msg,title,flag) \
	do{	char buf[1024] ;				\
		char header[128] ;				\
		ndsnprintf(buf,1024, "%s\n%s:%d line \n %s",(title),__FILE__,__LINE__, (msg)) ;	\
		nd_msgbox(buf,header,flag);		\
	}while(0)
#else 
	#define nd_msgbox_dg(text, cap,flag) (void) 0
#endif

#define nd_exit(code)  PostQuitMessage(code)

//define condition value
#include <process.h>

//file map 
typedef struct nd_filemap_t
{
	HANDLE hFile,hFileMap ;
	PVOID paddr ;
	size_t size ;
}nd_filemap_t;

#include <time.h>
static __INLINE__ struct tm* localtime_r(const time_t* _time, struct tm* _res_tm)
{
	if (EINVAL == localtime_s(_res_tm, _time)) {
		return NULL;
	}
	return _res_tm;
}
static __INLINE__ struct tm* gmtime_r(const time_t* _time, struct tm* _res_tm)
{
	if (EINVAL == gmtime_s(_res_tm, _time)) {
		return NULL;
	}
	return _res_tm;
}

static __INLINE__ time_t timegm( struct tm* _res_tm)
{
	return _mkgmtime(_res_tm);
}
#endif 
