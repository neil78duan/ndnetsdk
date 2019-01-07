/* file nd_mutex.c
 *
 * define mutex of ndengine
 *
 * create by neil
 * 2011/3/1 13:43:52
 */


#include "nd_common/nd_common.h"

// 0=multi-CPU, 1=single-CPU, -1=not set yet
static int s_multiProcess = -1 ;
static int s_spinCount = 0 ;

int initNDMutex(NDMutex *m) 
{	
	if(-1==s_multiProcess) {
		 s_spinCount = nd_getcpu_num() ;
		 s_multiProcess = s_spinCount == 1 ;
	}
	memset(m, 0 , sizeof(*m)) ;

	if(-1==nd_sem_init(m->hSig) ) {
		nd_logerror("nd_sem_init() : %s\n" AND nd_last_error() ) ;
		return -1 ;
	}
#if 0
	do 	{
		#define die(msg) { perror(msg); exit(EXIT_FAILURE); }

		int sval ;
		if (sem_getvalue(&m->hSig, &sval) == -1) 	die("nd_sem_init()") ;
		nd_log_screen("before post value =%d \n", sval);
		if (nd_sem_post(m->hSig) == -1) die("sem_post");
		if (sem_getvalue(&m->hSig, &sval) == -1) 	die("nd_sem_init()") ;
		nd_log_screen("AFTER post value =%d \n", sval);

		if (nd_sem_wait(m->hSig,ND_INFINITE) == -1) die("sem_post");

		if (sem_getvalue(&m->hSig, &sval) == -1) 	die("nd_sem_init()") ;
		nd_log_screen("AFTER WAIT value =%d \n TEST SEM SUCCESS!!!!!!!!\n", sval);

	} while (0);
	
#endif 

	if(0==s_multiProcess){
		nd_atomic_swap( &m->_spinCount,  s_spinCount);
    }    
	return 0;
}

//return vlues : 0(true) scuurss , -1(false) failed 
int tryEntryMutex(NDMutex *m)
{
	ndthread_t selfID= nd_thread_self() ;
	long spin = m->_spinCount ;
	int owned = 0;
	
	do { 
		owned = (nd_compare_swap(&m->lockCount, 0, 1));
		if(owned ) {
			m->ownerID = selfID ;
			m->used    = 1 ;
		}
		else {
			if(m->ownerID == selfID ) {
				++(m->used);
				nd_atomic_inc(&m->lockCount) ;
				owned = 1 ;
			}
		}
		
	} while(!owned && spin-- > 0 ) ;		
	return (owned?0:-1) ;
}

void entryMutex(NDMutex *m)
{
	ndthread_t selfID ;
	if(0==tryEntryMutex(m)) {
		//LOCK SUCCESS
		return ;
	}
	
	selfID= nd_thread_self() ;
	if(1==nd_atomic_inc(&m->lockCount) ){
		//entry success 
		m->ownerID = selfID ;
		m->used = 1 ;
	}
	else {
		if(m->ownerID == selfID){
			++(m->used) ;
		}
		else {
			nd_sem_wait(m->hSig , ND_INFINITE);
			//WaitForSingleObject(m->hSig , INFINITE);
			//yes I can get 
			//其实这里还需要重新测试条件,
			//不过所有新来的竞争者如果没有得到资源,都需要通过WaitForSingleObject
			//所以,不需要重新测试条件也可以
			m->ownerID = selfID ;
			m->used = 1 ;
		}
	}
}

void leaveMutex(NDMutex *m)
{
	nd_assert(m->ownerID==nd_thread_self()) ;
	if(--(m->used) > 0 ){
		//alread owned by myslef 
		nd_atomic_dec(&m->lockCount) ; 
	}
	else {
		m->ownerID = 0 ;
		if(nd_atomic_dec(&m->lockCount) > 0){
			nd_sem_post(m->hSig) ;
		}
	}
}

//destory 
void destoryMutex(NDMutex *m)
{
	nd_sem_destroy(m->hSig);
}

int initNDCondVar(NDCondVar *v) 
{
	memset(v, 0 , sizeof(*v)) ;
	return nd_sem_init(v->hSig) ;
}

int waitCondVar(NDCondVar *v , NDMutex *m)
{
	int hr ;
	ndthread_t self= nd_thread_self() ;
	
	if(m->ownerID != self) 
		return -1 ;
	
	nd_atomic_inc(&v->lockCount) ;
	leaveMutex(m) ;
	
	hr = nd_sem_wait(v->hSig , INFINITE) ;
	hr = (0==hr)? 0:-1 ;
	
	entryMutex(m) ;
	return (int)hr ;
}

int timewaitCondVar(NDCondVar *v, NDMutex *m, int mseconds)
{
	long hr ;
	ndthread_t self= nd_thread_self() ;
	
	if(m->ownerID != self) 
		return -1 ;
	if(0==mseconds) {
		return 0 ;
	}
	
	nd_atomic_inc(&v->lockCount) ;
	leaveMutex(m) ;
	
	hr = nd_sem_wait(v->hSig , mseconds);
	hr = (0==hr)? 0:-1 ;
	
	entryMutex(m) ;
	return (int)hr;
}

int signalCondVar(NDCondVar *v)
{
	ndatomic_t oldval ;
	while( (oldval=nd_atomic_read(&v->lockCount)) > 0 ){
		if(nd_compare_swap(&v->lockCount,oldval,(oldval-1)) )	{		
			nd_sem_post(v->hSig) ;
			break ;
		}
	}
	return 0 ;
}

int broadcastCondVar(NDCondVar *v)
{
	/*ndatomic_t oldval ;
	while( (oldval=nd_atomic_read(&v->lockCount)) > 0 ){
		//if(InterlockedCompareExchange((&v->lockCount),(oldval-1),oldval) > 0 )	{
		if(nd_compare_swap((&v->lockCount),oldval,(oldval-1)) )	{
			nd_sem_post(v->hSig) ;
		}
	}
	return 0 ;
	*/
	/*NOTE!!!
	 * maybe this is not a good way to post some times 
	 */
	register int oldval = nd_atomic_read(&v->lockCount) ;
	//oldval =(int) InterlockedCompareExchange((LPVOID*)(&v->lockCount),(LPVOID)oldval,(LPVOID)oldval)  ;
	while(oldval-->0){
		nd_sem_post(v->hSig) ;
		//nd_threadsched() ;
	}
	
	return 0 ;
}
int destoryCondVar(NDCondVar *v)
{
	nd_sem_destroy(v->hSig);
	return 0 ;
}
