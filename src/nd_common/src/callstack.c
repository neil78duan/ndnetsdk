/* file callstack.c
 * 
 * log function call stack 
 *
 * all right reserved by neil duan
 */


#include "nd_common/nd_comcfg.h"
#include "nd_common/nd_os.h"
#include "nd_common/nd_dbg.h"

#include <stdlib.h>
#include <stdio.h>

#define MAX_THREAD_NUM 32  //function call stacks of threads 
#define MAX_CALLSTACK_SIZE 100
#define FUNCTION_NAME_SIZE 64


#if defined(ND_UNIX) 

extern	int create_filemap( const char *filename, size_t size,nd_filemap_t *out_handle)  ;
#else 
extern	int create_filemap( const char *filename,char *map_name, size_t size,nd_filemap_t *out_handle)  ;
#endif
extern	int close_filemap(nd_filemap_t *mapinfo) ;
extern	int open_filemap(const char *filename, nd_filemap_t *out_handle)  ;

typedef char functionname_t[FUNCTION_NAME_SIZE] ;
struct callstack_info
{
	ndatomic_t th_id ;
	int stack_point ;
	functionname_t names[MAX_CALLSTACK_SIZE];
};

#define CALLSTACK_MAP_SIZE (MAX_THREAD_NUM * sizeof(struct callstack_info) )

static nd_filemap_t g_callstack_map , g_monitor;


int nd_callstack_init(const char *filename)
{
#if defined(ND_UNIX) 
	if(-1==create_filemap( filename, CALLSTACK_MAP_SIZE,&g_callstack_map) ) {
		return -1 ;
	}
#else 
	if(-1==create_filemap(NULL, filename, CALLSTACK_MAP_SIZE,&g_callstack_map) ) {
		return -1 ;
	}

#endif
	memset(g_callstack_map.paddr, 0, g_callstack_map.size) ;

	return 0 ;

}

int nd_callstack_end() 
{
	if (g_callstack_map.paddr){
		return close_filemap(&g_callstack_map) ;
	}
	return 0;
}

int nd_callstack_monitor_init(const char *filename)
{
	return open_filemap( filename, &g_monitor)  ;
}


int nd_callstack_monitor_end() 
{
	if (g_monitor.paddr){
		return close_filemap(&g_monitor) ;
	}
	return 0;
}


int push_func(const char *funcname)
{
	if (g_callstack_map.paddr){
		int i ;
		ndatomic_t self_id =(ndatomic_t) nd_thread_self() ;
		struct callstack_info *pcs =(struct callstack_info *) g_callstack_map.paddr ;

		for(i=0; i<MAX_THREAD_NUM; i++, pcs++) {
			if (self_id==nd_atomic_read(&pcs->th_id)) {
				if (pcs->stack_point < MAX_CALLSTACK_SIZE) {
					strncpy(pcs->names[pcs->stack_point++],funcname, sizeof(functionname_t)) ;
					return 0;
				}
				return -1;
			}
		}
		pcs =(struct callstack_info *) g_callstack_map.paddr ;
		for(i=0; i<MAX_THREAD_NUM; i++, pcs++) {
			if (nd_compare_swap(&pcs->th_id,0, self_id)) {
				strncpy(pcs->names[0],funcname, sizeof(functionname_t)) ;
				pcs->stack_point = 1 ;
				return 0 ;
			}
		}
	}
	return -1;
}

void pop_func()
{
	if (g_callstack_map.paddr){
		int i ;
		ndatomic_t self_id =(ndatomic_t) nd_thread_self() ;
		struct callstack_info *pcs =(struct callstack_info *) g_callstack_map.paddr ;

		for(i=0; i<MAX_THREAD_NUM; i++, pcs++) {
			if (self_id==nd_atomic_read(&pcs->th_id)) {
				--(pcs->stack_point) ;
				return  ;
			}
		}
	}
}


char *nd_get_callstack_desc(char *buf, size_t size) 
{
	char *p =NULL ;
	ndatomic_t self_id ;
	int i ,n;
	size_t len;
	struct callstack_info *pcs =(struct callstack_info *) g_callstack_map.paddr ;

	if (!buf || 0==size || !g_callstack_map.paddr){
		return NULL;
	}


	self_id =(ndatomic_t) nd_thread_self() ;
	len = size ;
	buf[0] = 0 ;
	for(i=0; i<MAX_THREAD_NUM; i++, pcs++) {
		if (nd_atomic_read(&pcs->th_id) == self_id) {
			p = buf ;
			for(n=pcs->stack_point-1;n>=0 ; --n){				
				if (0==pcs->names[n][0]){
					break ;
				}
				else {
					size_t ret = snprintf(p, size-(p-buf), "%s();", pcs->names[n]) ;
					if(ret <=0 )
						break ;
					p += ret ;
					if (p>= buf + size)
						break ;
				}
				
			}
			p= buf ;
		}
	}
	return p;
}
int nd_callstack_monitor_dump(nd_out_func func,FILE *outfile)
{
	if (g_monitor.paddr) {

		int i ,n;
		struct callstack_info *pcs =(struct callstack_info *) g_monitor.paddr ;

		for(i=0; i<MAX_THREAD_NUM; i++, pcs++) {
			if (nd_atomic_read(&pcs->th_id) != 0) {
				func(outfile,"thread =%d call stack :\n {\n", (int) pcs->th_id) ;
				for(n=0;n <pcs->stack_point ; n++){
					func(outfile,"\t%d: %s\n", n, pcs->names[n]) ;
				}
				func(outfile,"}\n", (int) pcs->th_id) ;
			}
		}
		return 0;
	}
	return -1;
}
