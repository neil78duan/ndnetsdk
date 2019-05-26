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
//#include "ndapplib/nd_datatransfer.h"
//#include "pg_loginmsg.h"

#include "ndcli/nd_iconn.h"
#include "nd_msg.h"
//#include "ndcli/nd_api_c.h"
//extern int nd_exchange_key(nd_handle nethandle) ;

CPPAPI void tryto_terminate(nd_handle netObj) ;
//CPPAPI int _big_data_recv_handler(NDIConn* pconn, nd_usermsgbuf_t *msg );

//#define  WAITMSG_TIMEOUT 300000

//
//NDObject * NDObject::FromHandle(nd_handle h)
//{
//	return htoConnector(h);
//}

//
//class NDCliConnector : public NDConnector
//{
//
//public:
//	NDCliConnector();
//	virtual ~NDCliConnector() ;
//
//	virtual int Open(const char*host, int port,const char *protocol_name, void *proxy=NULL) ;
//	virtual int Open(ndip_t& ip, int port,const char *protocol_name, void *proxy=NULL) ;
//	virtual int Close(int force=0) = 0;
//
//	int CallMsgHandle(nd_usermsgbuf_t *msgbuf)  ;
//	bool TestMsgIsHandle(ndmsgid_t maxid, ndmsgid_t minid) ;
//	void SetDftMsgHandler(nd_iconn_func) ;
//	void SetMsgNum(int maxmsg_num , int maxid_start) ;
//	int Reconnect(ndip_t& IP, int port,void *proxy=NULL)  ;//connect to another host
//	int ExchangeKey(void *output_key) =0;
//	const char *ErrorDesc() =0;
//	const char *ConvertErrorDesc(NDUINT32 errcode) =0;
//
//	void *GetUserData();
//	void SetUserData(void *pData);
//
//protected:
//
//	void *__userData ;
//}


NDCliConnector::NDCliConnector(int maxmsg_num , int maxid_start ):
	NDConnector(maxmsg_num,maxid_start)
{
	
}
NDCliConnector::~NDCliConnector()
{
	
}

//
//class NDConnector : public NDIConn
//{
//public :
//
//	virtual void *getScriptEngine();
//	virtual const char *getName();
//	virtual void setName(const char *name);
//
//	void Destroy(int flag = 0);
//	int Create(const char *protocol_name=NULL) ;
//	int Open(const char*host, int port,const char *protocol_name, void *proxy=NULL);
//	int Open(ndip_t& ip, int port,const char *protocol_name, void *proxy=NULL) ;
//	int Close(int force=0);
//
//	int Send(int maxid, int minid, void *data, size_t size) ;
//	int SendMsg(NDSendMsg &msg, int flag=ESF_NORMAL) ;
//	int SendMsg(nd_usermsgbuf_t *msghdr, int flag=ESF_NORMAL);
//	int SendRawData(void *data , size_t size) ;
//	int RecvRawData(void *buf, size_t size, ndtime_t waittm) ;
//
//	int CryptMsg(nd_usermsgbuf_t *msghdr, bool bEncrypt=true) ;
//
//	int CheckValid();
//	int WaitMsg(nd_usermsgbuf_t *msgbuf, ndtime_t wait_time=100);
//	int Update(ndtime_t wait_time);
//	void InstallMsgFunc(nd_iconn_func, ndmsgid_t maxid, ndmsgid_t minid,const char *name=NULL);
//	int CallMsgHandle(nd_usermsgbuf_t *msgbuf);
//
//	bool TestMsgIsHandle(ndmsgid_t maxid, ndmsgid_t minid);
//    void SetDftMsgHandler(nd_iconn_func func);
//
//	NDConnector(int maxmsg_num =ND_MAIN_MSG_CAPACITY, int maxid_start=ND_MSG_BASE_ID) ;
//	void SetMsgNum(int maxmsg_num , int maxid_start=ND_MSG_BASE_ID) ;
//	virtual~NDConnector() ;
//
//	int Reconnect(ndip_t& IP, int port,void *proxy=NULL) ;//connect to another host
//
//	nd_handle GetHandle() {return m_objhandle;}
//	int ExchangeKey(void *output_key) ;
//
//	int LastError() ;
//	void SetLastError(NDUINT32 errcode);
//	const char *ErrorDesc() ;
//	const char *ConvertErrorDesc(NDUINT32 errcode) ;
//
//    void *GetUserData() { return __userData ;}
//    void SetUserData(void *pData){ __userData = pData ;}
//
//	int ioctl(int cmd, void *val, int *size) ;
//
//	void SetBigDataHandler(nd_bigdata_handler entry) ;
//
//	int GetStatus();
//
//private:
//	//nd_handle m_objhandle ;
//	int msg_kinds ;
//	int msg_base ;
//	nd_handle m_objhandle ;
//
//    void *__userData ;
//
//};
//
//
//NDConnector::NDConnector(int maxmsg_num , int maxid_start)
//{
//	m_objhandle = NULL ;
//	msg_kinds = maxmsg_num;
//	msg_base = maxid_start;
//
//    __userData = 0 ;
//}
//
//NDConnector::~NDConnector()
//{
//	Destroy(0) ;
//}
//
//int NDConnector::LastError()
//{
//	if(!m_objhandle)
//		return NDERR_INVALID_HANDLE ;
//	else
//		return nd_object_lasterror(m_objhandle) ;
//
//}
//
//void *NDConnector::getScriptEngine()
//{
//	if (m_objhandle) {
//		return nd_message_get_script_engine(m_objhandle);
//	}
//	return NULL;
//}
//const char *NDConnector::getName()
//{
//	if (m_objhandle) {
//		return nd_object_get_instname(m_objhandle);
//	}
//	return NULL;
//
//}
//void NDConnector::setName(const char *name)
//{
//	if (m_objhandle) {
//		nd_object_set_instname(m_objhandle,name);
//	}
//}
//
//int NDConnector::ioctl(int cmd, void *val, int *size)
//{
//	if(!m_objhandle)
//		return -1 ;
//	else
//		return nd_net_ioctl((nd_netui_handle)m_objhandle, cmd, val, size) ;
//}
//
//
//void NDConnector::SetLastError(NDUINT32 errcode)
//{
//	if(m_objhandle)
//		nd_object_seterror(m_objhandle,errcode) ;
//}

const char *NDCliConnector::ErrorDesc()
{
	if(m_objhandle)
		return nd_object_errordesc(m_objhandle) ;
	return NULL;
}

const char *NDCliConnector::ConvertErrorDesc(NDUINT32 errcode)
{
	return nd_error_desc(errcode) ;
}
//
//void NDConnector::SetMsgNum(int maxmsg_num , int maxid_start)
//{
//	msg_kinds = maxmsg_num;
//	msg_base = maxid_start;
//}

int NDCliConnector::Open(const char *host, int port,const char *protocol_name,void *proxy)
{
	if(!m_objhandle) {
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
	return 0 ;

}

int NDCliConnector::Open(ndip_t& ip, int port,const char *protocol_name, void *proxy)
{
	char iptext[64];
	return Open(ND_INET_NTOA(ip, iptext), port, protocol_name, proxy) ;
}

int NDCliConnector::Reconnect(ndip_t& IP, int port,void *proxy)
{
	return ::nd_reconnect(m_objhandle,  IP,  port, (nd_proxy_info*)proxy) ;
}
int NDCliConnector::Close(int force)
{
	if(m_objhandle) {
		int ret =  nd_connector_close(m_objhandle, force) ;
		if(ret==0) {}
		return ret;
	}
	return -1 ;
}

//static int cliconn_translate_message(nd_netui_handle connect_handle, nd_packhdr_t *msg ,nd_handle listen_handle)  ;
int NDCliConnector::Create(const char *protocol_name)
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
	//((nd_netui_handle)m_objhandle)->user_data =(void*) this ;
	((nd_netui_handle)m_objhandle)->msg_caller =(void*) this ;

	//set message handle	
	if (msg_kinds > 0){
		if(-1==nd_msgtable_create(m_objhandle, msg_kinds, msg_base) ) {
			nd_object_destroy(m_objhandle, 0) ;
		}
	}
	return 0 ;

}
void NDCliConnector::Destroy(int flag)
{
	if(m_objhandle && ((nd_netui_handle)m_objhandle)->msg_caller ==(void*) this){
		nd_connector_close(m_objhandle, 0) ;
		nd_msgtable_destroy(m_objhandle, 0);
		nd_object_destroy(m_objhandle, 0) ;
		m_objhandle = 0 ;
	}
}
//
//int NDConnector::SendMsg(NDSendMsg &msg, int flag)
//{
//	return SendMsg(msg.GetMsgAddr(), flag) ;
//}
//
//int NDConnector::Send(int maxid, int minid, void *data, size_t size)
//{
//	if (!data || size == 0) {
//		nd_usermsghdr_t header;
//		nd_usermsghdr_init(&header);
//		header.maxid = maxid;
//		header.minid = minid;
//		return ::nd_connector_send(m_objhandle, &header.packet_hdr, ESF_NORMAL);
//	}
//	else {
//		NDOStreamMsg omsg(maxid, minid);
//		omsg.WriteStream((char*)data, size);
//		return SendMsg(omsg);
//	}
//
//}
//
//int NDConnector::SendMsg(nd_usermsgbuf_t *msghdr, int flag)
//{
//	nd_assert(m_objhandle);
//	ND_USERMSG_SYS_RESERVED(msghdr) = 0 ;
//	int ret = nd_connector_send(m_objhandle,(nd_packhdr_t*)msghdr, flag) ;
//	if (-1==ret && NDERR_WOULD_BLOCK != nd_object_lasterror(m_objhandle)) {
//		tryto_terminate(m_objhandle) ;
//	}
//	return ret ;
//
//}
//
//int NDConnector::SendRawData(void *data , size_t size)
//{
//	nd_assert(m_objhandle) ;
//	int ret = nd_connector_raw_write(m_objhandle,data,size) ;
//	if (-1 == ret && NDERR_WOULD_BLOCK != nd_object_lasterror(m_objhandle)) {
//		tryto_terminate(m_objhandle) ;
//	}
//	return ret ;
//
//}
//
//
//int NDConnector::CryptMsg(nd_usermsgbuf_t *msghdr, bool bEncrypt)
//{
//	if (bEncrypt) {
//		return nd_packet_encrypt(m_objhandle, (nd_packetbuf_t*) msghdr) ;
//	}
//	else {
//		return nd_packet_encrypt(m_objhandle, (nd_packetbuf_t*)msghdr) ;
//	}
//
//}
//
//int NDConnector::RecvRawData(void *buf, size_t size, ndtime_t waittm)
//{
//	nd_assert(m_objhandle) ;
//	int ret = nd_connector_raw_waitdata(m_objhandle, buf, size, waittm) ;
//
//	if (-1==ret ) {
//		if (NDERR_WOULD_BLOCK != nd_object_lasterror(m_objhandle) &&
//			NDERR_TIMEOUT != nd_object_lasterror(m_objhandle) &&
//			NDERR_INVALID_INPUT != nd_object_lasterror(m_objhandle) ) {
//			tryto_terminate(m_objhandle) ;
//		}
//	}
//
//	return ret ;
//
//}
//
//int NDConnector::Update(ndtime_t wait_time)
//{
//
//	int ret = 0;
//	//nd_msgui_buf msg_recv;
//	if (!m_objhandle) {
//		return -1;
//	}
//	if(m_objhandle->type==NDHANDLE_UDPNODE) {
//		nd_usermsgbuf_t msg_recv;
//RE_WAIT:
//		ret = nd_connector_waitmsg(m_objhandle, (nd_packetbuf_t *)&msg_recv,wait_time);
//		if(ret > 0) {
//			nd_translate_message(m_objhandle, (nd_packhdr_t*)&msg_recv, 0);
//			wait_time = 0;
//			goto RE_WAIT;
//			//return 0;
//		}
//
//	}
//	else {
//		ret = nd_connector_update(m_objhandle,wait_time) ;
//	}
//
//	if (-1==ret ) {
//		if (NDERR_WOULD_BLOCK != nd_object_lasterror(m_objhandle) &&
//			NDERR_TIMEOUT != nd_object_lasterror(m_objhandle)  ) {
//			tryto_terminate(m_objhandle) ;
//		}
//	}
//
//	return ret ;
//
//}

int NDCliConnector::CallMsgHandle(nd_usermsgbuf_t *msgbuf)
{
	return nd_translate_message(m_objhandle, (nd_packhdr_t*)msgbuf, NULL);
}

bool NDCliConnector::TestMsgIsHandle(ndmsgid_t maxid, ndmsgid_t minid)
{
	return nd_msgentry_is_handled(m_objhandle,maxid, minid)? true : false;
}
//
//int NDConnector::WaitMsg(nd_usermsgbuf_t*msgbuf, ndtime_t wait_time)
//{
//	int ret = nd_connector_waitmsg(m_objhandle, (nd_packetbuf_t *)msgbuf,wait_time);
//
//	if (-1==ret ) {
//		if (NDERR_WOULD_BLOCK != nd_object_lasterror(m_objhandle) &&
//			NDERR_TIMEOUT != nd_object_lasterror(m_objhandle) &&
//			NDERR_INVALID_INPUT != nd_object_lasterror(m_objhandle) ) {
//			tryto_terminate(m_objhandle) ;
//		}
//	}
//
//	return ret ;
//
//}

//void NDCliConnector::InstallMsgFunc(nd_iconn_func func, ndmsgid_t maxid, ndmsgid_t minid,const char *name)
//{
//	if(m_objhandle)
//		nd_msgentry_install(m_objhandle, (nd_usermsg_func)func,  maxid,  minid,EPL_CONNECT, name) ;
//}
//
//void NDCliConnector::SetDftMsgHandler(nd_iconn_func func)
//{
//    if(m_objhandle)
//        nd_msgentry_def_handler(m_objhandle, (nd_usermsg_func)func) ;
//
//}
//
//int NDConnector::CheckValid()
//{
//	if(!m_objhandle)
//		return 0 ;
//	return 	nd_connector_valid((nd_netui_handle)m_objhandle) ;
//}

int NDCliConnector::ExchangeKey(void *output_key)
{
	if(!m_objhandle)
		return -1 ;

	return 	nd_exchange_key(m_objhandle,output_key) ;
}
//
//int NDConnector::GetStatus()
//{
//	if (CheckValid()) {
//		return nd_connect_level_get(m_objhandle);
//	}
//	else {
//		return EPL_NONE;
//	}
//}
//
//NDIConn * htoConnector(nd_handle h)
//{
//	return (NDIConn*) (((nd_netui_handle)h)->user_data  );
//}


NDCliConnector* CreateConnectorObj(const char *protocol_name)
{
	NDCliConnector *pConn = new NDCliConnector() ;
	if(!pConn) {
		return NULL ;
	}
 
 	if(-1==pConn->Create(protocol_name)) {
 		delete pConn ;
 		return NULL ;
 	}
	return pConn ;
}

void DestroyConnectorObj(NDCliConnector *pconn)
{
	//NDConnector *p = (NDConnector*)pconn ;
	delete pconn;
}



void* ndSetLogoutFunc(void *func)
{
	return (void*)nd_setlog_func((nd_log_entry)func);
}
void ndSetLogFile(const char *pathfile)
{
	set_log_file(pathfile);
}

//
//int cliconn_translate_message(nd_netui_handle connect_handle, nd_packhdr_t *msg ,nd_handle listen_handle)
//{
//	if (connect_handle->is_log_recv){
//		nd_usermsghdr_t *pmsgHdr = (nd_usermsghdr_t*)msg;
//		nd_logmsg("received (%d,%d) len=%d\n", pmsgHdr->maxid, pmsgHdr->minid, ND_USERMSG_LEN(msg));
//	}
//
//	return nd_translate_message((nd_handle)connect_handle, msg, 0);
//
//}

//
//int _big_data_recv_handler(NDIConn* pconn, nd_usermsgbuf_t *msg )
//{
//	NDConnector *pNetObj = (NDConnector*)pconn ;
//	int len = ND_USERMSG_DATALEN(msg) ;
//	
//	NDIStreamMsg inmsg(msg) ;
//	len = pNetObj->getBigDataRecver()->OnRecv(inmsg) ;
//	if (len != NDERR_SUCCESS && len != NDERR_WOULD_BLOCK) {
//		nd_logerror("error onRecv big data message ret=%d\n", len) ;
//	}
//	return 0 ;
//}

///----------------------------

static int __bInit = 0 ;
static ndNetFunc __terminate_entry ;

void tryto_terminate(nd_handle netObj) ;

int ndInitNet()
{
	if (__bInit) {
		return 0;
	}
	const char *argv[] = {"ndNetCAPI"} ;
	
	nd_arg(1, argv);
	
	nd_common_init() ;
	nd_net_init() ;
	nd_net_set_crypt((nd_netcrypt)nd_TEAencrypt, (nd_netcrypt)nd_TEAdecrypt, sizeof(tea_v)) ;
	__bInit =1 ;
	__terminate_entry = 0 ;
	return 0 ;
}

void ndDeinitNet()
{
	if (__bInit==0) {
		return ;
	}
	nd_net_destroy() ;
	nd_common_release() ;
	__bInit = 0;
}


ndNetFunc ndSetTerminateFunc(ndNetFunc func)
{
	ndNetFunc ret =	__terminate_entry ;
	__terminate_entry = func;
	return ret ;
}

static int _s_waitTimeOutValue = 30000;
int ndSetTimeoutVal(int val)
{
	int ret = _s_waitTimeOutValue;
	_s_waitTimeOutValue = val;
	return ret;
}

int ndGetTimeoutVal()
{
	return _s_waitTimeOutValue;
}

int nd_checkErrorMsg(nd_handle nethandle,nd_usermsghdr_t *msg)
{
	if (msg->maxid==ND_MAIN_ID_SYS && msg->minid ==ND_MSG_SYS_ERROR) {
		NDIStreamMsg inmsg((nd_usermsgbuf_t*) msg) ;
		NDUINT32 errcode =NDERR_BADTHREAD;
		inmsg.Read(errcode) ;
		nd_object_seterror((nd_handle)nethandle, errcode) ;
		return  errcode ;
	}
	return  0 ;
	
}

int ndSendAndWaitMessage(nd_handle nethandle, nd_usermsgbuf_t *sendBuf, nd_usermsgbuf_t* recvBuf, ndmsgid_t waitMaxid, ndmsgid_t waitMinid, int sendFlag, int timeout)
{
	int ret = 0;
	ndtime_t start_tm = nd_time();
	if (nd_connector_send(nethandle, (nd_packhdr_t*)sendBuf, sendFlag) <= 0) {
		nd_object_seterror(nethandle, NDERR_WRITE);
		nd_logerror("send data error: NDERR_WRITE\n");
		return -1;
	}
RE_RECV:
	ret = nd_connector_waitmsg(nethandle, (nd_packetbuf_t *)recvBuf, timeout);
	if (ret <= 0 ) {
		//nd_object_seterror(nethandle, NDERR_TIMEOUT);
		nd_logerror("wait message timeout\n");
		return -1;
	}
	else if (recvBuf->msg_hdr.packet_hdr.ndsys_msg){
		if (-1 == nd_net_sysmsg_hander((nd_netui_handle)nethandle, (nd_sysresv_pack_t *)recvBuf)){
			nd_logerror("receive system mesaage and handler error \n");
			return -1;
		}
		if ((nd_time() - start_tm) >= timeout) {
			nd_object_seterror(nethandle, NDERR_TIMEOUT);
			nd_logerror("wait message(%d,%d) timeout\n", waitMaxid, waitMinid);
			return -1;
		}
		goto RE_RECV;
	}
	else if (nd_checkErrorMsg(nethandle, &recvBuf->msg_hdr)) {
		nd_logerror("receive error message \n");
		return -1;
	}
	else if (ND_USERMSG_MAXID(recvBuf) != waitMaxid || ND_USERMSG_MINID(recvBuf) != waitMinid) {
		if ((nd_time() - start_tm) >= timeout) {
			nd_object_seterror(nethandle, NDERR_TIMEOUT);
			nd_logerror("wait message(%d,%d) timeout\n", waitMaxid, waitMinid);
			return -1;
		}
		if (((nd_netui_handle)nethandle)->msg_handle) {
			//int ret = nd_translate_message(nethandle, (nd_packhdr_t*)recvBuf, NULL);
			int ret = _packet_handler((nd_netui_handle)nethandle, &recvBuf->msg_hdr.packet_hdr, NULL);
			if (ret == -1 && NDERR_UNHANDLED_MSG != nd_object_lasterror(nethandle) ){
				nd_logerror("wait message(%d,%d) error ,recvd(%d,%d)\n", waitMaxid, waitMinid, ND_USERMSG_MAXID(recvBuf), ND_USERMSG_MINID(recvBuf));
				return ret;
			}
		}
		goto RE_RECV;
	}
	return 0;
}

void tryto_terminate(nd_handle netObj)
{
	if (__terminate_entry) {
		nd_netui_handle handle = (nd_netui_handle) netObj ;
		if (handle->status != ETS_DEAD && handle->status != ETS_CLOSED) {
			__terminate_entry(netObj, NULL, 0) ;
		}
	}
}
