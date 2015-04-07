/* file nd_win.h
 * define windows type, function etc..
 * version  1.0 
 *
 * author : neil duan 
 * all right reserved  2007-9-27 
 */

#ifndef _ND_WIN_H_
#define _ND_WIN_H_

/*
#ifndef _WIN32_WINNT
#define  _WIN32_WINNT 0x0500		//default winnt 5
#endif
*/
// 
// #if defined(ND_DEBUG)
// #ifndef _CRTDBG_MAP_ALLOC
// #define _CRTDBG_MAP_ALLOC
// #endif
// #ifndef CRTDBG_MAP_ALLOC
// #define CRTDBG_MAP_ALLOC
// #endif
// #endif


#if defined(_WINSOCK2API_) || defined(_WINSOCKAPI_)
#else 
#include <winsock2.h>
#endif

#ifndef _WINDOWS_
#include <windows.h>
#endif


#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <errno.h>
//#include <crtdbg.h>

#include "nd_common/nd_comcfg.h"
#include "ndchar.h"

#pragma warning (disable:  4018)	
#pragma warning (disable:  4251)	
/* nonstandard extension 'single line comment' was used */
#pragma warning(disable:4001)	
// unreferenced formal parameter
#pragma warning(disable:4100)	
// Note: Creating precompiled header 
#pragma warning(disable:4699)	
// function not inlined
#pragma warning(disable:4710)	
// unreferenced inline function has been removed
#pragma warning(disable:4514)	
// assignment operator could not be generated
#pragma warning(disable:4512)

#pragma  warning(disable: 4996)

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#ifdef ND_COMPILE_AS_DLL
	#if  defined(ND_COMMON_EXPORTS) 
		#define ND_COMMON_API 				CPPAPI  __declspec(dllexport)
	#else
		#define ND_COMMON_API 				CPPAPI __declspec(dllimport)
	#endif
#else 
	#define ND_COMMON_API 				CPPAPI 
#endif

#define __INLINE__			__inline	


#define __ndthread  __declspec(thread)

ND_COMMON_API const char *nd_process_name();
ND_COMMON_API int nd_arg(int argc, const char *argv[]);


// compatible for unix
#define snprintf _snprintf
#define bzero(pstr,size) 	memset((pstr),0,(size))		//����bzero ����gcc bzero

//define assert
//only for x86
#ifdef WIN64
#define DebugBreak()    				(void)0
#else 
#ifdef _DEBUG
#define DebugBreak()    				__debugbreak() //_asm { int 3 }
#else  
#define DebugBreak()    				(void)0
#endif
#endif

__INLINE__ void ND_MsgBox(LPCSTR s) {
	char buf[1024] ;
	snprintf(buf, 1024, ("%s ASSERT FAILED") , nd_process_name()) ;
	MessageBoxA(GetActiveWindow(), s, buf, MB_OK);
}
__INLINE__ void ND_FAILED(LPCSTR szMsg) {
   ND_MsgBox(szMsg);
   DebugBreak();
}
// Put up an assertion failure message box.
__INLINE__ void ND_ASSERTFAIL(LPCSTR file, int line, PCSTR expr) {
   char sz[1024];
   snprintf(sz, 1024, "ASSERT failed in\nFile %s, line %d \n"
	   "nd_assert(%s)", file, line, expr);
   ND_FAILED(sz);
}
// Put up a message box if an assertion fails in a debug build.
//����NDASSERT()����assert()

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
typedef HANDLE 				ndsem_t ;		//�źű���
static __INLINE__ int _init_event(HANDLE *h)
{
	*h = CreateEvent(NULL,FALSE,FALSE,NULL) ;
	return (*h) ? 0 : -1 ;
}
#define nd_sem_wait(s,t)	WaitForSingleObject(s,t)			//�ȴ��ź�
#define nd_sem_post(s)		SetEvent(s)							//�����ź�
#define nd_sem_init(s)		_init_event(&(s))  //(s)=CreateEvent(NULL,FALSE,FALSE,NULL)	//initilize semahpore resource
#define nd_sem_destroy(s)   CloseHandle(s) 						//destroy semahpore resource

#define nd_sleep(ms)	Sleep(ms) 			//˯��1/1000 second

//thread operate
#define nd_thread_self()	GetCurrentThreadId()				//�õ��ֳ��Լ���id
#define nd_processid()		GetCurrentProcessId()				//�õ�����ID

ND_COMMON_API  DWORD _ErrBox(char *file, int line) ;
//last error 
#define nd_last_errno() GetLastError() 
ND_COMMON_API const char *nd_last_error() ;		//�õ�ϵͳ�����һ����������(����nd_commonģ���)
ND_COMMON_API const char *nd_str_error(int errcode) ; //��ϵͳ�����ת�������
#define nd_showerror()	_ErrBox(__FILE__,__LINE__)		//�������������ĶԻ���

#ifndef __FUNC__
#ifndef  __FUNCTION__ 
#define __FUNC__ 	"unknow_function"
#else 
#define __FUNC__ 	__FUNCTION__
#endif 
#endif

// ����messagebox
#define nd_msgbox(msg,title) MessageBoxA(GetActiveWindow(), msg, title, MB_OK)

//����TRACK
#ifdef ND_OPEN_TRACE
ND_COMMON_API int MyDbgReport(const char *file, int line, const char *stm, ...);
#define NDTRACF(msg)  MyDbgReport(__FILE__, __LINE__,msg) 
#define NDTRAC(msg)  MyDbgReport(NULL, 0 ,msg) 

#else 
#define MyDbgReport 
#define NDTRACF(msg) (void) 0
#define NDTRAC(msg) (void) 0

#endif	//_DEBUG

#ifdef ND_USE_MSGBOX
	#define nd_msgbox_dg(msg,title,flag) \
	do{	char buf[1024] ;				\
		char header[128] ;				\
		snprintf(buf,1024, "%s\n%s:%d line \n %s",(title),__FILE__,__LINE__, (msg)) ;	\
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


#endif 
