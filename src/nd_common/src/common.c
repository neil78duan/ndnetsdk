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
char *__g_process_name = NULL;			//进程名字
static int __s_common_init = 0 ;
static ndtime_t __s_common_init_base_tick = 0;
static time_t	__s_common_init_base_time = 0;

char *nd_process_name()
{
	static char s_proname[128] = {0};

	if(!__g_process_name) {
		return "nd_engine" ;
	}
	else if(0==s_proname[0]) {
		size_t len = (int) strlen(__g_process_name) ;
		char *p= __g_process_name + len;
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
/*

ndchar_t *nd_process_name() 
{
#ifdef ND_UNICODE
	int required ;
	size_t size;
	char *p = process_name();
	static ndchar_t s_proname[128] = {0} ;
	required = mbstowcs(NULL, p, 0);

	size = mbstowcs( s_proname, p, required+1);
	if (size == (size_t) (-1))
		return _NDT("ndengine");

	return s_proname ;

#else 
	return process_name();
#endif
	
}*/

int nd_arg(int argc, char *argv[])
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
	nd_setxml_log(nd_output) ;
	NDTRAC("common init\n") ;
#ifdef ND_UNIX
	setlocale(LC_CTYPE, "zh-CN.UTF-8");  //支持中文
#else 
	
#endif
	//nd_memory_init() ;
	if(-1==nd_mempool_root_init() )  {
		
		return -1 ;
	}
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
