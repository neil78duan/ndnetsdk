/* file nd_unix.c
 * define function in windows platform
 * version  1.0 
 *
 * author : neil duan 
 * all right reserved  2007-9-27 
 */
 

#if defined(ND_UNIX) 
#include "nd_common/nd_common.h"
#include <time.h>  
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>

#include <semaphore.h>
#include <signal.h>


//return NDSEM_SUCCESS wait success, NDSEM_ERROR error , NDSEM_TIMEOUT timeout

static int nd_clock_gettime( struct timespec *t)
{
    struct timeval now;
    gettimeofday(&now, NULL);
    
    t->tv_sec  = now.tv_sec;
    t->tv_nsec = now.tv_usec * 1000;
    
    return 0;
}

//calc two time duration
struct timeval nd_time_sub(struct timeval *start, struct timeval *end )
{
    signed long us_deta = 0 ;
    struct timeval ret ;
    
    if ( end->tv_usec >= start->tv_usec) {
        ret.tv_usec = end->tv_usec - start->tv_usec ;
        ret.tv_sec = end->tv_sec - start->tv_sec ;
    }
    else {
        
        us_deta = end->tv_sec - start->tv_sec  -1;
        if (us_deta < 0) {
            us_deta = 0 ;
        }
        ret.tv_sec = us_deta ;
        
        ret.tv_usec = 1000000 +  end->tv_usec - start->tv_usec ;
        
    }
    return ret ;
    
}

int mythread_cond_timewait(pthread_cond_t *cond,
                           pthread_mutex_t *mutex, unsigned long mseconds)
{
    //struct timeval now;
    struct timespec timeout;
    //int retcode;
    
    if(-1==nd_clock_gettime(&timeout) ){
        return -1 ;
    }
    
    timeout.tv_sec += mseconds /1000 ;
    timeout.tv_nsec += (mseconds %1000 ) * 1000000;
    return  pthread_cond_timedwait(cond, mutex, &timeout) ;
    
}


#ifdef __MAC_OS__

void pthread_sleep(NDUINT32 msec)
{
    useconds_t t = msec * 1000;
    usleep(t) ;
}

int _unix_sem_timewait(ndsem_t pSem , NDUINT32 waittime)
{
    sem_t *sem = pSem->_sem ;
    int times = waittime / 10 ;
    int ret ;
    if(ND_INFINITE==waittime) {
        return sem_wait(sem) ;
    }
    else if(times > 0){
        
        while (times) {
            --times ;
            ret = sem_trywait(sem) ;
            if (ret==-1 ) {
                if (errno==EAGAIN) {
                    continue ;
                }
                return NDSEM_ERROR ;
            }
            return NDSEM_SUCCESS;
        }
        return NDSEM_TIMEOUT ;
    }
    else {
        return  sem_trywait(sem);
    }
}


int _nd_sem_open(ndsem_t *sem, unsigned int value)
{
    static ndatomic_t _s_sem_index = 0 ;
    //char sem_name[64] ;
    ndsem_t  psem = (ndsem_t) malloc(sizeof(struct nd_mac_sem) );
    do {
        snprintf(psem->_name, sizeof(psem->_name), "ndsem%d", nd_atomic_dec( &_s_sem_index)) ;
        
        psem->_sem = sem_open( psem->_name, O_CREAT|O_EXCL, 0644, value );
        if (psem->_sem== SEM_FAILED) {
            //nd_logerror("sem_open(%s) : %s\n",psem->_name, nd_last_error()) ;
            if (errno != EEXIST) {
                free(psem) ;
                return -1 ;
            }
            
        }
        
    }while (psem->_sem==SEM_FAILED && errno==EEXIST) ;
    *sem = psem ;
    return 0;
}


int _nd_sem_close(ndsem_t sem)
{
    sem_close(sem->_sem) ;
    sem_unlink(sem->_name) ;
    free(sem) ;
    return  0;
}

#else

void pthread_sleep(NDUINT32 msec)
{
    pthread_cond_t _cond = PTHREAD_COND_INITIALIZER ;
    pthread_mutex_t _mutex = PTHREAD_MUTEX_INITIALIZER ;
    
    mythread_cond_timewait(&_cond,&_mutex, msec) ;
}

int _unix_sem_timewait(ndsem_t *sem , NDUINT32 waittime) 
{
	int ret ;
	if(ND_INFINITE==waittime) {		
		return sem_wait(sem) ;
	}
	else if(waittime){
		
		struct timespec ts ;
		
		if (nd_clock_gettime(&ts) == -1) {
			return -1 ;
		}
		ts.tv_sec += waittime /1000 ;
		ts.tv_nsec += (waittime %1000 ) * 1000000;
		
		//这里不能使用 (ret = sem_timedwait(sem, &ts)) == -1 && errno == EINTR 这个判断
		//否则ctrl+c将不能使程序正常退出
		//while ((ret = sem_timedwait(sem, &ts)) == -1 && errno == EINTR)
		//	continue ;
		//check what happened
		ret = sem_timedwait(sem, &ts);
		if (ret == -1) {
			if (errno == ETIMEDOUT)
				return NDSEM_TIMEOUT ;
			else
				return NDSEM_ERROR ;
		} 
		else
			return NDSEM_SUCCESS ;
	}
	else {
		return  sem_trywait(sem);
	}
}
#endif


ndtime_t nd_time(void)
{
	struct timeval tmnow, deta;
	static struct timeval __start_time ;	//程序启动时间
	static int __timer_inited = 0 ;
	
	if(0==__timer_inited) {
		gettimeofday(&__start_time, NULL) ;
		__timer_inited = 1 ;
	}
	gettimeofday(&tmnow, NULL) ;
    
    deta = nd_time_sub(&__start_time, &tmnow) ;

	
	return (ndtime_t)(deta.tv_sec * 1000 + deta.tv_usec / 1000 );
}


#include <sched.h>

//创建线程函数
ndth_handle nd_createthread(NDTH_FUNC func, void* param,ndthread_t *thid,int priority)
{
	pthread_t threadid ;
	
	struct sched_param schparam ;
	pthread_attr_t attr ;
	
	pthread_attr_init(&attr);

	pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
	pthread_attr_setdetachstate (&attr,PTHREAD_CREATE_JOINABLE);
	
	if(priority==NDT_PRIORITY_HIGHT) {
		struct sched_param param ;
		
		pthread_attr_getschedparam(&attr, &param) ;
		
		++(param.sched_priority);
		pthread_attr_setschedparam(&attr, &param); 
	}
	else if(NDT_PRIORITY_LOW) {
		struct sched_param param ;
		
		pthread_attr_getschedparam(&attr, &param) ;
		
		--(param.sched_priority);
		pthread_attr_setschedparam(&attr, &param); 
		
	}

	if(-1==pthread_create(&threadid, &attr, func, param) ){
		return 0 ;
	}
	
	if(thid) *thid = (ndthread_t)threadid ;
	return threadid ;
}

int nd_threadsched(void) 
{
	return sched_yield() ; 
}

void nd_threadexit(int exitcode)
{
    void *param =  exitcode ;
	pthread_exit( param );
}
//等待一个线程的结束
int nd_waitthread(ndth_handle handle) 
{
	ndthread_t selfid = nd_thread_self() ;
	if(selfid!=(ndthread_t)handle){
		return pthread_join(handle,NULL)  ;
	}
	return -1 ;
}

int nd_terminal_thread(ndth_handle handle,int exit_code)
{
#ifdef ND_ANDROID
    return -1;
#else
	return pthread_cancel(handle) ;
#endif
    //return  0;
}

#include <sys/ioctl.h>
#include <termios.h>

#ifdef __LINUX__
int nd_getch(void)
{
	int ret ;
	char ch;
	struct termios save, ne;
	ioctl(0, TCGETS, &save);
	ioctl(0, TCGETS, &ne);
	ne.c_lflag &= ~(ECHO | ICANON);
	ioctl(0, TCSETS, &ne);
	ret = read(0, &ch, 1);
	ioctl(0, TCSETS, &save);
	return ch;
}
#else
int nd_getch( void )
{
	int c = 0;
	struct termios org_opts, new_opts;
	int res = 0;
	  //-----  store old settings -----------
	res = tcgetattr( STDIN_FILENO, &org_opts );
	nd_assert( res == 0 );
	  //---- set new terminal parms --------
	memcpy( &new_opts, &org_opts, sizeof(new_opts) );
	new_opts.c_lflag &= ~( ICANON | ECHO | ECHOE | ECHOK | ECHONL | ECHOPRT | ECHOKE | ICRNL );
	tcsetattr( STDIN_FILENO, TCSANOW, &new_opts );
	c = getchar();
	  //------  restore old settings ---------
	res = tcsetattr( STDIN_FILENO, TCSANOW, &org_opts );
	nd_assert( res == 0 );
	return( c );
}
#endif

int kbhit ( void )
{
	struct timeval tv;
	struct termios old_termios, new_termios;
	int            error;
	int            count = 0;
	tcgetattr( 0, &old_termios );
	new_termios              = old_termios;
	/*
	 * raw mode
	 */
	new_termios.c_lflag     &= ~ICANON;
	/*
	 * disable echoing the char as it is typed
	 */
	new_termios.c_lflag     &= ~ECHO;
	/*
	 * minimum chars to wait for
	 */
	new_termios.c_cc[VMIN]   = 1;
	/*
	 * minimum wait time, 1 * 0.10s
	 */
	new_termios.c_cc[VTIME]  = 1;
	error                    = tcsetattr( 0, TCSANOW, &new_termios );
	tv.tv_sec                = 0;
	tv.tv_usec               = 100;
	/*
	 * insert a minimal delay
	 */
	select( 1, NULL, NULL, NULL, &tv );
	error                   += ioctl( 0, FIONREAD, &count );
	error                   += tcsetattr( 0, TCSANOW, &old_termios );
	return( error == 0 ? count : -1 );
}  /* end of kbhit */


#include <sys/resource.h>
int set_maxopen_fd(int max_fd)
{
	struct rlimit rt;
    /* 设置每个进程允许打开的最大文件数 */
    /*需要以root运行*/
    rt.rlim_max = rt.rlim_cur = max_fd;
    setrlimit(RLIMIT_NOFILE, &rt) ;
    
    return  getdtablesize() ;
}

int create_filemap(const char *filename, size_t size,nd_filemap_t *out_handle)
{
	int fd =-1,i;
	char temp;

	if (filename) {
		fd=open(filename,O_CREAT|O_RDWR|O_TRUNC,00644);
		if(fd < 0) {
			return -1 ;
		}
		if(-1==lseek(fd,size,SEEK_SET) ) {
			close( fd );
			return -1 ;
		}
		if(-1==write(fd,"",1) ) {
			close( fd );
			return -1 ;
		}
	}

	out_handle->paddr =  mmap( NULL,size, PROT_READ|PROT_WRITE,MAP_SHARED,fd,0 );
	out_handle->size = size ;
	if(-1!=fd)
		close( fd );
	return 0 ;
}

int close_filemap(nd_filemap_t *mapinfo)
{
	if (mapinfo->paddr && mapinfo->size){
		int ret = munmap( mapinfo->paddr, mapinfo->size);
		mapinfo->paddr=0;
		return ret ;
	}
	return 0 ;	
}

int open_filemap(const char *filename, nd_filemap_t *out_handle)
{
	int fd,i;
	long start, end ;
	size_t size ;
	fd=open( filename,O_CREAT|O_RDWR,00644 );
	if(fd < 0) {
		return -1 ;
	}
	start = lseek( fd, 0L, SEEK_SET );
	end = lseek( fd, 0L, SEEK_END );
	if(start==-1 || end == -1) {
		close(fd) ;
		return -1;
	}
	size = end - start ;

	out_handle->paddr =  mmap( NULL,size, PROT_READ|PROT_WRITE,MAP_SHARED,fd,0 );
	out_handle->size = size ;
	if(-1!=fd)
		close( fd );
	return 0 ;
}

int nd_getcpu_num() 
{
	return sysconf(_SC_NPROCESSORS_ONLN);
}
#endif 
