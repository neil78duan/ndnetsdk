/* file nd_unix.h
 * define unix type, function etc..
 * version  1.0 
 *
 * author : neil duan 
 * all right reserved  2007-9-27 
 */
 
#ifndef _ND_UINX_H_
#define _ND_UINX_H_

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

#define ND_COMMON_API CPPAPI

#ifndef __INLINE__
	#define __INLINE__			inline
#endif

#ifdef  ND_IOS
#define __ndthread  
#else
#define __ndthread  __thread
#endif


typedef unsigned int		HANDLE ;
typedef void				*HINSTANCE ;

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

#ifdef __MAC_OS__

#define ND_SEM_NAME_SIZE 16
typedef struct nd_mac_sem
{
    sem_t *_sem ;
    char _name[ND_SEM_NAME_SIZE] ;
} *ndsem_t ;
//typedef sem_t* 				ndsem_t ;
ND_COMMON_API int _unix_sem_timewait(ndsem_t sem , NDUINT32 waittime)  ;
ND_COMMON_API int _nd_sem_open(ndsem_t *sem,  unsigned int value) ;
ND_COMMON_API int _nd_sem_close(ndsem_t sem) ;

#define nd_sem_wait(s, timeout)		_unix_sem_timewait(s, timeout) //sem_wait(&(s))		//µ»¥˝–≈∫≈
#define nd_sem_post(s)		sem_post((s)->_sem)		//∑¢ÀÕ–≈∫≈
#define nd_sem_init(s)		_nd_sem_open(&(s),0)	//initilize semahpore resource, return 0 on success , error r
#define nd_sem_destroy(s)   _nd_sem_close(s) 		//destroy semahpore resource

#else

typedef sem_t 				ndsem_t ;			//–≈∫≈±‰¡ø
ND_COMMON_API int _unix_sem_timewait(ndsem_t *sem , NDUINT32 waittime)  ;

#define nd_sem_wait(s, timeout)		_unix_sem_timewait(&(s), timeout) //sem_wait(&(s))		//µ»¥˝–≈∫≈
#define nd_sem_post(s)		sem_post(&(s))		//∑¢ÀÕ–≈∫≈
#define nd_sem_init(s)		sem_init(&(s),0,0)	//initilize semahpore resource, return 0 on success , error r
#define nd_sem_destroy(s)   sem_destroy(&(s)) 		//destroy semahpore resource


ND_COMMON_API void nd_init_daemon(void) ;
#endif



#define nd_thread_self()	pthread_self()		//µ√µΩœ÷≥…◊‘º∫µƒid
#define nd_processid()		getpid()			//µ√µΩΩ¯≥ÃID


ND_COMMON_API void pthread_sleep(NDUINT32 msec) ;
#define nd_sleep			pthread_sleep  		//ÀØ√ﬂ1/1000 second

#ifdef ND_DEBUG
#define nd_assert(a)		assert(a)
#else 
#define nd_assert(a)
#endif


#if	0
#define __FUNC__ 	__ASSERT_FUNCTION
#else 
#define __FUNC__	__func__
#endif 

//last error 
#define nd_last_errno() errno
#define nd_last_error()	strerror(errno)
#define nd_str_error(_errid) strerror(_errid)
static __INLINE__ void _showerror(char *file, int line,const char *func) { 
	fprintf(stderr,"[%s:%d(%s)] last error:%s\n",file, line, func, nd_last_error()) ;}
#define nd_showerror() _showerror(__FILE__,__LINE__,__FUNC__)

//define truck functon

#ifdef ND_OPEN_TRACE
#define NDTRACF(msg,arg...) do{fprintf(stderr,"%s:%d",__FILE__,__LINE__) ; fprintf(stderr, msg,##arg);}while(0)
#define NDTRAC(msg,arg...)   fprintf(stderr, msg,##arg)
#define _CRTTRAC(msg,arg...) fprintf(stderr, msg,##arg)
#else 
#define NDTRACF(msg,arg...) 
#define NDTRAC(msg,arg...)
#define _CRTTRAC(msg,arg...)
#endif

#ifdef ND_USE_MSGBOX
	#define nd_msgbox(msg,title) fprintf(stderr,"%s:%s\n", title, msg) 
	#define nd_msgbox_dg(msg,title,flag) \
	do{	char buf[1024] ;				\
		char header[128] ;				\
		snprintf(buf,1024, "%s\n%s:%d line \n %s",(title),__FILE__,__LINE__, (msg)) ;	\
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

/*
typedef pthread_mutex_t		nd_mutex ;
typedef pthread_cond_t		nd_cond;

#define nd_mutex_init(m)	pthread_mutex_init((m), NULL) 
#define nd_mutex_lock(m) 	pthread_mutex_lock(m) 
#define nd_mutex_trylock(m)	pthread_mutex_trylock(m) 
#define nd_mutex_unlock(m) 	pthread_mutex_unlock(m) 
#define nd_mutex_destroy(m) pthread_mutex_destroy(m)

#define nd_cond_init(m)		pthread_cond_init((m), NULL) 
#define nd_cond_destroy(c)  pthread_cond_destroy(c)
#define nd_cond_wait(c, m)			pthread_cond_wait(c, m)
#define nd_cond_timewait(c,m, ms) 	mythread_cond_timewait(c,m,ms)
#define nd_cond_signal(v)			pthread_cond_signal(v) 
#define nd_cond_broadcast(v)		pthread_cond_broadcast(v)
*/
//file map 
typedef struct nd_filemap_t
{
	void* paddr ;
	size_t size ;
}nd_filemap_t;

#endif 
#endif
