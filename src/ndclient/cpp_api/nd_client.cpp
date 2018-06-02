/* file nd_connector.cpp
 *
 * implemention of connector 
 *
 * create by duan 
 * 2011/4/2 15:59:39
 */

//#define BUILD_AS_STATIC_LIB
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
#include "ndapplib/nd_datatransfer.h"
//#include "pg_loginmsg.h"

#include "ndcli/nd_iconn.h"
#include "nd_msg.h"
#include "ndcli/nd_api_c.h"
//extern int nd_exchange_key(nd_handle nethandle) ;

CPPAPI void tryto_terminate(netObject netObj) ;
CPPAPI int _big_data_recv_handler(NDIConn* pconn, nd_usermsgbuf_t *msg );

//#define  WAITMSG_TIMEOUT 300000


NDObject * NDObject::FromHandle(nd_handle h)
{
	return htoConnector(h);
}


class NDConnector : public NDIConn 
{
public :		
	/*virtual void OnCreate() { return; }
	virtual void OnDestroy() {}
	
	virtual nd_handle GetMmpool() {
		return nd_global_mmpool();
	}
	virtual int SetMmpool(nd_handle pool) {
		return 0;
	}*/

	virtual void *getScriptEngine();
	virtual const char *getName();
	virtual void setName(const char *name);

	void Destroy(int flag = 0);
	int Create(const char *protocol_name=NULL) ;
	int Open(const char*host, int port,const char *protocol_name, nd_proxy_info *proxy=NULL);
	int Open(ndip_t& ip, int port,const char *protocol_name, nd_proxy_info *proxy=NULL) ;
	int Close(int force=0);

	int Send(int maxid, int minid, void *data, size_t size) ;
	int SendMsg(NDSendMsg &msg, int flag=ESF_NORMAL) ;
	int SendMsg(nd_usermsgbuf_t *msghdr, int flag=ESF_NORMAL);
	int SendRawData(void *data , size_t size) ;
	int RecvRawData(void *buf, size_t size, ndtime_t waittm) ;
	
	int BigDataSend(NDUINT64 param, void *data, size_t datalen);
	
	int CryptMsg(nd_usermsgbuf_t *msghdr, bool bEncrypt=true) ;
	
	int CheckValid();
	int WaitMsg(nd_usermsgbuf_t *msgbuf, ndtime_t wait_time=100);
	int Update(ndtime_t wait_time);
	void InstallMsgFunc(nd_iconn_func, ndmsgid_t maxid, ndmsgid_t minid,const char *name=NULL);
	int CallMsgHandle(nd_usermsgbuf_t *msgbuf);

	bool TestMsgIsHandle(ndmsgid_t maxid, ndmsgid_t minid);
    void SetDftMsgHandler(nd_iconn_func func);
    
	NDConnector(int maxmsg_num =ND_MAIN_MSG_CAPACITY, int maxid_start=ND_MSG_BASE_ID) ;
	void SetMsgNum(int maxmsg_num , int maxid_start=ND_MSG_BASE_ID) ;
	virtual~NDConnector() ;
    
	int Reconnect(ndip_t& IP, int port,nd_proxy_info *proxy=NULL) ;//connect to another host

	nd_handle GetHandle() {return m_objhandle;}
	int ExchangeKey(void *output_key) ;

	int LastError() ;
	void SetLastError(NDUINT32 errcode);
	const char *ErrorDesc() ;
	const char *ConvertErrorDesc(NDUINT32 errcode) ;

    void *GetUserData() { return __userData ;}
    void SetUserData(void *pData){ __userData = pData ;}

	int ioctl(int cmd, void *val, int *size) ;
	
	void SetBigDataHandler(nd_bigdata_handler entry) ;

	NDBigDataReceiver *getBigDataRecver() {return &m_dataRecv;} 
	int GetStatus();
	
private:	
	//nd_handle m_objhandle ;
	int msg_kinds ;
	int msg_base ;
	nd_handle m_objhandle ;

    void *__userData ;
	
	NDBigDataReceiver m_dataRecv ;
};


NDConnector::NDConnector(int maxmsg_num , int maxid_start):m_dataRecv(NULL)
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

void *NDConnector::getScriptEngine()
{
	if (m_objhandle) {
		return nd_message_get_script_engine(m_objhandle);
	}
	return NULL;
}
const char *NDConnector::getName()
{
	if (m_objhandle) {
		return nd_object_get_instname(m_objhandle);
	}
	return NULL;

}
void NDConnector::setName(const char *name)
{
	if (m_objhandle) {
		nd_object_set_instname(m_objhandle,name);
	}
}

int NDConnector::ioctl(int cmd, void *val, int *size)
{
	if(!m_objhandle)
		return -1 ;
	else
		return nd_net_ioctl((nd_netui_handle)m_objhandle, cmd, val, size) ;
}


void NDConnector::SetBigDataHandler(nd_bigdata_handler entry) 
{
	m_dataRecv.SetHandler((data_recv_callback)entry) ;
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

const char *NDConnector::ConvertErrorDesc(NDUINT32 errcode)
{
	return nd_error_desc(errcode) ;
}

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
		nd_logerror("connect error %s %d :%s!" AND host AND port AND nd_last_error()) ;
		nd_object_destroy(m_objhandle,1) ;
		m_objhandle = NULL ;
		return -1;
	}
	m_dataRecv.SetReceive(m_objhandle) ;
	InstallMsgFunc(_big_data_recv_handler, ND_MAIN_ID_SYS, ND_MSG_BIG_DATA_TRANSFER) ;

	return 0 ;

}

int NDConnector::Open(ndip_t& ip, int port,const char *protocol_name, nd_proxy_info *proxy) 
{
	char iptext[64];
	return Open(ND_INET_NTOA(ip, iptext), port, protocol_name, proxy) ;
}

int NDConnector::Reconnect(ndip_t& IP, int port,nd_proxy_info *proxy)
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
		nd_logerror("NDConnector::create connector (%s) object error :%s!" AND protocol_name AND nd_last_error()) ;
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
	int val = 1;
	int size = sizeof(val);
	//nd_net_ioctl((nd_netui_handle)m_objhandle, NDIOCTL_LOG_RECV_MSG, &val, &size);
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
	return SendMsg(msg.GetMsgAddr(), flag) ;
}

int NDConnector::Send(int maxid, int minid, void *data, size_t size) 
{
	if (!data || size == 0) {
		nd_usermsghdr_t header;
		nd_usermsghdr_init(&header);
		header.maxid = maxid;
		header.minid = minid;
		return ::nd_connector_send(m_objhandle, &header.packet_hdr, ESF_NORMAL);
	}
	else {
		NDOStreamMsg omsg(maxid, minid);
		omsg.WriteStream((char*)data, size);
		return SendMsg(omsg);
	}

}

int NDConnector::SendMsg(nd_usermsgbuf_t *msghdr, int flag)
{
	nd_assert(m_objhandle);
	ND_USERMSG_SYS_RESERVED(msghdr) = 0 ;
	int ret = nd_connector_send(m_objhandle,(nd_packhdr_t*)msghdr, flag) ;
	if (-1==ret && NDERR_WOULD_BLOCK != nd_object_lasterror(m_objhandle)) {
		tryto_terminate((netObject)m_objhandle) ;
	}
	return ret ;

}

int NDConnector::SendRawData(void *data , size_t size) 
{
	nd_assert(m_objhandle) ;
	int ret = nd_connector_raw_write(m_objhandle,data,size) ;
	if (-1 == ret && NDERR_WOULD_BLOCK != nd_object_lasterror(m_objhandle)) {
		tryto_terminate((netObject)m_objhandle) ;
	}
	return ret ;

}

int NDConnector::BigDataSend(NDUINT64 param,  void *data, size_t datalen) 
{	
	return BigDataAsyncSend(m_objhandle, data, datalen,  param, NULL) ;
}

int NDConnector::CryptMsg(nd_usermsgbuf_t *msghdr, bool bEncrypt) 
{
	if (bEncrypt) {
		return nd_packet_encrypt(m_objhandle, (nd_packetbuf_t*) msghdr) ;
	}
	else {
		return nd_packet_encrypt(m_objhandle, (nd_packetbuf_t*)msghdr) ;
	}
	
}

int NDConnector::RecvRawData(void *buf, size_t size, ndtime_t waittm) 
{
	nd_assert(m_objhandle) ;
	int ret = nd_connector_raw_waitdata(m_objhandle, buf, size, waittm) ;
		
	if (-1==ret ) {
		if (NDERR_WOULD_BLOCK != nd_object_lasterror(m_objhandle) &&
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
	if (!m_objhandle) {
		return -1;
	}
	if(m_objhandle->type==NDHANDLE_UDPNODE) {
		nd_usermsgbuf_t msg_recv;
RE_WAIT:
		ret = nd_connector_waitmsg(m_objhandle, (nd_packetbuf_t *)&msg_recv,wait_time);
		if(ret > 0) {			
			//msg_entry(connect_handle, &msg_recv) ;
			//cliconn_translate_message((nd_netui_handle)m_objhandle, (nd_packhdr_t*)&msg_recv, 0) ;
			nd_translate_message_ex(m_objhandle, (nd_packhdr_t*)&msg_recv, 0, (nd_handle)this);
			wait_time = 0;
			goto RE_WAIT;
			//return 0;
		}

	}
	else {
		ret = nd_connector_update(m_objhandle,wait_time) ;
	}
	
	if (-1==ret ) {
		if (NDERR_WOULD_BLOCK != nd_object_lasterror(m_objhandle) &&
			NDERR_TIMEOUT != nd_object_lasterror(m_objhandle)  ) {
			tryto_terminate((netObject)m_objhandle) ;
		}
	}
	
	return ret ;

}

int NDConnector::CallMsgHandle(nd_usermsgbuf_t *msgbuf)
{
	return nd_translate_message_ex(m_objhandle, (nd_packhdr_t*)msgbuf, NULL, (nd_handle)this);
}

bool NDConnector::TestMsgIsHandle(ndmsgid_t maxid, ndmsgid_t minid)
{
	return nd_msgentry_is_handled(m_objhandle,maxid, minid)? true : false;
}

int NDConnector::WaitMsg(nd_usermsgbuf_t*msgbuf, ndtime_t wait_time)
{
	int ret = nd_connector_waitmsg(m_objhandle, (nd_packetbuf_t *)msgbuf,wait_time);
	
	if (-1==ret ) {
		if (NDERR_WOULD_BLOCK != nd_object_lasterror(m_objhandle) &&
			NDERR_TIMEOUT != nd_object_lasterror(m_objhandle) &&
			NDERR_INVALID_INPUT != nd_object_lasterror(m_objhandle) ) {
			tryto_terminate((netObject)m_objhandle) ;
		}
	}
	
	return ret ;

}
void NDConnector::InstallMsgFunc(nd_iconn_func func, ndmsgid_t maxid, ndmsgid_t minid,const char *name)
{
	if(m_objhandle)
		nd_msgentry_install(m_objhandle, (nd_usermsg_func)func,  maxid,  minid,EPL_CONNECT, name) ;
}

void NDConnector::SetDftMsgHandler(nd_iconn_func func)
{
    if(m_objhandle)
        nd_msgentry_def_handler(m_objhandle, (nd_usermsg_func)func) ;

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

int NDConnector::GetStatus()
{
	if (CheckValid()) {
		return nd_connect_level_get(m_objhandle);
	}
	else {
		return EPL_NONE;
	}
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


void* ndSetLogoutFunc(void *func)
{
	return (void*)nd_setlog_func((nd_log_entry)func);
}
void ndSetLogFile(const char *pathfile)
{
	set_log_file(pathfile);
}


int cliconn_translate_message(nd_netui_handle connect_handle, nd_packhdr_t *msg ,nd_handle listen_handle)
{
	if (connect_handle->is_log_recv){
		nd_usermsghdr_t *pmsgHdr = (nd_usermsghdr_t*)msg;
		nd_logmsg("received (%d,%d) len=%d\n", pmsgHdr->maxid, pmsgHdr->minid, ND_USERMSG_LEN(msg));
	}

	return nd_translate_message_ex((nd_handle)connect_handle, msg, 0, (nd_handle)htoConnector((nd_handle)connect_handle));
	
}


int _big_data_recv_handler(NDIConn* pconn, nd_usermsgbuf_t *msg )
{
	NDConnector *pNetObj = (NDConnector*)pconn ;
	int len = ND_USERMSG_DATALEN(msg) ;
	
	NDIStreamMsg inmsg(msg) ;
	len = pNetObj->getBigDataRecver()->OnRecv(inmsg) ;
	if (len != NDERR_SUCCESS && len != NDERR_WOULD_BLOCK) {
		nd_logerror("error onRecv big data message ret=%d\n", len) ;
	}
	return 0 ;
}



