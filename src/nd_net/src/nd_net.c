/* file nd_net.c
 * net mode initialize destroy  entry
 * neil
 * 2007-10-6
 */


#include "nd_net/nd_netlib.h"
//#include "nd_net/nd_sock.h"

#if !defined(ND_UNIX) 
#pragma comment(lib, "Ws2_32.lib") 

// #if defined(ND_COMPILE_AS_DLL)
// #ifdef ND_DEBUG
// #pragma comment(lib, "nd_common_dbg.lib") 
// #else 
// #pragma comment(lib, "nd_common.lib") 
// #endif
// #endif
#endif

int register_connector(void) ;
int nd_net_init(void)
{
#if !defined(ND_UNIX) 
	{
		WORD wVersionRequested = MAKEWORD(2,2);
		
		WSADATA wsaData;

 		HRESULT nRet = WSAStartup(wVersionRequested, &wsaData);

		if (0!=nRet || wsaData.wVersion != wVersionRequested)
		{	
			WSACleanup();
			return -1;
		}
	}
#else 
#endif
	
	register_connector() ;
	//nd_logmsg("nd_net_init success \n") ;
	return 0 ;
}

void nd_net_destroy(void)
{
	_release_send_stream();
#if !defined(ND_UNIX) 
	WSACleanup();
#else 
#endif
}

//
int register_connector(void)
{
	int ret ;
	struct nd_handle_reginfo reginfo ;
	
	//tcp connector register 
	reginfo.object_size = sizeof(struct nd_tcp_node ) ;
	reginfo.init_entry =(nd_init_func ) nd_tcpnode_init ;
	reginfo.close_entry = (nd_close_callback )_connector_destroy  ;
	ndstrcpy(reginfo.name, "tcp-connector" ) ;
	
	ret = nd_object_register(&reginfo) ;
	if(-1==ret) {
		nd_logerror("register tcp-connector error") ;
		return -1 ;
	}

	//udp register
	reginfo.object_size = sizeof(nd_udp_node) ;
	reginfo.init_entry = (nd_init_func ) udp_node_init ;
	reginfo.close_entry = (nd_close_callback )nd_udp_close  ;
	ndstrcpy(reginfo.name, "udp-node" ) ;

	ret = nd_object_register(&reginfo) ;
	if(-1==ret) {
		nd_logerror("register udt-connector error") ;
		return -1 ;
	}
	//udt connector register 
	reginfo.object_size = sizeof(nd_udt_node) ;
	reginfo.init_entry = (nd_init_func ) nd_udtnode_init ;
	reginfo.close_entry = (nd_close_callback )_connector_destroy  ;
	ndstrcpy(reginfo.name, "udt-connector" ) ;

	ret = nd_object_register(&reginfo) ;
	if(-1==ret) {
		nd_logerror("register udt-connector error") ;
		return -1 ;
	}


//	//ICMP-udt connector register
//	reginfo.object_size = sizeof(nd_udt_node) ;
//	reginfo.init_entry = (nd_init_func ) udt_icmp_init ;
//	reginfo.close_entry = (nd_close_callback )_connector_destroy  ;
//	ndstrcpy(reginfo.name, "icmp-connector" ) ;
//
//	ret = nd_object_register(&reginfo) ;
//	if(-1==ret) {
//		nd_logerror("register udt-connector error") ;
//		return -1 ;
//	}
	return ret ;
}
