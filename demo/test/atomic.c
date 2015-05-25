/* file : atomic.c
 * test nd atomic operation
 * 2007-10
 */

#include "nd_common/nd_common.h"
/*

test the following routine

long nd_compare_swap(ndatomic_t *desc,ndatomic_t cmp, ndatomic_t exch);
void nd_atomic_inc(ndatomic_t *p);
void nd_atomic_dec(ndatomic_t *p) ;	
void nd_atomic_add(ndatomic_t *p,int step) ;
void nd_atomic_sub(ndatomic_t *p,int step);
int  nd_atomic_swap(ndatomic_t *p, ndatomic_tval);
int nd_testandset(ndatomic_t *p);
	
*/

ndfastlock_t _lock ;
int _atomic_op = 0 ;
int _sum = 0 ;
void *func(void *param)
{
	int ret =0;
	int i =0;
	//for(i=0; i<10; i++) {
	//	nd_atomic_swap(&_atomic_op,i) ;
	//	fprintf(stderr, "%d : ret = %d value=%d\n",i, ret, _atomic_op) ;
	//	fflush(stderr);
	//}
	//ret = nd_testandset(&_atomic_op) ;
	printf("start %d\n", nd_thread_self()) ;
	for(i=0; i<100000; i++) {
		if(nd_flock(&_lock)) {
			_sum += 10 ;
			_sum -=2 ;_sum-=8 ;
			//printf("run here %d\n", nd_thread_self()) ;
			nd_funlock(&_lock);
		}
		nd_threadsched() ;
		//nd_sleep(10) ;
		//nd_atomic_increment(&_atomic_op) ;
				
	}
	
	//printf("%d : ret = %d value=%d\n",i, ret, _atomic_op) ;
	return (void *)0 ;
	
}

void *func1(void *param)
{
	//int ret ;
	int i =0;
	//for(i=0; i<10; i++) {
	//	ret = nd_atomic_swap(&_atomic_op,i) ;
	//	printf("%d : ret = %d value=%d\n",i, ret, _atomic_op) ;
	//}
	//ret = nd_testandset(&_atomic_op) ;
	printf("start %d\n", nd_thread_self()) ;
	for(i=0; i<1000000; i++) {
		if(nd_flock(&_lock)) {
			//printf("func1 run here %d\n", nd_thread_self()) ;
			_sum -=50 ;_sum-=40 ;_sum-=10 ;
			_sum+=100 ;
			nd_flock(&_lock);
			//_atomic_swap(&_atomic_op, 0);
		}

		nd_threadsched() ;
		//nd_sleep(10) ;
		//nd_atomic_increment(&_atomic_op) ;
	}
	
	//printf("%d : ret = %d value=%d\n",i, ret, _atomic_op) ;
	return (void *)0 ;
	
}

void _test_atomic()
{

	int i ;

	int ret = 0 ;
	ndatomic_t a= 0 ,cmp, exch ,old;

	//test and set
	ret = nd_testandset(&a) ;
	printf("TEST T_AND_S :\n\t after test_and_set ret = %d a = %d ret =%d t_and_a test %s\n" , 
		ret , a ,ret,(a==1 && 0==ret)? "success": "failed") ;
	
	a = 5000, cmp =5000; exch = 4000;
	old = a ;
	printf("CMP_SWAP SUCCESS TEST: \n\t before compare and swap a=%d cmp=%d exch=%d \n", a, cmp, exch);
	ret = nd_compare_swap(&a, cmp, exch) ;
	printf("\t after compare and swap a=%d cmp=%d exch=%d ,ret=%d\n", a, cmp, exch,ret);
	printf("\t compare and swaped test %s! \n" , (exch==a && ret) ? "success":"failed" ) ;

	a = 5000, cmp =5001; exch = 4000;
	old = a ;
	printf("CMP_SWAP FAILED TEST: \n\t before compare and swap a=%d cmp=%d exch=%d  \n", a, cmp, exch);
	ret = nd_compare_swap(&a, cmp, exch) ;
	printf("\t after failed compare and swap a=%d cmp=%d exch=%d ret=%d\n", a, cmp, exch,ret);
	printf("\t compare and swaped test %s! \n" , (exch==a && ret) ? "failed":"success" ) ;

	a= 100 ; exch=21 ; 
	old = a;
	printf("TEST ATOMIC ADD:\n\t before test a=%d step=%d\n", a, exch) ;
	ret = nd_atomic_add(&a, exch) ;
	printf("\tafter atomic_add a=%d ret=%d test %s\n", a ,ret,(ret==old && a==(ret+exch))?"success":"failed") ;

	a = 0 ;
	cmp = 100 ;
	printf("TEST SWAP:\n\t before test a=%d  exch=%d \n", a, exch);
	
	old = a ;
	ret = nd_atomic_swap(&a,exch )  ;
	printf("\t after atomic swap a=%d exch=%d ret=%d test %s\n", a, exch,ret, (a==exch && ret==old)?"success" : "failed") ;


	//test inc
	printf("test atomic_inc dec add \n");
	a = 200 ;
	old = a ;
	ret = nd_atomic_inc(&a) ;
	printf("\t before inc a=%d\n\t after test a= %d ret = %d test %s\n" ,old, a, ret, (ret==old+1)?"success" : "failed") ;
	

	nd_atomic_set(&a,0) ;
	for(i=0; i<1000; i++) {
		nd_atomic_inc(&a) ;
	}
	printf("\tafter inc %d times a=%d test %s \n", i, a, i==a? "success" : "failed") ;
	
	for(; i>0; i--) {
		nd_atomic_dec(&a) ;
	}
	printf("\tafter dec %d times a=%d test %s \n", i, a, i==a? "success" : "failed") ;

	cmp = 100 ;
	a = 90 ;
	old = a ;
	printf("\t before add a=%d val=%d \n", a, cmp) ;
	nd_atomic_add(&a,cmp) ;
	printf("\t after atomic_add a=%d test %s \n", a, (a==old+cmp)?"success":"failed") ;

	printf("atomic test END!\n" ) ;
}

/*
long nd_compare_swap(ndatomic_t *desc,ndatomic_t cmp, ndatomic_t exch);
void nd_atomic_inc(ndatomic_t *p);
void nd_atomic_dec(ndatomic_t *p) ;	
void nd_atomic_add(ndatomic_t *p,int step) ;
void nd_atomic_sub(ndatomic_t *p,int step);
int  nd_atomic_swap(ndatomic_t *p, ndatomic_tval);
*/
int test_atomic()
{

	
	ndthread_t thid ,thid1;
	ndth_handle h ,h1 ;
	
	_test_atomic() ;
	
	h = nd_createthread(func, NULL, &thid, 0) ;
	
	h1 = nd_createthread(func1, NULL, &thid1, 0) ;
	
	if(h)
		nd_waitthread(h) ;
	if(h1)
		nd_waitthread(h1) ;	
	
	
	
	ndprintf(_NDT("fast lockend \n\ttest ok %d!, at = %d\nPress ANYKEY to continue!"),_sum,_atomic_op );
	nd_assert(_sum==0);

	//getch() ;
	return 0;
}
