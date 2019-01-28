/* file nd_unix.h
 * define unix type, function etc..
 * version  1.0 
 *
 * author : neil duan 
 * all right reserved  2007-9-27 
 */
 
#ifndef _ND_UINX_H_
#define _ND_UINX_H_

#include "nd_common/nd_comcfg.h"
#include "nd_common/nd_export_def.h"

#if defined(ND_UNIX) 
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <sched.h>
#include <sys/ioctl.h>
#include <semaphore.h>
#include <assert.h>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdarg.h>

//#define ND_COMMON_API CPPAPI

#ifndef __INLINE__
	#define __INLINE__			inline
#endif

#if  defined(__ND_IOS__) || defined(__ND_ANDROID__)
#define __ndthread  
#else
#define __ndthread  __thread
#endif


typedef unsigned int			HANDLE ;
typedef void				*HINSTANCE ;

#define __FUNC__	__func__
//last error 
#define nd_last_errno() errno
#define nd_last_error()	strerror(errno)
#define nd_str_error(_errid) strerror(_errid)
static __INLINE__ void _showerror(char *file, int line, const char *func) {
	ndfprintf(stderr, "[%s:%d(%s)] last error:%s\n", file, line, func, nd_last_error());
}
#define nd_showerror() _showerror(__FILE__,__LINE__,__FUNC__)

#define LOWORD(a)			((a) & 0xffff)
#define HIWORD(a)			(((a) >>16) & 0xffff)

#ifndef __cplusplus

#ifndef max 
#define max(a,b) 			(((a)>(b))? (a) : (b))
#endif 

#ifndef min
#define min(a,b) 			(((a)>(b))? (b) : (a))
#endif

#endif 

ND_COMMON_API int nd_getch(void);
ND_COMMON_API int kbhit ( void );
#define getch	nd_getch

ND_COMMON_API void nd_init_daemon(void) ;

#define NDSEM_SUCCESS		0
#define NDSEM_ERROR			-1
#define NDSEM_TIMEOUT		1
#define INFINITE            0xFFFFFFFF

#define ND_SEM_NAME_SIZE 128
struct nd_name_sem
{
	sem_t *_sem ;
	char _name[ND_SEM_NAME_SIZE] ;
};

typedef struct nd_name_sem *nd_sem_name_t ;
typedef struct nd_name_sem *ndsem_t ;

ND_COMMON_API int _nd_sem_open(nd_sem_name_t *sem,  unsigned int value) ;
ND_COMMON_API int _nd_sem_close(nd_sem_name_t sem) ;
ND_COMMON_API nd_sem_name_t _nd_sem_open_ex(const char *name, unsigned int value,int flag);
ND_COMMON_API int _unix_sem_timewait(nd_sem_name_t sem , NDUINT32 waittime)  ;

//#if defined(__ND_LINUX__)
//
//typedef sem_t 	*ndsem_t ;
//ND_COMMON_API int _sys_sem_timewait(sem_t *sem , NDUINT32 waittime)  ;
//
//#define nd_sem_wait(s, timeout)	_sys_sem_timewait(s, timeout) //sem_wait(&(s))
//static __INLINE__ int nd_sem_post(ndsem_t s)
//{
//	int ret = sem_post(s);
//	if (-1 == ret) {
//		nd_showerror();
//	}
//	return ret;
//}
//
//static __INLINE__ int _sys_sem_init(ndsem_t *sem)
//{
//	ndsem_t mysem = (ndsem_t) malloc(sizeof(sem_t)) ;
//	int ret = sem_init(mysem,0,0) ;
//	if(ret ==0) {
//		*sem =mysem ;
//	}
//	else {
//		free(mysem) ;
//		nd_showerror();
//	}
//	return ret ;
//}
//static __INLINE__ int _sys_sem_destroy(ndsem_t sem)
//{
//	int ret =sem_destroy(sem) ;
//	free(sem) ;
//	if (-1 == ret)
//		nd_showerror();
//	return ret;
//}
//
//#define nd_sem_init(s)		_sys_sem_init(&(s))
//#define nd_sem_destroy(s)   	_sys_sem_destroy(s) 		//destroy semahpore resource
//#define nd_sem_open(name) 	sem_open( name, O_CREAT, 0644, 0 )
//
//#else

#define nd_sem_wait(s, timeout)		_unix_sem_timewait(s, timeout) //sem_wait(&(s))
#define nd_sem_post(s)		sem_post((s)->_sem)		//
#define nd_sem_init(s)		_nd_sem_open(&(s),0)	//initilize semahpore resource, return 0 on success , error r
#define nd_sem_destroy(s)   _nd_sem_close(s) 		//destroy semahpore resource
#define nd_sem_open(name) _nd_sem_open_ex( name, 0, O_CREAT)

//#endif

#if defined(__ND_MAC__) || defined(__ND_IOS__)
#else
ND_COMMON_API void nd_init_daemon(void) ;
#endif



#define nd_thread_self()	pthread_self()
#define nd_processid()		getpid()


ND_COMMON_API void pthread_sleep(NDUINT32 msec) ;
#define nd_sleep			pthread_sleep  

#ifdef ND_DEBUG
#define nd_assert(a)		assert(a)
#else 
#define nd_assert(a)
#endif


//define truck functon

#ifdef ND_OPEN_TRACE
#define NDTRACF(msg,arg...) do{ndfprintf(stderr,"%s:%d",__FILE__,__LINE__) ; ndfprintf(stderr, msg,##arg);}while(0)
#define NDTRAC(msg,arg...)   ndfprintf(stderr, msg,##arg)
#define _CRTTRAC(msg,arg...) ndfprintf(stderr, msg,##arg)
#else 
#define NDTRACF(msg,arg...) 
#define NDTRAC(msg,arg...)
#define _CRTTRAC(msg,arg...)
#endif

#ifdef ND_USE_MSGBOX
	#define nd_msgbox(msg,title) ndfprintf(stderr,"%s:%s\n", title, msg) 
	#define nd_msgbox_dg(msg,title,flag) \
	do{	char buf[1024] ;				\
		char header[128] ;				\
		ndsnprintf(buf,1024, "%s\n%s:%d line \n %s",(title),__FILE__,__LINE__, (msg)) ;	\
		nd_msgbox(buf,header);		\
	}while(0)
#else 
	#define nd_msgbox_dg(text, cap) (void) 0
	#define nd_msgbox(msg,title) (void) 0
#endif

#define nd_exit(code) exit(code)

ND_COMMON_API int mythread_cond_timewait(pthread_cond_t *cond,
							pthread_mutex_t *mutex, 
							unsigned long mseconds);


//file map 
typedef struct nd_filemap_t
{
	void* paddr ;
	size_t size ;
}nd_filemap_t;

#endif 
#endif
