/* file : checksum.c
 * test checksum arithmetic
 * 2008-5-16
 * all right reserved
 */

#include "nd_common/nd_common.h"

#include "nd_net/nd_netlib.h"
#include "nd_net/nd_netui.h"


void calc_cs(udt_pocketbuf *pocket , int len);
#define  _READ_SIZE 512

int checksum_test()
{
	int ret = 0, send_times =0;
	udt_pocketbuf packet;
	u_32 check_sum,cs_tmp ;
	
	int nReadNum = rand() %(_READ_SIZE-10) +1 ;
	char *paddr ;

	FILE *pf = fopen("3.pdf", "r+b") ;
	
	if(!pf) {
		printf("open file error!\n") ;
		return -1;
	}

	init_udt_pocket(&(packet.pocket)) ;
	
	paddr =pocket_data(&(packet.pocket));
	POCKET_TYPE(&(packet.pocket)) = 3 ;
	while(nReadNum=fread(paddr,1,nReadNum,pf)) {
		
		calc_cs(&packet ,nReadNum) ;
		nd_sleep(1);

		check_sum =	POCKET_CHECKSUM(&(packet.pocket))  ;
		POCKET_CHECKSUM(&(packet.pocket)) = 0 ;

		
		nReadNum += ndt_header_size(&(packet.pocket)) ;
		cs_tmp = nd_checksum((NDUINT16*)&(packet.pocket),nReadNum) ;
		if(cs_tmp!=check_sum) {
			ndprintf("test checksum error check_sum=%d cs_tmp=%d\n", check_sum,cs_tmp);
			getch() ;
			exit(1);
		}
		else{
			ndprintf("test check OK\n", ++send_times,ret);
		}

		nReadNum = rand() %(_READ_SIZE-10) +1 ;
		
	}
	
	
	fclose(pf) ;
	
	//nd_connector_send(connect_handle,&msg_buf,ESF_URGENCY) ;

	return 0;	
}

void calc_cs(udt_pocketbuf *pocket , int len)
{
//	u_32 sums ;
//	
//	POCKET_SESSIONID(&pocket->pocket) = 1024 ;
//	pocket->pocket.window_len = 4096;
//
//	len += ndt_header_size(&pocket->pocket) ;
//	POCKET_CHECKSUM(&pocket->pocket) = 0 ;
//
//	sums = nd_checksum((NDUINT16*)pocket,len) ;
//	POCKET_CHECKSUM(&pocket->pocket) = LOWORD(sums) ;

	//udt_host2net(pocket) ;

	//ret = nd_socket_udp_write(socket_node->listen_fd, (char*)pocket, len, &socket_node->dest_addr) ;
	
	//udt_net2host(pocket) ;
	
}
