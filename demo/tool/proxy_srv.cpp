/* file : srv_test.cpp
* test by proxy server
* demo raw data hook program by socks5 proxy server
*/

#include "nd_appcpp/nd_instance.h"
#include "nd_appcpp/nd_listener.h"
#include "nd_appcpp/nd_connector.h"
#include "nd_appcpp/nd_cmmgr.h"

static int update_remote_connect(void *param) ;

static int listen_data_handle(nd_handle session, void *data, size_t size, nd_handle listener) ;

static int read_remote_proxy(char *file, struct nd_proxy_info *proxy_info );

#if defined(ND_DEBUG)
#pragma comment(lib,"nd_appcpp_dbg.lib")
#pragma comment(lib,"nd_app_dbg.lib")
#else
#pragma comment(lib,"nd_appcpp.lib")
#pragma comment(lib,"nd_app.lib")
#endif

#ifdef SINGLE_THREAD_MOD
#error NOT SURPORT SINGLE_THREAD_MOD
#endif

class ProxySession : public NDSession
{
public :
	ProxySession() : proxy_state(0),m_is_udp(0){}
	virtual ~ProxySession() {}
	int GetStat() {return  proxy_state ;}
	void SetStat(int s) {proxy_state = s;}
	int ConnectRemote(char *host, short port, int isudp=0) ;
	void UpdateRemote() ;
	int SendRemote(void *data, size_t size) ;
	void Shutdown() ;
	ndsocket_t GetRemotefd() ;
	nd_handle GetConnector() {return m_remote_handle;}

	SOCKADDR_IN *GetRemoteAddr() {return &m_remote_addr;}
protected :
	//NDConnector m_remote_connector;
	nd_handle m_remote_handle; 

	SOCKADDR_IN m_client_addr, m_remote_addr;

	int m_is_udp;
	int proxy_state ;
};

class ProxyListener : public NDListener 
{
public:
	ProxyListener() {} ;
	virtual ~ProxyListener() {} ;

	NDSession *ConstructSession(void *addr) { return new(addr) ProxySession ;}
	void DestructSession(ProxySession *psession) { delete psession ;}

	void OnClose(NDSession *pSession)
	{
		((ProxySession*)pSession)->Shutdown() ;
	}
	//int OnAccept(NDSession *pSession, SOCKADDR_IN*addr);	//连接进入回调函数
};

class ProxyInst : public NDInstanceSrv 
{

public :
	ProxyInst() {} 
	virtual ~ProxyInst() {} 

	int InitProxy() ;
	int InitServer( size_t session_size);
protected:

	int CreateUpdate() ;
	NDListener *CreateListener() {return &proxy_Listen;}
	void DestroyListener() {}
	ProxyListener proxy_Listen ;
	nd_thsrvid_t  update_remote_th;
};

struct nd_proxy_info g_remote_proxy ;

int main(int argc, char *argv[])
{
	ProxyInst nd_instance ;

	NDInstanceSrv::InitApp(argc, argv) ;

	nd_instance.InitServer(sizeof(ProxySession));

	nd_instance.InitProxy()  ;

	nd_instance.StartServer() ;

	nd_instance.WaitServer() ;

	nd_instance.EndServer(0) ;

	ProxyInst::DestroyApp() ;

	printf_dbg("program exit from main\n Press ANY KEY to continue\n") ;
	getch();
	return 0;
}


ndsocket_t ProxySession::GetRemotefd() 
{
	if(nd_connector_valid((nd_netui_handle)m_remote_handle)) {
		return nd_connector_fd(m_remote_handle) ;
	}
	return -1;
	
}
int ProxySession::ConnectRemote(char *host, short port, int isudp) 
{
	int sock_len = sizeof(SOCKADDR_IN);
	m_is_udp = isudp;
	if (isudp) {
		m_remote_handle = nd_object_create("udp-node") ;
		if(!m_remote_handle)
			return -1 ;
		//bind 
		if (-1==nd_net_bind(0,0, m_remote_handle)) {
			nd_object_destroy(m_remote_handle,1) ;
			m_remote_handle = 0 ;
			return -1;
		}
		::getsockname(nd_connector_fd(GetSessionHandle()),(struct sockaddr *)&m_client_addr,(socklen_t*)&sock_len) ;
		m_client_addr.sin_port = htons(port) ;
		m_client_addr.sin_family = AF_INET ;

		sock_len = sizeof(SOCKADDR_IN);
		::getsockname(nd_connector_fd(m_remote_handle),(struct sockaddr *)&m_remote_addr,(socklen_t*)&sock_len) ;
		m_remote_addr.sin_addr.s_addr = m_client_addr.sin_addr.s_addr;
		m_remote_addr.sin_family = AF_INET ;
		//m_remote_addr.sin_port = htons()

	}
	else {
		m_remote_handle = nd_object_create("tcp-connector") ;
		if(!m_remote_handle)
			return -1 ;
		//bind 
		if (-1==nd_connector_open(m_remote_handle, host, port,NULL) ) {
			nd_object_destroy(m_remote_handle,1) ;
			m_remote_handle = 0 ;
			return -1;
		}
		::getsockname(nd_connector_fd(GetSessionHandle()),(struct sockaddr *)&m_client_addr,(socklen_t*)&sock_len) ;
		m_client_addr.sin_family = AF_INET ;

		sock_len = sizeof(SOCKADDR_IN);
		::getsockname(nd_connector_fd(m_remote_handle),(struct sockaddr *)&m_remote_addr,(socklen_t*)&sock_len) ;
		m_remote_addr.sin_family = AF_INET ;
	}

	return 0 ;
	
}

void ProxySession::UpdateRemote() 
{
	int ret ;
	char buf[1024*16] ;
	if(!nd_connector_valid((nd_netui_handle)m_remote_handle)) {
		return ;
	}


	ret = nd_connector_raw_waitdata(m_remote_handle, buf, sizeof(buf), 0) ;

	//ret = m_remote_connector.RecvRawData(buf, sizeof(buf),0) ;
	if(0==ret || (ret == -1 && NDERR_WUOLD_BLOCK !=nd_object_lasterror(m_remote_handle))  ) {
		Shutdown();
	}
	if(ret <= 0) {
		return ;
	}
	if (!m_is_udp) {
		SendRawData(buf, ret) ;
		return ;

	}

	//读取地址
	struct sockaddr_in* read_addr = nd_udp_read_addr(m_remote_handle) ;
	if (read_addr->sin_addr.s_addr==m_client_addr.sin_addr.s_addr && read_addr->sin_port==m_client_addr.sin_port ) {
		//来自client
		int sock_len;
		struct sockaddr_in aim_addr ;
		if(buf[0]!=0 || buf[1]!=0 || buf[2]!=0) {
			return ;
		}
		if (buf[3]==1)	{
			if(ret <= 10) {
				return ;
			}
			aim_addr.sin_addr.s_addr =*(int*) &buf[4] ;
			aim_addr.sin_port = *(short*) &buf[8] ;
			sock_len = 10;
			ret -= 10 ;
		}
		else if(buf[3]==3){
			int sock_len = buf[4] + 5;
			if ( sock_len >= ret) {
				return ;
			}

			aim_addr.sin_port = *(short *) &buf[sock_len] ;
			sock_len += 2 ;

			if ( sock_len >= ret) {
				return ;
			}
			ret -= sock_len ;

		}

		aim_addr.sin_family = AF_INET ;
		memcpy(buf, buf+sock_len, ret) ;
		nd_udp_sendto(m_remote_handle,buf,ret, &aim_addr) ; 
	}
	else {
		//来自远程
		nd_proxy_sendto(nd_connector_fd(m_remote_handle),buf,ret, read_addr, &m_client_addr) ;
	}

}

int ProxySession::SendRemote(void *data, size_t size) 
{
	return nd_session_raw_write(m_remote_handle, data, size) ;
	//return m_remote_connector.SendRawData(data,size);
}

void ProxySession::Shutdown() 
{
	Close(0) ;
	nd_object_destroy(m_remote_handle,0) ;
	m_remote_handle = NULL;
	//m_remote_connector.Close(0) ;
	//m_remote_connector.Destroy() ;
}

int ProxyInst::InitServer( size_t session_size)
{
	pListen = CreateListener() ;

	if(!pListen) {
		return -1 ;
	}
	pListen->Initilize(NDInstanceSrv::srvcfg.max_connect, session_size, "listen-tcp") ;

	return 0;

}
int ProxyInst::InitProxy() 
{
	if (-1==CreateUpdate())
		return -1;

	NDListener *pLis = GetDeftListener() ;
	nd_assert(pLis) ;
	nd_handle h_listen = pLis->GetHandle() ;


	nd_hook_data(h_listen, listen_data_handle);

	read_remote_proxy(NDInstanceSrv::config_file, &g_remote_proxy ) ;
	return 0;

}
int ProxyInst::CreateUpdate()
{	
	struct nd_thsrv_createinfo th_info = {
		SUBSRV_RUNMOD_LOOP,	//srv_entry run module (ref e_subsrv_runmod)
		(nd_threadsrv_entry)update_remote_connect,			//service main entry function address
		(void*)this,		//param of srv_entry 
		NULL ,					
		_NDT("Instance_timer"),			//service name
	};

	update_remote_th = nd_thsrv_createex(&th_info, NDT_PRIORITY_LOW,0)  ;
	if(!update_remote_th )
		return -1;
	return 0 ;
}

/* entry of  timer update client map*/
int update_remote_connect(void *param)
{
	ProxySession *psession ;
	NDInstanceSrv *pinst = static_cast<NDInstanceSrv *> (param) ;

	NDCmIterator cm_it(pinst) ;
	NDOStreamMsg sendmsg(MAXID_SYS, SYM_TEST) ;

	if(!cm_it.CheckValid()) {
		nd_sleep(100) ;
		return 0 ;
	}
	
	for (psession=(ProxySession *) cm_it.First(); psession; psession=(ProxySession *)cm_it.Next()) {
		psession->UpdateRemote() ;		
	}
	nd_sleep(10) ;
	return 0 ;
}

static int _listen_data_handle(nd_handle session, void *data, size_t size, nd_handle listener) ;

int listen_data_handle(nd_handle session, void *data, size_t size, nd_handle listener)
{
	int ret = _listen_data_handle( session, data,  size,  listener) ;
	if(ret==-1) {
		return ret ;
	}
	return (int) size ;
}
//直接处理来自socket的原始数据
int _listen_data_handle(nd_handle session, void *data, size_t size, nd_handle listener)
{

	ProxySession *psession =(ProxySession*) NDGetSession( session) ;
	nd_assert(psession) ;

	switch(psession->GetStat()) {
		case 2:
			psession->SendRemote(data, size) ;
			break ;
		case 0 :
			{
				char *cmd =(char*) data ;
				char buf[128] ;
				if (size < 3){
					goto PROXY_ERROR ;
				}
				if(cmd[0] != 5 )
					goto PROXY_ERROR ;

				buf[0] = 5; buf[1] = 0 ;
				psession->SendRawData(buf, 2) ;
				psession->SetStat(1) ;
				return 0 ;

			}
			break ;

		case 1 :
			{
				short port;
				int is_udp = 0 ;
				int port_offset=8 ;
				char *cmd =(char*) data ;
				char host[256];
				
				if (cmd[0]!=5) {
					goto PROXY_ERROR ;
				}

				//get host and port
				if(cmd[3]==1){
					memset(host,0,sizeof(host));
					snprintf(host,128,"%d.%d.%d.%d",0xff&cmd[4],0xff&cmd[5],0xff&cmd[6],0xff&cmd[7]);
				}
				else{
					int host_len= cmd[4] ;
					if(host_len<=0)
						goto PROXY_ERROR ;
					memcpy(host,cmd+5,host_len);
					host[host_len]=0;

					port_offset=host_len+5;
				}
				port=(0xff&cmd[port_offset+1]) + ((0xff&cmd[port_offset]) << 8);

				if(cmd[1]==1) {
					//connect
				}
				else if(cmd[1]==2) {
					//bind not surport
					goto PROXY_ERROR ;
				}
				else if(cmd[1]==3) {
					//udp not surport
					is_udp = 1;
				}

				//get 

				if(0==psession->ConnectRemote(host, port,is_udp) ) {

					SOCKADDR_IN *addr = psession->GetRemoteAddr() ;

					host[0] = 5 ; host[1]=0 ; host[2] = 0;
					host[3] = 1 ; 

					*(unsigned int*)(&host[4]) = addr->sin_addr.s_addr ;//nd_sock_getip(fd) ;
					*(unsigned short*)(&host[8]) = addr->sin_port;
					
					psession->SendRawData(host, 10) ;

				}
				else
					goto PROXY_ERROR ;
				psession->SetStat(2);
				return 0 ;

			}
			break ;
	}

	return 0;
PROXY_ERROR:
	{
		char res[2] ;
		res[0]= 5; res[1] = 1 ;
		psession->SendRawData(res, 2) ;
		return -1;
	}
}


int read_remote_proxy(char *file, struct nd_proxy_info *proxy_info )
{
	int ret;
	ndxml  *xml_node, *xml_read ;
	ndxml_root xmlroot;

	nd_assert(file) ;
	ret = ndxml_load(file, &xmlroot) ;
	if(0!=ret) {
		return -1;
	}
	xml_node = ndxml_getnode(&xmlroot,"server_config") ;
	if(!xml_node) {		
		ndxml_destroy(&xmlroot);
		return -1;
	}

	//read proxy
	xml_read = ndxml_refsub(xml_node, "proxy") ;
	if(xml_read) {

		ndxml *xmltmp = ndxml_refsub(xml_read, "proxy_type") ;
		if (!xmltmp) 	goto CFG_EXIT ;		
		proxy_info->proxy_type = ndxml_getval_int(xmltmp);
		if(0==proxy_info->proxy_type )goto CFG_EXIT ;

		xmltmp = ndxml_refsub(xml_read, "proxy_host") ;
		if (!xmltmp) 	goto CFG_EXIT ;		
		ndxml_getval_buf(xmltmp, proxy_info->proxy_host, sizeof(proxy_info->proxy_host)) ;


		xmltmp = ndxml_refsub(xml_read, "proxy_port") ;
		if (!xmltmp) 	goto CFG_EXIT ;		
		proxy_info->proxy_port = ndxml_getval_int(xmltmp) ;

		xmltmp = ndxml_refsub(xml_read, "proxy_user") ;
		if (xmltmp)
			ndxml_getval_buf(xmltmp, proxy_info->user, sizeof(proxy_info->user)) ;

		xmltmp = ndxml_refsub(xml_read, "proxy_password") ;
		if (xmltmp) 
			ndxml_getval_buf(xmltmp, proxy_info->password, sizeof(proxy_info->password)) ;
	}

CFG_EXIT:
	ndxml_destroy(&xmlroot);
	return 0;

}