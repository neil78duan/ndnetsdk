/* common.c
 * define nd common initilize /destroy function 
 *
 * version 1.0 
 * all right reserved by neil duan
 * 2007-10
 */

#include "nd_common/nd_common.h"
#include "nd_common/nd_mempool.h"
#include "nd_common/nd_os.h"

#ifdef ND_UNIX
#include <locale.h>
#endif
const char *__g_process_name = NULL;			//进程名字
static int __s_common_init = 0 ;
static ndtime_t __s_common_init_base_tick = 0;
static time_t	__s_common_init_base_time = 0;

const char *nd_process_name()
{
	static char s_proname[128] = {0};

	if(!__g_process_name) {
		return "ndengine" ;
	}
	else if(0==s_proname[0]) {
		size_t len = strlen(__g_process_name) ;
		const char *p= __g_process_name + len;
		char *desc =s_proname;
		while(len-- > 0){
			if(*p==0x2f || *p==0x5c) {
				++p ;
				break ;
			}
			--p ;
		}
		len = 128 ;
		while(*p && len-- > 0) {
			if(*p== '.')
				break ;
			*desc++ = *p++ ;
		}
		*desc = 0 ;
	}
	return s_proname ;
}


int nd_arg(int argc, const char *argv[])
{
	__g_process_name = argv[0] ;
	return 0;
}

ndbigtime_t	nd_bigtime(void)
{
	ndtime_t detla_tick = nd_time() - __s_common_init_base_tick;

	ndbigtime_t ret = (ndbigtime_t)__s_common_init_base_time * 1000 + (ndbigtime_t)detla_tick;

	return ret;
}

int nd_common_init()
{
	if (__s_common_init){
		return 0 ;
	}
#ifdef ND_DEBUG
	nd_setxml_log(nd_output) ;
#else
	nd_setxml_log(nd_default_filelog) ;
#endif

	//nd_logmsg("%s common init\n" AND nd_process_name() ) ;
#ifdef ND_UNIX
	setlocale(LC_CTYPE, "zh-CN.UTF-8");  //支持中文
#else 
	
#endif
	//nd_memory_init() ;
	//if(-1==nd_mempool_root_init() )  {
	//	return -1 ;
	//}
	nd_sourcelog_init() ;

	__s_common_init = 1 ;
	__s_common_init_base_tick = nd_time();
	__s_common_init_base_time = time(NULL);

	return 0 ;
}

void nd_common_release()
{
	destroy_object_register_manager() ;
	nd_sourcelog_dump();

	nd_mempool_root_release();
	//nd_memory_destroy() ;
	__s_common_init = 0;
}
int nd_common_isinit() 
{
	return __s_common_init  ;
}
const char * nd_common_machine_info(char buf[], size_t size)
{
	int len = 0;
	char *p = buf;

#if defined( __ND_WIN__)
	len = snprintf(buf, size, "MACH-%s", "win");
#elif defined( __ND_MAC__)
	len = snprintf(buf, size, "MACH-%s", "mac-");
#elif defined( __ND_IOS__)
	len = snprintf(buf, size, "MACH-%s", "ios-");
#elif defined( __ND_ANDROID__)
	len = snprintf(buf, size, "MACH-%s", "android-");
#elif defined( __ND_LINUX__)
	len = snprintf(buf, size, "MACH-%s", "linux-");
#else 
	len = snprintf(buf, size, "MACH-%s", "unknown-");
#endif 
	p += len;
	size -= len;
#if defined(X86_64)
	len = snprintf(p, size, "-%s", "x86-64");
#else 
	len = snprintf(p, size, "-%s", "32");
#endif 
	p += len;
	size -= len;

#ifdef ND_DEBUG
	len = snprintf(p, size, "-%s", "debug");
#else 
	len = snprintf(p, size, "-%s", "release");
#endif
	return buf;
}


