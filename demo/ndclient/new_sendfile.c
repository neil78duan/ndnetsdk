/* file new_sendfile.c
 * test client newly send file
 *
 * neil duan
 * 2008-8-14 
 */

#include "nd_common/nd_common.h"

#if defined(ND_DEBUG)
#pragma comment(lib,"nd_cliapp_dbg.lib")
#else
#pragma comment(lib,"nd_cliapp.lib")
#endif

#include "nd_cliapp/nd_cliapp.h"
#include <time.h>

nd_handle _h_connect ;
int __exit ;

char *__send_file, *__recv_file ;
#define _INPUT_FILE __send_file
#define _OUTPUT_FILE __recv_file
#define _READ_SIZE  4000


int msg_entry(nd_handle connect_handle , nd_usermsgbuf_t *msg, nd_handle h_listen)
{
	static FILE *wf ;
	
	ndmsgparam_t param = ND_USERMSG_PARAM(msg) ;
		
	int data_len = ND_USERMSG_LEN(msg) - ND_USERMSG_HDRLEN ;

	char *data = ND_USERMSG_DATA(msg) ;

	if(data_len>0 && 1==param) {
		nd_assert(wf);
		fwrite(data,data_len,1,wf) ;

	}
	else if(2==param){
		fflush(wf);
		fclose(wf);
		wf = 0 ;
		__exit = 1;
	}
	else if(3==param) {
		wf=fopen(data, "w+b") ;
		nd_assert(wf);
	}
	return 0 ;
}


int send_wait(nd_handle connect_handle,nd_usermsgbuf_t *msg_buf)
{
	int ret,send_len;
	nd_usermsgbuf_t msg_recv;

RE_SEND:
	send_len = 	nd_connectmsg_post(connect_handle,msg_buf)  ;		
	nd_connector_update(connect_handle,10) ;
	/*memset(&msg_recv,0,sizeof(msg_recv)) ;
	ret = nd_connector_waitmsg(connect_handle, (nd_packetbuf_t *)&msg_recv,50);
	if(ret > 0) {			
		//msg_entry(connect_handle, &msg_recv) ;
		nd_translate_message(connect_handle, (nd_packhdr_t*)&msg_recv,NULL) ;
	}*/
	if(send_len <= 0)
		goto RE_SEND ;

	/*
	else if(-1==ret) {
		ndprintf(_NDT("closed by remote ret = 0\n")) ;
		return -1 ;
	}
	else {
		ndprintf(_NDT("wait time out ret = %d\npress any key to continue\n"), ret) ;
		return 0;		
	}
	*/
	return send_len;
}

int send_file(nd_handle connect_handle)
{
	int _send_times= 0;
	int ret = 0;
	nd_usermsgbuf_t msg_buf;

	int nReadNum = rand() %(_READ_SIZE-10) +1 ;
	char *paddr ;

	FILE *pf = fopen(_INPUT_FILE, "r+b") ;
	
	if(!pf) {
		printf("open file error!\n") ;
		return -1;
	}

	//send file name
	//MSGUI_MSGID(&msg_buf) = TEST_ECHO_ID ;
	ND_USERMSG_MAXID(&msg_buf) = MAXID_SYS ;
	ND_USERMSG_MINID(&msg_buf) = SYM_ECHO ;
	paddr = ND_USERMSG_DATA(&msg_buf) ;
	
	ndstrcpy(paddr,_OUTPUT_FILE) ;
	ND_USERMSG_LEN(&msg_buf) =  ND_USERMSG_HDRLEN + ndstrlen(_OUTPUT_FILE)+1 ;
	ND_USERMSG_PARAM(&msg_buf) = 3 ;
	
	ret = send_wait(connect_handle, &msg_buf) ;
	if(ret<0)
		return -1 ;
	//nd_connector_send(connect_handle,&msg_buf,ESF_URGENCY) ;
	
	//WAIT DATA
	//nd_connector_update(connect_handle,_msg_entry,1000) ;

	//send file data
	ND_USERMSG_MAXID(&msg_buf) = MAXID_SYS ;
	ND_USERMSG_MINID(&msg_buf) = SYM_ECHO ;
	ND_USERMSG_PARAM(&msg_buf) = 1 ;
	paddr = ND_USERMSG_DATA(&msg_buf) ;

	while((nReadNum=fread(paddr,1,nReadNum,pf))) {
		
		ND_USERMSG_LEN(&msg_buf) =  ND_USERMSG_HDRLEN + nReadNum ;
		ND_USERMSG_MAXID(&msg_buf) = MAXID_SYS ;
		ND_USERMSG_MINID(&msg_buf) = SYM_ECHO ;

		ret = send_wait(connect_handle, &msg_buf) ;
		if(ret<0)
			return -1 ;
		nReadNum = rand() %(_READ_SIZE-10) +1 ;
		ndprintf("%d,send one message success  sendlen=%d\n", ++_send_times,ret);
	}
	ND_USERMSG_PARAM(&msg_buf) = 2 ;
	ND_USERMSG_LEN(&msg_buf) =  ND_USERMSG_HDRLEN  ;

	fclose(pf) ;
	nd_connectmsg_send(connect_handle,&msg_buf) ;

	return 0;
}

int main(int argc , char *argv[])
{	
	if(argc != 5) {
		ndprintf("usage: %s -f config.xml send_file recv_file\n",argv[0]) ;
		getch();
		exit(1) ;
	}

	_INPUT_FILE = argv[3] ;
	_OUTPUT_FILE = argv[4];
	srand( (unsigned)time( NULL ) );

	if(nd_cliapp_init(argc, argv)){
		ndprintf(_NDT("connect error :%s!"), nd_last_error()) ;
		getch();
		exit(1);
	}

	_h_connect = create_connector() ;
	if(!_h_connect) {
		ndprintf(_NDT("connect error :%s!"), nd_last_error()) ;
		getch();
		exit(1);
	}
		
	nd_msgentry_install(_h_connect,msg_entry,MAXID_SYS,SYM_ECHO,0) ;
	/*if(-1==start_encrypt(_h_connect) ) {
			
		fprintf(stderr, "press any key to continue!\n") ;
		getch();
		exit(1) ;
	}
	else {
		ndprintf(_NDT("start encrypt success!\n")) ;
	}*/

	send_file(_h_connect) ;
	//send_func(get_connect_handle());
	//test_send(argv[1],atoi(argv[2])) ;
	
	
	while(0==__exit) {
		nd_connector_update(_h_connect,1000) ;
		//nd_sleep(100) ;
	}
	destroy_connect(_h_connect) ;
	nd_cliapp_end(0);
	
	fprintf(stderr, "client test exit!\n") ;
	getch();
	return 0;
}
