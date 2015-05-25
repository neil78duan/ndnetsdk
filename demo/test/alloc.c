/* file alloc.c
 * test memory alloc
 *
 * 2007-10
 * neil
 */

#include "nd_common/nd_alloc.h"
#include "nd_common/nd_common.h"

#define _TSIZE 1024

#define BIG_GRANULARITY	1024
int malloc_func(void *p_addr[], int num)
{
	size_t sz =0;
	int i =0;

	for(i=0; i<num; i++) {
		/*
		if(i%2) {
			sz =1 + rand()%1000 ;

			sz *= BIG_GRANULARITY ;
		}
		else */{
			sz = rand() + 16 ;
			sz = sz & 0xffff ;
		}
		
		p_addr[i]=malloc(sz) ;
		if(p_addr[i]){
			memset(p_addr[i], 0, sz) ;
			*(size_t*)p_addr[i] = sz ;
		}		
		nd_threadsched() ;
	ndprintf("%d %zu alloc success %p !\n", i, sz, p_addr[i]) ;
	}
	return i ;
	
}

int write_buf(void *p_addr[], int num) 
{
	int i ;
	size_t sz ;
	for (i=0; i<num; i++)
	{
		if(!p_addr[i]) 
			continue ;
		sz = *(size_t*) p_addr[i] ;
		memset(p_addr[i], 'a', sz) ;
		*(size_t*)p_addr[i] = sz ;
	ndprintf("%d write %zu memory success!\n", i, sz ) ;
	}
	return i ;
}

int free_func(void *p_addr[], int num)
{
	int i ;
	size_t sz ;
	for (i=0; i<num; i++)
	{
		int n ;
		if(!p_addr[i]) 
			continue ;
        ndprintf("%d %p  free memory !\n",i, p_addr[i]  ) ;

		sz = *(size_t*) p_addr[i] ;
		for (n=sizeof(sz); n<sz; n++)
		{
			nd_assert( *(((char*)p_addr[i]) +n) =='a') ;
			if(*(((char*)p_addr[i]) +n) !='a') {
				abort() ;
			}
		}
		free(p_addr[i] ) ;
	//ndprintf("%d %zu  free memory success !\n",i, sz ) ;
	
	}
	return i ;
}

void run_test_mm()
{
	int i;
	for (i=0; i<20; i++)
	{
		void *p_addr[_TSIZE] = {0} ;
		malloc_func(p_addr,_TSIZE) ;
		write_buf(p_addr,_TSIZE) ;
		free_func(p_addr,_TSIZE) ;
		nd_threadsched() ;
	}
	
	ndprintf("%d test memory over !\n", nd_thread_self() ) ;
}

void *alloc_entry(void *param)
{
	run_test_mm();
	return NULL ;
}

#define _TMAX_TH 1
int alloc_test()
{

	int i ;
	ndthread_t thid[_TMAX_TH];
	ndth_handle h[_TMAX_TH] ;
	
	char *p ;
	p = malloc(100) ;
	if(p) {
		free(p) ;
	}
	p= malloc(100) ;
	if(p){
		free(p) ;
	}

	for (i=0; i<_TMAX_TH;i++){
		h[i] = nd_createthread(alloc_entry, NULL, &thid[i], 0) ;
	}
	
	
	for (i=0; i<_TMAX_TH;i++){
		nd_waitthread(h[i]) ;
	}
	
	ndprintf(_NDT("malloc test complete please check ndlog.log \n"));
	
	//getch() ;
	return 0;
}
//////////////////////////////////////////////////////////////////////////
//test mempool

int run_test_pool()
{
	int i ;
	void *p  ;
	size_t size = rand() % 4096 + 1 ;
	size_t total_size ;
	nd_handle pool =  nd_pool_create(EMEMPOOL_HUGE, "testalloc") ;			//创建一个内存池,返回内存池地址
	
	if(!pool) {
		printf("create pool error\n") ;
		return  1;
	}

	for (i=0; i<100; i++)
	{	
		total_size = 0 ;
		while( (p=nd_pool_alloc(pool,size))!=NULL) {
			total_size += size ;
			memset(p, 'b', size) ;
			size = rand() % 4096 + 1 ;
		}
//		size = nd_pool_freespace( pool );
//		if(size) {
//			p=nd_pool_alloc(pool,size) ;
//			if(p)memset(p, 'b', size) ;
//			total_size += size ;
//		}
		printf("total alloc size = %d\n", total_size) ;
		nd_pool_reset(pool) ;
	}
	//nd_pool_destroy(pool) ;
	nd_object_destroy(pool,0) ;
	return 0 ;

}

void *pool_entry(void *param)
{
	if(-1== run_test_pool() ) {
		return (void*)1;
	}
	return NULL ;
}

int pool_test()
{

	int i ;
	ndthread_t thid[_TMAX_TH];
	ndth_handle h[_TMAX_TH] ;
	
	for (i=0; i<_TMAX_TH;i++){
		h[i] = nd_createthread(pool_entry, NULL, &thid[i], 0) ;
	}
	
	
	for (i=0; i<_TMAX_TH;i++){
		nd_waitthread(h[i]) ;
	}
	
	ndprintf(_NDT("malloc test complete please check ndlog.log \n"));
	
	//getch() ;
	return 0;
}

//////////////////////////////////////////////////////////////////////////
// test big block memory alloc/free

//test static alloc
int static_alloc_test()
{
	char *p;
	char *p_buf[100] ; 
	int i ;
	nd_handle h = nd_sa_create(100, 32, NULL) ;
	if(!h) {
		printf("create static pool error\n") ;
		return  1;
	}

	for(i=0; i<100; i++) {
		p = nd_sa_alloc(h) ;
		nd_assert(p) ;
		ndsprintf(p, "hello world %d", i) ;
		p_buf[i] = p ;
	}
	p = nd_sa_alloc(h) ;
	nd_assert(p==NULL) ;
	nd_assert(nd_sa_freenum(h)==0) ;

	for(i=0; i<100; i++) {
		nd_sa_free(p_buf[i],h) ;
	}

	nd_assert(nd_sa_freenum(h)==nd_sa_capacity(h)) ;

	return nd_sa_destroy(h,0);
}

int test_alloc()
{
    alloc_test();
    //pool_test();
    static_alloc_test();
    return  0;
}
