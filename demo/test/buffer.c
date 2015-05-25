/* file : buffer.c
 * test nd_recbug and nd_linebuf
 *
 * 2007-10
 */

#include "nd_common/nd_common.h"

#define _READ_SIZE 128

#define _INPUT_FILE "client_test"
#define _OUTPUT_FILE "client_test-test_buf"

int cbuf_test()
{
	char readBuf[_READ_SIZE] = {0};
	char buf1[_READ_SIZE]= {0} ;
	int nReadNum ;
	int nNeedRead = rand() %(_READ_SIZE-10) +1 ;

	FILE *pf = fopen(_INPUT_FILE, "r+b") ;
	FILE *wf = fopen(_OUTPUT_FILE, "w+b") ;
	
	ndrecybuf_t   g_cbuf ;

	ndcbuf_init(&g_cbuf);
	
	if(!pf||!wf) {
		printf("open file error!\n") ;
		return -1;
	}
	while(nReadNum=fread(readBuf,1,nNeedRead,pf)) {
		ndcbuf_write(&g_cbuf,readBuf,nReadNum,EBUF_SPECIFIED) ;
		nNeedRead = rand() % nReadNum ;
		if(nNeedRead==0)
			nNeedRead=nReadNum ;
REREAD:
		memset(buf1,0x7f,sizeof(buf1)) ;
		nReadNum = ndcbuf_read(&g_cbuf,buf1,nNeedRead,EBUF_ALL) ;

		fwrite(buf1,nReadNum,1,wf) ;
		nNeedRead=ndcbuf_freespace(&g_cbuf); //error in here
		if(nNeedRead==0) {
			nNeedRead = ndcbuf_datalen(&g_cbuf) ;
			nNeedRead = min(nNeedRead,_READ_SIZE) ;
			goto REREAD;
		}
		else {
			int tmp = rand() %(_READ_SIZE-10) +1 ;
			nNeedRead = min(nNeedRead,tmp);
		}
	}
	while(nNeedRead=ndcbuf_datalen(&g_cbuf) ) {
		char buf_end[C_BUF_SIZE] ;
		nReadNum = ndcbuf_read(&g_cbuf,buf_end,nNeedRead,EBUF_ALL) ;
		fwrite(buf_end,nReadNum,1,wf) ;
	}
	
	fclose(pf) ;
	fclose(wf) ;
	return 0;
}

#define _LBUF_SIZE 1024
int lbuf_test()
{
	int nReadNum = rand() %(_READ_SIZE-10) +1 ;
	int i = 0,nFree;
	char *paddr ;
	struct nd_linebuf lbuf ;

	FILE *pf = fopen(_INPUT_FILE, "r+b") ;
	FILE *wf = fopen(_OUTPUT_FILE, "w+b") ;
	
	if(!pf||!wf) {
		printf("open file error!\n") ;
		return -1;
	}
	
	ndlbuf_init(&lbuf, sizeof(lbuf.__buf));
	
	paddr = ndlbuf_addr(&lbuf) ;

	while(nReadNum=fread(paddr,1,nReadNum,pf)) {
		ndlbuf_add_data(&lbuf,nReadNum) ;
		nReadNum = ndlbuf_datalen(&lbuf) ;

		if(nReadNum>_LBUF_SIZE){
			nReadNum = _LBUF_SIZE ;
			paddr = ndlbuf_data(&lbuf) ;
			if(fwrite(paddr,nReadNum,1,wf) > 0)
				ndlbuf_sub_data(&lbuf,nReadNum) ;
		}
		nReadNum = rand() %(_READ_SIZE-10) +1 ;
		nFree = ndlbuf_freespace(&lbuf) ;
		//if(nFree){
			nd_assert(nFree) ;
		//}
		nReadNum = min(nReadNum,nFree) ;
		paddr = ndlbuf_addr(&lbuf);
	}
	nReadNum = ndlbuf_datalen(&lbuf) ;
	if (nReadNum){
		paddr = ndlbuf_data(&lbuf) ;
			if(fwrite(paddr,nReadNum,1,wf) > 0)
				ndlbuf_sub_data(&lbuf,nReadNum) ;
	}
	fclose(pf) ;
	fclose(wf) ;
	return 0;
}


int lbuf_test2()
{
	char readBuf[_READ_SIZE] = {0};
	char buf1[_READ_SIZE]= {0} ;
	int nReadNum ;
	int nNeedRead = rand() %(_READ_SIZE-10) +1 ;

	FILE *pf = fopen(_INPUT_FILE, "r+b") ;
	FILE *wf = fopen(_OUTPUT_FILE, "w+b") ;
	
	struct nd_linebuf   g_cbuf ;

	ndlbuf_init(&g_cbuf,sizeof(g_cbuf.__buf));
	
	if(!pf||!wf) {
		printf("open file error!\n") ;
		return -1;
	}
	while(nReadNum=fread(readBuf,1,nNeedRead,pf)) {
		ndlbuf_write(&g_cbuf,readBuf,nReadNum,EBUF_SPECIFIED) ;
		nNeedRead = rand() % nReadNum ;
		if(nNeedRead==0)
			nNeedRead=nReadNum ;
REREAD:
		memset(buf1,0x7f,sizeof(buf1)) ;
		nReadNum = ndlbuf_read(&g_cbuf,buf1,nNeedRead,EBUF_ALL) ;

		fwrite(buf1,nReadNum,1,wf) ;
		nNeedRead=ndlbuf_freespace(&g_cbuf); //error in here
		if(nNeedRead==0) {
			nNeedRead = ndlbuf_datalen(&g_cbuf) ;
			nNeedRead = min(nNeedRead,_READ_SIZE) ;
			goto REREAD;
		}
		else {
			int tmp = rand() %(_READ_SIZE-10) +1 ;
			nNeedRead = min(nNeedRead,tmp);
		}
	}
	while(nNeedRead=ndlbuf_datalen(&g_cbuf) ) {
		char buf_end[C_BUF_SIZE] ;
		nReadNum = ndlbuf_read(&g_cbuf,buf_end,nNeedRead,EBUF_ALL) ;
		fwrite(buf_end,nReadNum,1,wf) ;
	}
	
	fclose(pf) ;
	fclose(wf) ;
	return 0;
}