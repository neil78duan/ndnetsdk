/* file nd_connector.cpp
 *
 * implemention of connector 
 *
 * create by duan 
 * 2011/4/2 15:59:39
 */

#define BUILD_AS_STATIC_LIB
// 
// #if _MSC_VER==1600
// #ifdef ND_DEBUG
// #pragma comment(lib,"vs10_static_cli_dbg.lib")
// #else 
// #pragma comment(lib,"vs10_static_cli.lib")
// #endif
// #else 
// 
// #endif


#include "nd_common/nd_common.h"
#include "nd_net/nd_netlib.h"
#include "nd_crypt/nd_crypt.h"
#include "ndapplib/nd_msgpacket.h"
//#include "pg_loginmsg.h"

#include "ndcli/nd_iconn.h"
#include "nd_msg.h"
#include "ndcli/nd_api_c.h"
//extern int nd_exchange_key(nd_handle nethandle) ;

CPPAPI void tryto_terminate(netObject netObj) ;

class NDConnector : public NDIConn 
{
public :		
	void Destroy(int flag = 0);
	int Create(const char *protocol_name=NULL) ;
	int Open(const char*host, int port,const char *protocol_name, nd_proxy_info *proxy=NULL);
	int Close(int force=0);

	int Send(int maxid, int minid, void *data, size_t size) ;
	int SendMsg(NDSendMsg &msg, int flag=ESF_NORMAL) ;
	int SendMsg(nd_usermsgbuf_t *msghdr, int flag=ESF_NORMAL);
	int SendRawData(void *data , size_t size) ;
	int RecvRawData(void *buf, size_t size, ndtime_t waittm) ;

	int CheckValid();
	int WaitMsg(nd_usermsgbuf_t *msgbuf, ndtime_t wait_time=100);
	int Update(ndtime_t wait_time);
	void InstallMsgFunc(nd_iconn_func, ndmsgid_t maxid, ndmsgid_t minid);
    void SetDftMsgHandler(nd_iconn_func func);
    
	NDConnector(int maxmsg_num =ND_MAIN_MSG_CAPACITY, int maxid_start=ND_MSG_BASE_ID) ;
	void SetMsgNum(int maxmsg_num , int maxid_start=ND_MSG_BASE_ID) ;
	virtual~NDConnector() ;
    
	int Reconnect(ndip_t IP, int port,nd_proxy_info *proxy=NULL) ;//connect to another host

	nd_handle GetHandle() {return m_objhandle;}
	int ExchangeKey(void *output_key) ;

	int LastError() ;
	void SetLastError(NDUINT32 errcode);
	const char *ErrorDesc() ;

    void *GetUserData() { return __userData ;}
    void SetUserData(void *pData){ __userData = pData ;}

	int ioctl(int cmd, void *val, int *size) ;

private:
	//nd_handle m_objhandle ;
	int msg_kinds ;
	int msg_base ;
	nd_handle m_objhandle ;

    void *__userData ;
};



NDConnector::NDConnector(int maxmsg_num , int maxid_start) 
{
	m_objhandle = NULL ;
	msg_kinds = maxmsg_num;
	msg_base = maxid_start;

    __userData = 0 ;
}

NDConnector::~NDConnector() 
{
	Destroy(0) ;
}

int NDConnector::LastError() 
{
	if(!m_objhandle)
		return NDERR_INVALID_HANDLE ;
	else 
		return nd_object_lasterror(m_objhandle) ;

}

int NDConnector::ioctl(int cmd, void *val, int *size)
{
	if(!m_objhandle)
		return -1 ;
	else
		return nd_net_ioctl((nd_netui_handle)m_objhandle, cmd, val, size) ;
}

void NDConnector::SetLastError(NDUINT32 errcode) 
{
	if(m_objhandle)
		nd_object_seterror(m_objhandle,errcode) ;
}

const char *NDConnector::ErrorDesc() 
{
	if(m_objhandle)
		return nd_object_errordesc(m_objhandle) ;
	return NULL;
}

//…Ë÷√œ˚œ¢”≥…‰±Ì¥Û–°,±ÿ–Î‘⁄OPEN∫Ø ˝«∞µ˜”√
void NDConnector::SetMsgNum(int maxmsg_num , int maxid_start) 
{
	msg_kinds = maxmsg_num;
	msg_base = maxid_start;
}

int NDConnector::Open(const char *host, int port,const char *protocol_name,nd_proxy_info *proxy)
{
	if(!m_objhandle) {
		//return -1 ;
		if(-1==Create(protocol_name) ) {
			return -1;
		}
	}

	if(-1==nd_connector_open( m_objhandle, host,  port,(struct nd_proxy_info*)proxy ) ) {
		nd_logerror("connect error :%s!" AND nd_last_error()) ;
		nd_object_destroy(m_objhandle,1) ;
		m_objhandle = NULL ;
		return -1;
	}

	return 0 ;

}

int NDConnector::Reconnect(ndip_t IP, int port,nd_proxy_info *proxy)
{
	return ::nd_reconnect(m_objhandle,  IP,  port, (nd_proxy_info*)proxy) ;
}
int NDConnector::Close(int force)
{
	if(m_objhandle) {
		int ret =  nd_connector_close(m_objhandle, force) ;
		if(ret==0) {}
		return ret;
	}
	return -1 ;
}

static int cliconn_translate_message(nd_netui_handle connect_handle, nd_packhdr_t *msg ,nd_handle listen_handle)  ;
int NDConnector::Create(const char *protocol_name)
{
	//connect to host 
	if (m_objhandle) {
		Destroy() ;
		m_objhandle = NULL;
	}
	m_objhandle = nd_object_create(protocol_name? protocol_name: "tcp-connector"  ) ;

	if(!m_objhandle){		
		nd_logerror("connect error :%s!" AND nd_last_error()) ;
		return -1;
	}
	((nd_netui_handle)m_objhandle)->user_data =(void*) this ;

	//set message handle	
	if (msg_kinds > 0){
		if(-1==nd_msgtable_create(m_objhandle, msg_kinds, msg_base) ) {
			nd_object_destroy(m_objhandle, 0) ;
		}
		nd_hook_packet(m_objhandle,(net_msg_entry )cliconn_translate_message);
	}
	return 0 ;

}
void NDConnector::Destroy(int flag)
{
	if(m_objhandle && ((nd_netui_handle)m_objhandle)->user_data ==(void*) this){
		nd_connector_close(m_objhandle, 0) ;
		nd_msgtable_destroy(m_objhandle, 0);
		nd_object_destroy(m_objhandle, 0) ;
		m_objhandle = 0 ;
	}
}

int NDConnector::SendMsg(NDSendMsg &msg, int flag) 
{
	nd_assert(m_objhandle) ;
	int ret = nd_connector_send(m_objhandle,(nd_packhdr_t*) (msg.GetMsgAddr()), flag) ;
	if (-1==ret && NDERR_WUOLD_BLOCK != nd_object_lasterror(m_objhandle)) {
		tryto_terminate((netObject)m_objhandle) ;
	}
	return ret ;
}

int NDConnector::Send(int maxid, int minid, void *data, size_t size) 
{
	NDOStreamMsg omsg(maxid, minid) ;
	if(-1==omsg.WriteBin(data, size) ) {
		return -1 ;
	}
	int ret = nd_connector_send(m_objhandle,(nd_packhdr_t*) (omsg.GetMsgAddr()),ESF_NORMAL) ;
	if (-1==ret && NDERR_WUOLD_BLOCK != nd_object_lasterror(m_objhandle)) {
		tryto_terminate((netObject)m_objhandle) ;
	}
	return ret ;
}

int NDConnector::SendMsg(nd_usermsgbuf_t *msghdr, int flag)
{
	int ret = nd_connector_send(m_objhandle,(nd_packhdr_t*)msghdr, flag) ;
	if (-1==ret && NDERR_WUOLD_BLOCK != nd_object_lasterror(m_objhandle)) {
		tryto_terminate((netObject)m_objhandle) ;
	}
	return ret ;

}

int NDConnector::SendRawData(void *data , size_t size) 
{
	nd_assert(m_objhandle) ;
	int ret = nd_connector_raw_write(m_objhandle,data,size) ;
	if (-1==ret && NDERR_WUOLD_BLOCK != nd_object_lasterror(m_objhandle)) {
		tryto_terminate((netObject)m_objhandle) ;
	}
	return ret ;

}

int NDConnector::RecvRawData(void *buf, size_t size, ndtime_t waittm) 
{
	nd_assert(m_objhandle) ;
	int ret = nd_connector_raw_waitdata(m_objhandle, buf, size, waittm) ;
		
	if (-1==ret ) {
		if (NDERR_WUOLD_BLOCK != nd_object_lasterror(m_objhandle) &&
			NDERR_TIMEOUT != nd_object_lasterror(m_objhandle) &&
			NDERR_INVALID_INPUT != nd_object_lasterror(m_objhandle) ) {
			tryto_terminate((netObject)m_objhandle) ;
		}
	}

	return ret ;

}

int NDConnector::Update(ndtime_t wait_time)
{

	int ret = 0;
	//nd_msgui_buf msg_recv;
	if(m_objhandle->type==NDHANDLE_UDPNODE) {
		nd_usermsgbuf_t msg_recv;
RE_WAIT:
		ret = nd_connector_waitmsg(m_objhandle, (nd_packetbuf_t *)&msg_recv,wait_time);
		if(ret > 0) {			
			//msg_entry(connect_handle, &msg_recv) ;
			cliconn_translate_message((nd_netui_handle)m_objhandle, (nd_packhdr_t*)&msg_recv, 0) ;
			wait_time = 0;
			goto RE_WAIT;
			//return 0;
		}

	}
	else {
		ret = nd_connector_update(m_objhandle,wait_time) ;
	}
	
	if (-1==ret ) {
		if (NDERR_WUOLD_BLOCK != nd_object_lasterror(m_objhandle) &&
			NDERR_TIMEOUT != nd_object_lasterror(m_objhandle)  ) {
			tryto_terminate((netObject)m_objhandle) ;
		}
	}
	
	return ret ;

}

int NDConnector::WaitMsg(nd_usermsgbuf_t*msgbuf, ndtime_t wait_time)
{
	int ret = nd_connector_waitmsg(m_objhandle, (nd_packetbuf_t *)msgbuf,wait_time);
	
	if (-1==ret ) {
		if (NDERR_WUOLD_BLOCK != nd_object_lasterror(m_objhandle) &&
			NDERR_TIMEOUT != nd_object_lasterror(m_objhandle) &&
			NDERR_INVALID_INPUT != nd_object_lasterror(m_objhandle) ) {
			tryto_terminate((netObject)m_objhandle) ;
		}
	}
	
	return ret ;

}
void NDConnector::InstallMsgFunc(nd_iconn_func func, ndmsgid_t maxid, ndmsgid_t minid)
{
	if(m_objhandle)
		nd_msgentry_install(m_objhandle, (nd_usermsg_func)func,  maxid,  minid,EPL_CONNECT) ;
}

void NDConnector::SetDftMsgHandler(nd_iconn_func func)
{
    if(m_objhandle)
        nd_msgentry_def_handler((nd_netui_handle)m_objhandle, (nd_usermsg_func)func) ;

}

int NDConnector::CheckValid()
{
	if(!m_objhandle)
		return 0 ;
	return 	nd_connector_valid((nd_netui_handle)m_objhandle) ;
}

int NDConnector::ExchangeKey(void *output_key)
{
	if(!m_objhandle)
		return -1 ;

	return 	nd_exchange_key((netObject)m_objhandle,output_key) ;
}


NDIConn * htoConnector(nd_handle h)
{
	return (NDIConn*) (((nd_netui_handle)h)->user_data  );
}


NDIConn* CreateConnectorObj(const char *protocol_name)
{
	NDConnector *pConn = new NDConnector() ;
	if(!pConn) {
		return NULL ;
	}
 
 	if(-1==pConn->Create(protocol_name)) {
 		delete pConn ;
 		return NULL ;
 	}
	return pConn ;
}

void DestroyConnectorObj(NDIConn *pconn) 
{
	NDConnector *p = (NDConnector*)pconn ;
	delete p;
}

int InitNet() 
{
	//char *config_file = NULL ;
	const char *argv[] = {"ndclient"} ;
	nd_arg(1, argv);

	nd_common_init() ;
	nd_net_init() ;
	//RSAinit_random(&__rsa_contex.randomStruct);
	nd_net_set_crypt((nd_netcrypt)nd_TEAencrypt, (nd_netcrypt)nd_TEAdecrypt, sizeof(tea_v)) ;
	return 0 ;
}

void DeinitNet() 
{
	nd_net_destroy() ;
	nd_common_release() ;
}


struct msg_entry_node
{
    NDUINT32			level;
    nd_usermsg_func		entry ;	//»Îø⁄∫Ø ˝
};

struct sub_msgentry
{
    struct msg_entry_node   msg_buf[SUB_MSG_NUM] ;
};

struct msgentry_root
{
    ND_OBJ_BASE;
    int	main_num ;			//∞¸∫¨∂‡…Ÿ∏ˆœ˚œ¢¿‡±
    int msgid_base ;		//÷˜œ˚œ¢∫≈∆ ºµÿ÷∑
    nd_usermsg_func		def_entry ;	//ƒ¨»œ»Îø⁄∫Ø ˝
    struct sub_msgentry sub_buf[ND_MAIN_MSG_CAPACITY] ;
};



int cliconn_translate_message(nd_netui_handle connect_handle, nd_packhdr_t *msg ,nd_handle listen_handle)
{
	ENTER_FUNC()
	int ret = 0 ;
	int data_len = nd_pack_len(msg);
	struct msgentry_root *root_entry= NULL;
	nd_usermsg_func  func = NULL ;
	nd_usermsghdr_t *usermsg =  (nd_usermsghdr_t *) msg ;

	nd_assert(msg) ;
	nd_assert(connect_handle) ;

	root_entry =(struct msgentry_root *)nd_get_msg_hadle(connect_handle);

	if(root_entry) {
		ndmsgid_t main_index , minid;
		nd_netmsg_ntoh(usermsg) ;
		main_index = usermsg->maxid - root_entry->msgid_base;
		minid = usermsg->minid ;
		if(main_index >= root_entry->main_num || minid>=SUB_MSG_NUM){
			nd_object_seterror((nd_handle)connect_handle, NDERR_INVALID_INPUT) ;
			LEAVE_FUNC();
			return -1 ;
		}

		func = root_entry->sub_buf[main_index].msg_buf[minid].entry ;
        if(!func && root_entry->def_entry){
            func = root_entry->def_entry ;
        }
        
		if(func) {
			NDIConn *pc = htoConnector((nd_handle)connect_handle);
            nd_iconn_func handler = (nd_iconn_func) func ;
			ret = handler(pc, (nd_usermsgbuf_t*)usermsg) ;
		}
	}

	if(-1==ret) {
		nd_object_seterror((nd_handle)connect_handle, NDERR_USER) ;
		LEAVE_FUNC();
		return -1 ;
	}
	LEAVE_FUNC();
	return  data_len ;
}