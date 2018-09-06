/* file nd_os.h
 * define platform correlation of nd engine
 * version 1.0 
 * neil duan
 * all right reserved 2007
 * 2007-9-27 
 */
 
#ifndef _ND_OS_H_
#define _ND_OS_H_

#include "nd_common/nd_define.h"
#include "nd_common/nd_comcfg.h"
#include <stddef.h>

#ifdef __cplusplus
#define CPPAPI extern "C"
#else 
#define CPPAPI 
#endif

#define ndfprintf		fprintf     //ʹ��vararg�����ĸ�ʽ����� 
#define ndprintf		printf      //ʹ��vararg�����ĸ�ʽ���������׼��� 
#define ndsprintf		sprintf     //����vararg�������ʽ�����ַ��� 
#define ndvfprintf		vfprintf    //ʹ��stdarg�������ʽ��������ļ� 
#define ndvsprintf		vsnprintf    //��ʽ��stdarg������д���ַ��� 
#define ndsnprintf 		snprintf
#define ndstrncmp		strncmp

#define ndstrcat	strcat			//��һ���ַ����ӵ���һ���ַ�����β�� 
#define ndstrncat	strncat			//����ָ��ճ���ַ�����ճ�ӳ���. 
#define ndstrchr	strchr			//�������ַ����ĵ�һ��λ�� 
#define ndstrrchr	strrchr			//��β����ʼ�������ַ������ֵĵ�һ��λ�� 
#define ndstrpbrk	strpbrk			//��һ�ַ��ַ����в�����һ�ַ������κ�һ���ַ���һ�γ��ֵ�λ�� 
#define ndstrstr	strstr			//��һ�ַ����в�����һ�ַ�����һ�γ��ֵ�λ�� 
#define ndstrcspn	strcspn			//���ز������ڶ����ַ����ĵĳ�ʼ��Ŀ 
#define ndstrspn	strspn			//���ذ����ڶ����ַ����ĳ�ʼ��Ŀ 
#define ndstrcpy	strcpy			//�����ַ��� 
#define ndstrncpy	strncpy			//ͬʱָ����������Ŀ 
#define ndstrcmp	strcmp			//�Ƚ��������ַ��� 
#define ndstrncmp	strncmp			//ָ���Ƚ��ַ��ַ�������Ŀ 
#define ndstrlen	strlen			// ��ÿ��ַ�������Ŀ 
#define ndstrtok	strtok


#if !defined(ND_UNIX) 
#include "nd_common/nd_win.h"

	typedef HANDLE			ndth_handle ;	//�߳̾��
	typedef DWORD 			ndthread_t;	//�߳�id
	__INLINE__ int nd_thread_equal(ndthread_t t1, ndthread_t t2){return (t1==t2) ;}
	
	//typedef volatile long atomic_t ;
	static __INLINE__ size_t set_maxopen_fd(size_t max_fd) {return 0xffff;}
	static __INLINE__ size_t get_maxopen_fd() {return 0xffff;}
	ND_COMMON_API char* get_rlimit_info(char *buf, int buf_size) ;
		
	#define nd_close_handle(h)	CloseHandle(h)

#else //if __LINUX__		//UNIX OR linux platform
#include "nd_common/nd_unix.h"	
	typedef pthread_t		ndth_handle ;	//�߳̾��
	typedef pthread_t 		ndthread_t;		//�߳�ID
	#define  nd_thread_equal	pthread_equal
	ND_COMMON_API size_t set_maxopen_fd(size_t max_fd) ;
	ND_COMMON_API size_t get_maxopen_fd();
	#define nd_close_handle(h)	(void)0
	ND_COMMON_API int enable_core_dump(void);
	ND_COMMON_API char* get_rlimit_info(char *buf, int buf_size) ;

#endif

#define ND_INFINITE		0xffffffff

#include "nd_common/nd_atomic.h"

typedef void* (*NDTH_FUNC)(void* param) ;		//�̺߳���
enum {
	NDT_PRIORITY_NORMAL,
	NDT_PRIORITY_HIGHT,
	NDT_PRIORITY_LOW
};
//�����̺߳���
ND_COMMON_API ndth_handle nd_createthread(NDTH_FUNC func, void* param,ndthread_t *thid,int priority);
//ǿ���߳��ó�ִ��ʱ��
ND_COMMON_API int nd_threadsched(void) ;
//ǿ���߳��˳�
ND_COMMON_API void nd_threadexit(int exitcode);
//�ȴ�һ���̵߳Ľ���
ND_COMMON_API int nd_waitthread(ndth_handle handle) ;

ND_COMMON_API int nd_terminal_thread(ndth_handle handle,int exit_code);

ND_COMMON_API int nd_getcpu_num() ;

ND_COMMON_API int nd_get_sys_callstack(char *buf, size_t size) ;

ND_COMMON_API const char *nd_get_sys_username() ;//get system user name

typedef struct _sND_mutex
{
	int _spinCount ;	//spin
	ndatomic_t lockCount ;		//lock
	int used ;			//used times 
	ndthread_t ownerID ;		//owner thread id 
	//HANDLE hSig ;
	ndsem_t hSig;
}NDMutex ;

typedef struct _sND_condvar
{
	int lockCount ;		//lock
	ndsem_t hSig ;
}NDCondVar ;


/*define mutex operation*/
ND_COMMON_API int initNDMutex(NDMutex *m) ;
ND_COMMON_API int tryEntryMutex(NDMutex *m);
ND_COMMON_API void entryMutex(NDMutex *m);
ND_COMMON_API void leaveMutex(NDMutex *m);
ND_COMMON_API void destoryMutex(NDMutex *m);

/*define condition value*/
ND_COMMON_API int initNDCondVar(NDCondVar *v) ;
ND_COMMON_API int waitCondVar(NDCondVar *v, NDMutex *m);
ND_COMMON_API int timewaitCondVar(NDCondVar *v, NDMutex *m, int mseconds);
ND_COMMON_API int signalCondVar(NDCondVar *v);
ND_COMMON_API int destoryCondVar(NDCondVar *v);
ND_COMMON_API int broadcastCondVar(NDCondVar *v) ;

//#ifdef _MSC_VER

typedef NDMutex					nd_mutex ;
typedef NDCondVar 				nd_cond ;

//���廥��ӿ�
#define nd_mutex_init(m)	initNDMutex(m)	//��ʼ������
#define nd_mutex_lock(m) 	entryMutex(m)	//lock
#define nd_mutex_trylock(m) tryEntryMutex(m) 
#define nd_mutex_unlock(m) 	leaveMutex(m) 
#define nd_mutex_destroy(m) destoryMutex(m) 

//�������������ӿ�
#define nd_cond_init(c)				initNDCondVar(c)
#define nd_cond_destroy(c)			destoryCondVar(c)
#define nd_cond_wait(c, m)			waitCondVar(c, m)
#define nd_cond_timewait(c,m, ms)	timewaitCondVar(c,m,ms)
#define nd_cond_signal(v)			signalCondVar(v) 
#define nd_cond_broadcast(v)		broadcastCondVar(v) 

//#else
//#endif

typedef struct ndfast_lock
{
	ndatomic_t  locked;
}ndfastlock_t;

#define nd_flock(l) (0==nd_testandset(&((l)->locked)) ) 
#define nd_funlock(l) nd_atomic_swap(&((l)->locked),0)
#define nd_flock_init(l) nd_atomic_set(&((l)->locked),0) ;

static  __INLINE__ int nd_key_esc() 
{
	if(kbhit()) {
		if(ND_ESC==getch())
			return 1 ;
	}
	return 0 ;
}
#define NDMIN(a,b) ((a) < (b) ? (a) : (b))
#define NDMAX(a,b) ((a) > (b) ? (a) : (b))

ND_COMMON_API int nd_mem_share_create(const char *name, size_t size, nd_filemap_t *map_handle);
ND_COMMON_API int nd_mem_share_close(nd_filemap_t *map_handle);

static  __INLINE__ void * nd_mem_share_addr(nd_filemap_t *map_handler)
{
	return (void*)map_handler->paddr;
}
static  __INLINE__ size_t nd_mem_share_size(nd_filemap_t *map_handler)
{
	return map_handler->size;
}

#endif
