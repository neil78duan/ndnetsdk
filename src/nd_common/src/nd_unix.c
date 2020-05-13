/* file nd_unix.c
 * define function in windows platform
 * version  1.0 
 *
 * author : neil duan 
 * all right reserved  2007-9-27 
 */
 

#include "nd_common/nd_common.h"

#if defined(ND_UNIX)
#include <time.h>  
#include <sys/mman.h>
#include<sys/param.h>
#include <fcntl.h>
#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>

#include <semaphore.h>
#include <signal.h>
#include <sys/stat.h>
#include <dlfcn.h>

ndpid_t run_exec(const char * working_path,const char *path, char *plist[])
{
	int i = 0 ;
#ifndef __ND_ANDROID__
	for (i = 0; i < NOFILE; i++) {
		close(i);
	}
#endif
	if (working_path && *working_path) {
		nd_chdir(working_path);
	}
	//umask(0);
	
	setsid() ;
	execv(path, plist) ;
	nd_logerror("create process %s error %s [%s] working_path=[%s]\n", path, nd_last_error(),nd_getcwd(), working_path? working_path:"NULL") ;
	exit(errno) ;
	return  0;
}

ndpid_t nd_createprocess(const char *working_path, const char *path, ...)
{
	// get args
	int i = 0 ;
	const char *p ;
	ndpid_t pid = 0 ;
	va_list va;
	char *plist[32] ;

	plist[0] = (char*)nd_filename(path);
	i = 1;
	
	va_start(va, path);
	while((p = va_arg(va, const char*) )) {
		plist[i++] = (char*) p ;
		if(i>= 31 ){
			break ;
		}
	}
	va_end(va);
	
	plist[i] = NULL ;
	
	pid = fork() ;
	
	if(pid== 0) {
		char tmp_path[ND_FILE_PATH_SIZE];
		if (nd_path_is_relative(path)) {
			path = nd_absolute_filename(path,tmp_path,sizeof(tmp_path)) ;
		}
		return run_exec(working_path,path, plist) ;
	}
	else {
		return  pid ;
	}
	
}

void nd_terminate_process(ndpid_t pid)
{
	kill(pid, SIGINT);

}

const char *nd_get_sys_username(void)
{
	struct passwd *pwd = getpwuid(getuid());
	if(pwd){
		return pwd->pw_name;
	}
	else {
		nd_logerror("get system user name error :%s\n", nd_last_error()) ;
		return "unknow-user";
	}
}

HINSTANCE nd_dll_load(const char *dllpath)
{
	HINSTANCE h = dlopen(dllpath, RTLD_LAZY);
	if(!h) {
		nd_logerror("dlopen(%s) : %s\n", dllpath, dlerror()) ;
		return NULL ;
	}
	return h;
}
void nd_dll_unload(HINSTANCE hdll)
{
	dlclose(hdll);
}
void* nd_dll_entry(HINSTANCE hdll, const char *name)
{
	return (void*)dlsym(hdll, name);
}

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

#ifndef NOFILE
#define NOFILE 3 
#endif
void nd_init_daemon(void)
{
#if !defined(__ND_ANDROID__) && !defined(__ND_IOS__)
	pid_t pid;
	//int i;
	pid = fork();
	if(pid > 0){
		exit(0);
	}
	else if(pid < 0 ){
		perror("fork()\n");
		exit(1);
	}
	else if(pid == 0){
		int fd ,fdtablesize;
		setsid();
		chdir("/tmp");
		//umask(0);
		for (fd = 0, fdtablesize = getdtablesize(); fd < fdtablesize; fd++) {
			close(fd);
		}
		signal(SIGCHLD,SIG_IGN);
		
		nd_log_close_screen(1) ;
		return ;
	}
#endif
}

int _nd_sem_open(nd_sem_name_t *sem, unsigned int value)
{
#if defined(__ND_IOS__)
	nd_sem_name_t  psem = (nd_sem_name_t) malloc(sizeof(struct nd_name_sem) );
	psem->_sem = sem_open("/s", O_CREAT, 0644, 1);
	if (psem->_sem== SEM_FAILED) {
		if (errno != EEXIST) {
			nd_logerror("sem_open(%s) : %s\n",psem->_name, nd_last_error()) ;
			free(psem) ;
			return -1 ;
		}
	}
	*sem = psem ;
#elif defined(__ND_ANDROID__)
	nd_sem_name_t  psem = (nd_sem_name_t) malloc(sizeof(struct nd_name_sem) );
	psem->_sem =(sem_t*) malloc(sizeof(sem_t)) ;
	if( sem_init(psem->_sem, O_CREAT, value) ){
		nd_logerror("sem_open(%s) : %s\n",psem->_name, nd_last_error()) ;
		free(psem->_sem) ;
		free(psem) ;
		return -1;
	}
	
	*sem = psem ;
#else
	
	
	static ndatomic_t _s_sem_index = 0 ;
	//char sem_name[64] ;
	nd_sem_name_t  psem = (nd_sem_name_t) malloc(sizeof(struct nd_name_sem) );
	do {
		ndsnprintf(psem->_name, sizeof(psem->_name), "%s_sem_%d", nd_process_name(),  nd_atomic_inc( &_s_sem_index)) ;
		
		psem->_sem = sem_open( psem->_name, O_CREAT|O_EXCL, 0644, value );
		if (psem->_sem== SEM_FAILED) {
			
			if (errno != EEXIST) {
				nd_logerror("sem_open(%s) : %s\n",psem->_name, nd_last_error()) ;
				free(psem) ;
				return -1 ;
			}
			
		}
		
	}while (psem->_sem==SEM_FAILED && errno==EEXIST) ;
	*sem = psem ;
#endif
	
	return 0;
}

nd_sem_name_t _nd_sem_open_ex(const char *name, unsigned int value,int flag)
{
	
	nd_sem_name_t  psem =  NULL ;
	
#if defined(__ND_IOS__)
	sem_t *mysem = sem_open("/s", O_CREAT, 0644, value);
#elif defined(__ND_ANDROID__)
	sem_t *mysem =(sem_t*) malloc(sizeof(sem_t)) ;
	if( sem_init(mysem, O_CREAT|O_EXCL, value) ){
		nd_logerror("sem_open(%s) : %s\n",psem->_name, nd_last_error()) ;
		free(psem) ;
		return NULL;
	}
	
#else
	sem_t *mysem = sem_open( name, flag/*O_CREAT|O_EXCL*/, 0644, value );
#endif
	
	if (mysem == SEM_FAILED) {
		nd_logerror("sem_open(%s) : %s\n",name, nd_last_error()) ;
		return NULL;
	}
	psem = (nd_sem_name_t) malloc(sizeof(struct nd_name_sem) );
	if(!psem) {
		sem_close(mysem) ;
		sem_unlink(name) ;
		return NULL ;
	}
	
	psem->_sem = mysem ;
	ndstrncpy(psem->_name,name, sizeof(psem->_name)) ;
	
	return psem;
}


int _nd_sem_close(nd_sem_name_t sem)
{
	sem_close(sem->_sem) ;
#if  defined(__ND_IOS__)
#elif defined(__ND_ANDROID__)
	if (sem && sem->_sem) {
		free(sem->_sem) ;
	}
#else
	sem_unlink(sem->_name) ;
#endif
	free(sem) ;
	return  0;
}

#if defined(__ND_MAC__) || defined(__ND_IOS__)

void pthread_sleep(NDUINT32 msec)
{
    useconds_t t = msec * 1000;
    usleep(t) ;
}

int _unix_sem_timewait(nd_sem_name_t pSem , NDUINT32 waittime)
{
    sem_t *sem = pSem->_sem ;
    int times = waittime / 50 ;
    int ret ;
    if(ND_INFINITE==waittime) {
        ret = sem_wait(sem) ;
		if(ret ==-1) {
			nd_showerror() ;
		}
		return ret;
    }
    else if(times > 0){
        while (times) {
            --times ;
            ret = sem_trywait(sem) ;
            if (ret==-1 ) {
                if (errno==EAGAIN) {
					pthread_sleep(50) ;
                    continue ;
                }
                return NDSEM_ERROR ;
            }
            return NDSEM_SUCCESS;
        }
        return NDSEM_TIMEOUT ;
    }
    else {
        ret = sem_trywait(sem);
		if(ret==0) {
			return NDSEM_ERROR ;
		}
		else if (errno==EAGAIN) {
			pthread_sleep(50) ;
			return sem_trywait(sem); ;
		}
		
		return NDSEM_TIMEOUT ;
    }
}


#else

void pthread_sleep(NDUINT32 msec)
{
    pthread_cond_t _cond = PTHREAD_COND_INITIALIZER ;
    pthread_mutex_t _mutex = PTHREAD_MUTEX_INITIALIZER ;
    
    mythread_cond_timewait(&_cond,&_mutex, msec) ;
}


int _sys_sem_timewait(sem_t *sem , NDUINT32 waittime)
{
	int ret ;
	if(ND_INFINITE==waittime) {
		return sem_wait(sem) ;
	}
	else if(0== waittime){
		return sem_trywait(sem);
//		ret = sem_trywait(sem);
//		if (ret == -1) {
//			nd_showerror();
//		}
//		return ret;
	}
	else {		
		struct timespec ts ;
		
		if (nd_clock_gettime(&ts) == -1) {
			nd_showerror();
			return -1 ;
		}
		ts.tv_sec += waittime /1000 ;
		ts.tv_nsec += (waittime %1000 ) * 1000000;
		
		//check what happened
		ret = sem_timedwait(sem, &ts);
		if (ret == -1) {
			if (errno == ETIMEDOUT) {
				return NDSEM_TIMEOUT;
			}
			else {
				return NDSEM_ERROR;
			}
		}
		else {
			return NDSEM_SUCCESS;
		}
	}
	
}

int _unix_sem_timewait(nd_sem_name_t pSem , NDUINT32 waittime)
{
	return _sys_sem_timewait(pSem->_sem, waittime) ;
	
}

#endif


ndtime_t nd_time(void)
{
	struct timeval tmnow, deta;
	static struct timeval __start_time ;	//≥Ã–Ú∆Ù∂Ø ±º‰
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

//¥¥Ω®œﬂ≥Ã∫Ø ˝
ndth_handle nd_createthread(NDTH_FUNC func, void* param,ndthread_t *thid,int priority)
{
	pthread_t threadid ;
	
	//struct sched_param schparam ;
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
    //void *param = (void*)（ exitcode ）;
	void *param = 0 ;
	memcpy(&param, &exitcode, sizeof(exitcode)) ;
	pthread_exit( param );
}
//µ»¥˝“ª∏ˆœﬂ≥ÃµƒΩ· ¯
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
#ifdef __ND_ANDROID__
    return -1;
#else
	return pthread_cancel(handle) ;
#endif
    //return  0;
}

#include <sys/ioctl.h>
#include <termios.h>

#if defined(__ND_LINUX__) || defined(__ND_ANDROID__)
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
size_t set_maxopen_fd(size_t max_fd)
{
	struct rlimit rt={0};
	if(0!=getrlimit(RLIMIT_NOFILE, &rt) ) {
		return 0 ;
	}

	rt.rlim_max = max(rt.rlim_max ,max_fd) ;
	
	rt.rlim_cur = max(rt.rlim_cur ,max_fd) ;
	
    setrlimit(RLIMIT_NOFILE, &rt) ;    
    return  get_maxopen_fd() ;
}
size_t get_maxopen_fd(void)
{
	struct rlimit rt={0};
	if(0==getrlimit(RLIMIT_NOFILE, &rt) ) {
		return rt.rlim_max ;
	}
#if defined(__ND_LINUX__) || defined(__ND_ANDROID__)
	return 1024 ;
#else
	return 256;
#endif
}

int enable_core_dump(void)
{
	struct rlimit   limit;
	limit.rlim_cur = RLIM_INFINITY;
	limit.rlim_max = RLIM_INFINITY;
	return setrlimit(RLIMIT_CORE, &limit);
}

#define GET_RLIMIT_INFO(_name,_buf, size)  do {\
	struct rlimit   limit = {0};	\
	if( getrlimit(_name, &limit) == 0 && size > 0) {\
		int _len = ndsnprintf(_buf, size, "%s:cur=%llu max=%llu\n",  #_name, (NDUINT64)limit.rlim_cur, (NDUINT64)limit.rlim_max) ; 	\
		size -= _len ;				\
		_buf += _len ;				\
	} \
}while(0) 

char* get_rlimit_info(char *buf, int buf_size) 
{
	char *p = buf ;
#ifdef ND_UNIX
	GET_RLIMIT_INFO(RLIMIT_CORE, p, buf_size) ;
	GET_RLIMIT_INFO(RLIMIT_CPU,  p, buf_size) ;
	GET_RLIMIT_INFO(RLIMIT_DATA, p, buf_size) ;
	GET_RLIMIT_INFO(RLIMIT_FSIZE,p, buf_size) ;
	GET_RLIMIT_INFO(RLIMIT_MEMLOCK,p, buf_size) ;
	GET_RLIMIT_INFO(RLIMIT_NOFILE, p, buf_size) ;
	GET_RLIMIT_INFO(RLIMIT_NPROC,  p, buf_size) ;
	GET_RLIMIT_INFO(RLIMIT_RSS,    p, buf_size) ;
	GET_RLIMIT_INFO(RLIMIT_STACK,  p, buf_size) ;
	
#endif
	
	return buf;
}

int create_filemap(const char *filename, size_t size,nd_filemap_t *out_handle)
{
	int fd =-1,ret=0;
	//char temp;

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
    if ( MAP_FAILED==out_handle->paddr) {
        out_handle->paddr = NULL ;
        out_handle->size = 0 ;
        ret = -1 ;
    }
    else {
        out_handle->size = size ;
        ret = 0 ;
    }
    if(-1!=fd) {
		close( fd );
    }
    
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
	int fd,ret;
	long start, end ;
	size_t size ;
	fd=open( filename,/*O_CREAT|*/O_RDWR,00644 );
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
    
    if ( MAP_FAILED==out_handle->paddr) {
        out_handle->paddr = NULL ;
        out_handle->size = 0 ;
        ret = -1 ;
    }
    else {
        out_handle->size = size ;
        ret = 0 ;
    }
    if(-1!=fd) {
        close( fd );
    }

	return ret ;
}

int open_filemap_r(const char *filename, nd_filemap_t *out_handle)
{
	int fd,ret;
	long start, end ;
	size_t size ;
	fd=open( filename,/*O_CREAT|*/O_RDWR,00644 );
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

	out_handle->paddr =  mmap( NULL,size, PROT_READ,MAP_SHARED,fd,0 );

	if ( MAP_FAILED==out_handle->paddr) {
		out_handle->paddr = NULL ;
		out_handle->size = 0 ;
		ret = -1 ;
	}
	else {
		out_handle->size = size ;
		ret = 0 ;
	}
	if(-1!=fd) {
		close( fd );
	}

	return ret ;
}


int nd_mem_share_create(const char *name, size_t size, nd_filemap_t *map_handle)
{
	if (0 == open_filemap( name, map_handle) ){
		return 0 ;
	}
	if(size ==0 ) {
		return -1;
	}
	if(-1==create_filemap( name, size, map_handle) ) {
		return -1 ;
	}

	memset(map_handle->paddr, 0, map_handle->size) ;

	return 0 ;

}
int nd_mem_share_close( nd_filemap_t *map_handle)
{
	return close_filemap(map_handle) ;
}

int nd_getcpu_num(void) 
{
	return (int)sysconf(_SC_NPROCESSORS_ONLN);
}

#ifndef __ND_ANDROID__
#include <execinfo.h>

int nd_get_sys_callstack(char *buf, size_t size)
{
	void* callstack[128];
	int i, frames = backtrace(callstack, 128);
	char** strs = backtrace_symbols(callstack, frames);
	char *p= buf ;
	int ret = 0 ;
	for (i = 0; i < frames; ++i) {
		int len = ndsnprintf(p, (size - ret), "%s\n", strs[i]);
		ret += len ;
		p += len ;
	}
	free(strs);
	return ret;
}

int nd_sys_callstack_dump(nd_out_func func,FILE *outfile)
{
	void* callstack[128];
	int i, frames = backtrace(callstack, 128);
	char** strs = backtrace_symbols(callstack, frames);
	int ret = 0 ;
	for (i = 0; i < frames; ++i) {
		
		int len = func(outfile, "%s\n",strs[i] ) ;
		ret += len ;
	}
	free(strs);
	return ret;
}
#else

int nd_get_sys_callstack(char *buf, size_t size)
{
	return 0;
}

int nd_sys_callstack_dump(nd_out_func func,FILE *outfile)
{
	
	return 0;
}
#endif

#endif
