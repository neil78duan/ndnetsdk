/* file srvcore.c
 * server core module 
 *
 * all right reserved by neil duan
 * 2009-1-5 
 * version 1.0
 */

#include "nd_srvcore/nd_srvlib.h"

#if !defined(ND_UNIX) 

extern int nd_iocp_node_init(struct nd_session_iocp *iocp_map,nd_handle h_listen) ;
extern int iocp_close_client(struct nd_session_iocp *iocp_map, int force) ; 
#endif 

extern void nd_listen_contex_init(nd_listen_handle handle) ;

int register_listensrv(void) ;

int nd_srvcore_init(void)
{
	return register_listensrv() ;
}

void nd_srvcore_destroy(void)
{

}

static void srv_tcp_init(struct  listen_contex *lc)
{
	nd_listen_contex_init((nd_listen_handle)lc) ;
	
	lc->io_mod = ND_LISTEN_COMMON ;
	lc->tcp.sock_type = SOCK_STREAM;
	INIT_LIST_HEAD(&lc->list_thread) ;

}

static void srv_ext_init(struct  listen_contex *lc)
{

	nd_listen_contex_init((nd_listen_handle)lc) ;

	lc->io_mod = ND_LISTEN_OS_EXT ;
	lc->tcp.sock_type = SOCK_STREAM;
	INIT_LIST_HEAD(&lc->list_thread) ;

#if !defined(ND_UNIX) 
	nd_srv_set_cm_init(&lc->tcp,(cm_init )nd_iocp_node_init) ;
#else 
#endif
	
}

static void srv_udt_init(struct  listen_contex *lc)
{

	nd_listen_contex_init((nd_listen_handle)lc) ;

	lc->io_mod = ND_LISTEN_UDT_STREAM ;

	lc->tcp.protocol = PROTOCOL_UDT ;

	lc->tcp.sock_type = SOCK_DGRAM ;
	INIT_LIST_HEAD(&lc->list_thread) ;
	nd_srv_set_cm_init(&lc->tcp,(cm_init )nd_session_udt_init) ;
}

int register_listensrv(void)
{
	int ret ;
	struct nd_handle_reginfo reginfo ;
	reginfo.object_size = sizeof(struct  listen_contex ) ;
	reginfo.close_entry = (nd_close_callback )nd_listener_close ;
	
	//tcp connector register 
	reginfo.init_entry = (nd_init_func )srv_tcp_init ;
	ndstrcpy(reginfo.name, "listen-tcp" ) ;
	
	ret = nd_object_register(&reginfo) ;
	if(-1==ret) {
		nd_logerror("register tcp-listen server error") ;
		return -1 ;
	}

	
	//udt connector register 
	reginfo.init_entry =(nd_init_func ) srv_udt_init ;
	ndstrcpy(reginfo.name, "listen-udt" ) ;
	
	ret = nd_object_register(&reginfo) ;
	if(-1==ret) {
		nd_logerror("register udtlisten server error") ;
		return -1 ;
	}


	//ext connector register 
	reginfo.init_entry =(nd_init_func ) srv_ext_init ;
	ndstrcpy(reginfo.name, "listen-ext" ) ;
	
	ret = nd_object_register(&reginfo) ;
	if(-1==ret) {
		nd_logerror("register ext-listen server error") ;
		return -1 ;
	}
	return ret ;
}
