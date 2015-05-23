/* file : test_client.c
 * client connect test 
 * 2007-10
 */

#pragma comment(lib,"nd_common.lib")
#pragma comment(lib,"nd_net.lib")


#pragma comment(lib,"nd_crypt.lib")
#include "nd_common/nd_common.h"
#include "nd_srvcore/nd_srvlib.h"
#include "nd_srvcore/nd_listensrv.h"
#include "nd_net/nd_netlib.h"
#include "nd_net/nd_netui.h"

#include "nd_crypt/nd_crypt.h"

#include <time.h>

char *__send_file, *__recv_file ;
#define _INPUT_FILE __send_file
#define _OUTPUT_FILE __recv_file
#define _READ_SIZE  512

volatile int __exit = 0;

int msg_handle(nd_netui_handle connect_handle, NETMSG_T mid,NDUINT32 param, NDUINT8 *data, int data_len)
{
	static int index = 0 ;
	if(data_len>0) {
		ndprintf(_NDT("\treceive message id=%d param=%d data=%s len=%d\n"),
			mid,param, data, data_len );
	}
	else {
		ndprintf(_NDT("\treceive message id=%d param=%d  len=%d\n"),
			mid,param,  data_len );
	}
	if(++index>1000) {
		ndprintf(_NDT("MESSAGE FUNCTON RETURN -1 connect exit\n")) ;
		__exit = 1;
		return -1 ;
	}
	return 0;
}

int msg_entry1(nd_netui_handle connect_handle, NETMSG_T mid,NDUINT32 param, NDUINT8 *data, int data_len)
{
	//FILE **wf1 = (FILE**)param;
	static FILE *wf ;
	//nd_assert(wf) ;

	ndprintf(_NDT("\tcurrent recv=%d\n"),data_len + NETUI_PARAM_SIZE);
	//ndprintf(_NDT("received message %d "),NDNET_MSGID(msg));
	if(data_len>0 && 1==mid) {
		nd_assert(wf);
		fwrite(data,data_len,1,wf) ;
		//NDNET_DATA(msg) + NDNET_DATALEN(msg) = 0 ; 
		//msg->buf[NDNET_DATALEN(msg)] = 0 ;

		//ndprintf(_NDT("%s\n"),NDNET_DATA(msg));
	}
	else if(2==mid){
		fflush(wf);
		fclose(wf);
		__exit = 1;
	}
	else if(3==mid) {
		//FILE **open_fp = param ;
		wf=fopen(data, "w+b") ;
		nd_assert(wf);
	}
	return 0 ;
}

int send_func(nd_netui_handle connect_handle)
{
	int _send_times= 0;
	int ret = 0;
	nd_msgui_buf msg_buf;
	//struct ndnet_msg msg_buf;
	char *paddr ;

	//_init_ndmsg_buf(&msg_buf);

	MSGUI_MSGID(&msg_buf) = 1 ;

	while ( __exit==0 && _send_times<10){
		paddr = MSGUI_DATA(&msg_buf);
		ndsprintf(paddr, _NDT("hello world %d"), _send_times++) ;
		MSGUI_DATALEN(&msg_buf) = ndstrlen(paddr)+1 ;
		MSGUI_PARAM(&msg_buf) = 0 ;
		//nd_connector_send(connect_handle,&msg_buf,ESF_POST) ;	
		nd_connector_send(connect_handle,&msg_buf,ESF_NORMAL) ;
		memset(&msg_buf,0,sizeof(msg_buf)) ;
		ret = nd_connector_waitmsg(connect_handle, &msg_buf,1000);
		if(ret > 0) {
			msg_handle(connect_handle, msg_buf.msgid,msg_buf.param,
					msg_buf._data, msg_buf.data_len);
		}
		

		else if(-1==ret) {
			ndprintf(_NDT("closed by remote ret = 0\n")) ;
			break ;
		}
		else {
			ndprintf(_NDT("wait time out ret = %d\n"), ret) ;
			
		}
		
	}
	__exit = 1 ;
	
	return 0;
}
int send_file(nd_netui_handle connect_handle)
{
	static int _send_times= 0;
	int ret = 0;
	nd_msgui_buf msg_buf;
	//struct ndnet_msg msg_buf;

	int nReadNum = rand() %(_READ_SIZE-10) +1 ;
	char *paddr ;

	FILE *pf = fopen(_INPUT_FILE, "r+b") ;
	
	if(!pf) {
		printf("open file error!\n") ;
		return -1;
	}

	//_init_ndmsg_buf(&msg_buf);

	MSGUI_MSGID(&msg_buf) = 3 ;
	paddr = MSGUI_DATA(&msg_buf);
	ndstrcpy(paddr,_OUTPUT_FILE) ;
	MSGUI_DATALEN(&msg_buf) = ndstrlen(_OUTPUT_FILE)+1 ;
	//msg_buf.msg_hdr.param = 0 ;
	MSGUI_PARAM(&msg_buf) = 0 ;
	nd_connector_send(connect_handle,&msg_buf,ESF_URGENCY) ;
	
	nd_connector_update(connect_handle,msg_entry1,1000) ;
	MSGUI_MSGID(&msg_buf) = 1 ;
	
	paddr = MSGUI_DATA(&msg_buf);
	while(nReadNum=fread(paddr,1,nReadNum,pf)) {
		
		MSGUI_DATALEN(&msg_buf) = nReadNum ;

		//getch();
		nd_sleep(1);
		++MSGUI_PARAM(&msg_buf) ;
		ret = nd_connector_send(connect_handle,&msg_buf,ESF_NORMAL) ;
		nd_connector_update(connect_handle,msg_entry1,1000) ;
//#error in here
		nd_assert(ret > 0);
		nReadNum = rand() %(_READ_SIZE-10) +1 ;
		ndprintf("%d,send one message success  sendlen=%d\n", ++_send_times,ret);
	}
	
	MSGUI_MSGID(&msg_buf) = 2 ;
	MSGUI_DATALEN(&msg_buf) = 0 ;

	fclose(pf) ;
	++MSGUI_PARAM(&msg_buf) ;
	nd_connector_send(connect_handle,&msg_buf,ESF_URGENCY) ;

	return 0;
}

static  void* recv_msgt(void *arg)
{
	//struct nd_tcp_node *conn_node = arg ;
	nd_netui_handle connect_handle = arg ;
	//FILE *wf = fopen(_OUTPUT_FILE, "w+b") ;
	//FILE *wf ;
	//conn_node->msg_entry = msg_entry1;
	while (0==__exit){
		//do_client_msg(conn_node,&wf);
		nd_connector_update(connect_handle,msg_entry1,1000) ;
	}
//	fflush(wf);
//	fclose(wf);
	return (void *) 0;
} 

int __pki_ok ;
static char __rsa_digest[16] ;
static RSA_KEY	 __pub_key ;
static tea_k	__crypt_key;

RSA_KEY *get_pubkey()
{
	return &__pub_key;
}

//TEST PKI
int pki_entry(nd_cli_handle nethandle,NETMSG_T msgid, NDUINT32 param,NDUINT8 *data, int data_len) 
{	
	int ret = 0 ;
	switch(msgid){
	case 1:	
		if(data_len==sizeof(__pub_key)) {
			memcpy(&__pub_key, data, data_len) ;
		}
		else {
			ret = 1 ;
		}
		break ;
	case 2:
	{
		if(param) {
			nd_connector_set_crypt(nethandle, NULL, 0 ); 
		}
		else {
			nd_connector_set_crypt(nethandle, &__crypt_key, sizeof(__crypt_key)) ;
		}
		break ;
	}
	
	case 3:		//get rsa digest
	{
		if(data_len==16) {
			MD5Crypt16((char*)&__pub_key, sizeof(__pub_key) , __rsa_digest);
			if(0==MD5cmp(__rsa_digest,data)) {
				__pki_ok = 1 ;
			}
		}
		
		break ;
	}
	default :
		data[data_len] = 0 ;
		ndprintf("recv msgid=%d, data=%s\n", msgid, data) ;
		
	}
	return ret ;
}

int pki_exch(nd_cli_handle connect_handle)
{
	int _send_times= 0;
	int ret = 0;
	
	nd_msgui_buf msg_buf;
	
	//get rsa public key
	create_netui_msg(&msg_buf,1,0, NULL, 0) ;
	nd_connector_send(connect_handle,&msg_buf,ESF_NORMAL) ;
	ret = nd_connector_waitmsg(connect_handle, &msg_buf,1000*10);
	if(ret <= 0) {
		ndprintf("error on getting rsakey\n") ;
		return -1 ;
	}
	pki_entry(connect_handle, msg_buf.msgid,msg_buf.param,
				msg_buf._data, msg_buf.data_len);

	//get rsa md5 check (digest)
	create_netui_msg(&msg_buf,3,0, NULL, 0) ;
	nd_connector_send(connect_handle,&msg_buf,ESF_NORMAL) ;
	ret = nd_connector_waitmsg(connect_handle, &msg_buf,1000*10);
	if(ret <= 0) {
		ndprintf("error on getting rsakey\n") ;
		return -1 ;
	}
	pki_entry(connect_handle, msg_buf.msgid,msg_buf.param,
				msg_buf._data, msg_buf.data_len);

	if(!__pki_ok){
		ndprintf("error on getting rsa digest\n") ;
		return -1 ;
	}
	
	//send symm encrypt key
	if(0!=tea_key(&__crypt_key) ){
		ndprintf("error on create crypt-key\n") ;
		return -1 ;
	}
	
	msg_buf.data_len = sizeof(__crypt_key) ;
	if(0!=RSAencrypt(&__pub_key, (char *)&__crypt_key, msg_buf._data, &msg_buf.data_len)) {
		ndprintf("rsa encrypt error data error\n") ;
		return -1 ;
	}

	msg_buf.msgid = 2 ;
	nd_connector_send(connect_handle,&msg_buf,ESF_NORMAL) ;
	ret = nd_connector_waitmsg(connect_handle, &msg_buf,1000*10);
	if(ret <= 0) {
		ndprintf("sending crypt key error\n") ;
		return -1 ;
	}
	pki_entry(connect_handle, msg_buf.msgid,msg_buf.param,
				msg_buf._data, msg_buf.data_len);

	if(!nd_connector_check_crypt(connect_handle)){
		ndprintf("exchange PKI error\n") ;
		return -1 ;
	}
	ndprintf("exchange PKI SUCCESS\n") ;
	

	return 0;
}
int main(int argc , char *argv[])
{	
//	ndthread_t recv_id;
//	ndth_handle hRecv;
	nd_netui_handle connect_handle ;
	if(argc != 5) {
		ndprintf("usage: %s host port send_file recv_file\n",argv[0]) ;
		getch();
		exit(1) ;
	}
	_INPUT_FILE = argv[3] ;
	_OUTPUT_FILE = argv[4];
	srand( (unsigned)time( NULL ) );

	nd_arg(argc, argv);
	nd_common_init() ;
	
	nd_net_init() ;

	connect_handle = nd_connector_open(argv[1], atoi(argv[2]),ND_UDT_STREAM) ;
	if(!connect_handle){		
		ndprintf(_NDT("connect error :%s!"), nd_last_error()) ;
		getch();
		exit(1);
	}
	/*nd_tcpnode_init(&conn_node) ;
	if(-1==nd_tcpnode_connect(argv[1], atoi(argv[2]),&conn_node )){
		ndprintf(_NDT("connect error :%s!"), nd_last_error()) ;
		getch();
		exit(1);
	}*/
	
	//nd_socket_nonblock(conn_node.fd,0);

	//hRecv = nd_createthread(recv_msgt, connect_handle, &recv_id, 0) ;

	
	if(-1==pki_exch(connect_handle) ) {
			
		fprintf(stderr, "press any key to continue!\n") ;
		getch();
		exit(1) ;
	}
	//send_file("localhost",7828);
	//send_file(connect_handle) ;
	send_func(connect_handle);
	//test_send(argv[1],atoi(argv[2])) ;
	
	
//	nd_waitthread(hRecv) ;
//	nd_close_handle(hRecv) ;

	while(0==__exit) {
		nd_connector_update(connect_handle,msg_entry1,1000) ;
	}
	nd_connector_close(connect_handle,0);

	nd_net_destroy() ;
	nd_common_release() ;
	
	fprintf(stderr, "client test exit!\n") ;
	getch();
	return 0;
}