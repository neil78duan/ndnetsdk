/* file mutex.c
 * test mutex 
 *
 * 2007-10
 */

/* 这里需要重新编写测试程序,生产者从文件读取数据,消费者把数据写入文件,最后检测文件是否出错*/
#include "nd_common/nd_alloc.h"
#include "nd_common/nd_common.h"

#define READ_FILE "client_test"
#define WRITE_FILE "client_test-out"

#define SUMSIZE 100000000
#define BUFSIZE 8

int sum = 0;
nd_cond slots ;
nd_cond items ;
nd_mutex slot_lock ;
nd_mutex item_lock ;
int nslots = BUFSIZE;
int producer_done = 0;
int nitems = 0;

#define CALC(n) (n)		//执行的计算式

static nd_mutex buffer_lock ;

void Init_condTest()
{
	nd_cond_init(&slots) ;
	nd_cond_init(&items) ;
	nd_mutex_init(&slot_lock) ;
	nd_mutex_init(&item_lock) ;
	
	nd_mutex_init(&buffer_lock) ;
}


#define BUFSIZE 8
static char buffer[BUFSIZE] ;
static int bufin = 0 ;
static int bufout = 0 ;


void get_item(char *itemp) 
{
	nd_mutex_lock(&buffer_lock);
		*itemp = buffer[bufout];
		bufout = (bufout+1)%BUFSIZE ;
	nd_mutex_unlock(&buffer_lock) ;
}
void put_item(char item) 
{
	nd_mutex_lock(&buffer_lock) ;
		buffer[bufin] = item;
		bufin = (bufin+1)%BUFSIZE ;
	nd_mutex_unlock(&buffer_lock) ;
}

static void *producer(void *arg1)
{
	int nReadNum;
	char readbuf[1] ;
	FILE *pf = fopen(READ_FILE, "r+b") ;
	if(!pf) {
		printf("open file error!\n") ;
		return NULL;
	}
	while(nReadNum=fread(readbuf,1,1,pf)){
		nd_mutex_lock(&slot_lock) ;
		while(nslots<=0) 
			nd_cond_wait(&slots,&slot_lock) ;
		nslots-- ;
		nd_mutex_unlock(&slot_lock) ;
		put_item(readbuf[0]) ;
		
		nd_mutex_lock(&item_lock) ;
			nitems++ ;
			nd_cond_signal(&items) ;
		nd_mutex_unlock(&item_lock) ;
	}
	nd_mutex_lock(&item_lock) ;
		producer_done = 1 ;
		nd_cond_broadcast(&items) ;
	nd_mutex_unlock(&item_lock) ;

	
	fclose(pf) ;
	return 0 ;
}
static  void* consumer(void *arg2) 
{
//	int myitem ;
	char readbuf[1] ;
	FILE *wf = fopen(WRITE_FILE, "w+b") ;
	if(!wf) {
		printf("open file error!\n") ;
		return NULL ;
	}
	for( ; ; ){
		nd_mutex_lock(&item_lock) ;
			while((nitems<=0) && !producer_done)
				nd_cond_wait(&items,&item_lock) ;
			if((nitems<=0) && producer_done) {
				nd_mutex_unlock(&item_lock) ;
				break ;
			}
			nitems-- ;
		nd_mutex_unlock(&item_lock) ;
		get_item(readbuf) ;
//		nd_atomic_add(&sum,myitem) ;
		//sum += myitem ;
		fwrite(readbuf,1,1,wf) ;

		nd_mutex_lock(&slot_lock) ;
			nslots++ ;	
			nd_cond_signal(&slots) ;
		nd_mutex_unlock(&slot_lock) ;
	}
	
	fclose(wf) ;
	return (void*)0 ;
}

//测试函数
int mutex_test() 
{
	int ret ;
	ndthread_t produce_id, consumer_id;
	ndth_handle hProducer,hconsumer;
	Init_condTest() ;
	hProducer = nd_createthread(producer, NULL, &produce_id, 0) ;
	nd_assert(hProducer) ;
	hconsumer = nd_createthread(consumer, NULL, &consumer_id, 0) ;
	nd_assert(hconsumer) ;

	ret = nd_waitthread(hProducer) ;
	nd_assert(ret==0) ;
	ret = nd_waitthread(hconsumer) ;
	nd_assert(ret==0) ;
	ND_RUN_HERE() ;
	return 0 ;
	
}
