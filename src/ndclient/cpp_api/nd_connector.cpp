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

#define  WAITMSG_TIMEOUT 10000

class NDConnector : public NDIConn 
{
public :		
	void Destroy(int flag = 0);
	int Create(char *protocol_name=NULL) ;
	int Open(char*host, int port,char *protocol_name, pg_proxy_info *proxy=NULL);
	int Close(int force=0);

	int Send(int maxid, int minid, void *data, size_t size) ;
	int SendMsg(NDSendMsg &msg, int flag=ESF_NORMAL) ;
	int SendMsg(nd_usermsgbuf_t *msghdr, int flag=ESF_NORMAL);
	int SendRawData(void *data , size_t size) ;
	int RecvRawData(void *buf, size_t size, ndtime_t waittm) ;

	int CheckValid();
	int WaitMsg(nd_usermsgbuf_t *msgbuf, ndtime_t wait_time=100);
	int Update(ndtime_t wait_time);
	void InstallMsgFunc( nd_msg_func, ndmsgid_t maxid, ndmsgid_t minid);

	NDConnector(int maxmsg_num =16, int maxid_start=0) ;
	void SetMsgNum(int maxmsg_num , int maxid_start=0) ;
	virtual~NDConnector() ;

	int Reconnect(ndip_t IP, int port,pg_proxy_info *proxy=NULL) ;//connect to another host

	NDUINT32 GetID() {return m_id;}
	void SetID(NDUINT32 id) {m_id = id;}
	NDUINT32 GetType() {return m_type;}
	void SetType(NDUINT32 type) {m_type = type;}
	nd_handle GetHandle() {return m_objhandle;}
	int ExchangeKey() ;

	int LastError() ;
	void SetLastError(NDUINT32 errcode);
	const char *ErrorDesc() ;

private:
	//nd_handle m_objhandle ;
	int msg_kinds ;
	int msg_base ;

	NDUINT32 m_id ;				//id
	NDUINT32 m_type ;			//¿‡–Õ
	nd_handle m_objhandle ;

	//crypt key
	int __pki_ok ;
	char __rsa_digest[16] ;
	ND_RSA_CONTEX __rsa_contex;
	tea_k	__crypt_key;
};


NDConnector::NDConnector(int maxmsg_num , int maxid_start) 
{
	m_objhandle = NULL ;
	msg_kinds = maxmsg_num;
	msg_base = maxid_start;
	m_id = 0 ;				//id
	m_type  = 0;			//¿‡–Õ
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

int NDConnector::Open(char *host, int port, char *protocol_name,pg_proxy_info *proxy)
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

int NDConnector::Reconnect(ndip_t IP, int port,pg_proxy_info *proxy)
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

int pg_translate_message(nd_netui_handle connect_handle, nd_packhdr_t *msg ,nd_handle listen_handle)  ;
int NDConnector::Create(char *protocol_name) 
{
	//connect to host 
	if (m_objhandle) {
		Destroy() ;
		m_objhandle = NULL;
	}
	m_objhandle = nd_object_create(protocol_name? protocol_name: (char*)"tcp-connector"  ) ;

	if(!m_objhandle){		
		nd_logerror((char*)"connect error :%s!" AND nd_last_error()) ;
		return -1;
	}
	((nd_netui_handle)m_objhandle)->user_data =(void*) this ;

	//set message handle	
	if (msg_kinds > 0){
		if(-1==nd_msgtable_create(m_objhandle, msg_kinds, msg_base) ) {
			nd_object_destroy(m_objhandle, 0) ;
		}
		nd_hook_packet(m_objhandle,(net_msg_entry )pg_translate_message);
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
	return nd_connector_send(m_objhandle,(nd_packhdr_t*) (msg.GetMsgAddr()), flag) ;
}

int NDConnector::Send(int maxid, int minid, void *data, size_t size) 
{
	NDOStreamMsg omsg(maxid, minid) ;
	if(-1==omsg.WriteBin(data, size) ) {
		return -1 ;
	}
	return nd_connector_send(m_objhandle,(nd_packhdr_t*) (omsg.GetMsgAddr()),ESF_NORMAL) ;

}

int NDConnector::SendMsg(nd_usermsgbuf_t *msghdr, int flag)
{
	return nd_connector_send(m_objhandle,(nd_packhdr_t*)msghdr, flag) ;
}

int NDConnector::SendRawData(void *data , size_t size) 
{
	nd_assert(m_objhandle) ;
	return nd_connector_raw_write(m_objhandle,data,size) ;

}

int NDConnector::RecvRawData(void *buf, size_t size, ndtime_t waittm) 
{
	nd_assert(m_objhandle) ;
	return nd_connector_raw_waitdata(m_objhandle, buf, size, waittm) ;
}

int NDConnector::Update(ndtime_t wait_time)
{

	int ret;
	//nd_msgui_buf msg_recv;
	if(m_objhandle->type==NDHANDLE_UDPNODE) {
		nd_usermsgbuf_t msg_recv;
RE_WAIT:
		ret = nd_connector_waitmsg(m_objhandle, (nd_packetbuf_t *)&msg_recv,wait_time);
		if(ret > 0) {			
			//msg_entry(connect_handle, &msg_recv) ;
			nd_translate_message(m_objhandle, (nd_packhdr_t*)&msg_recv, 0) ;
			wait_time = 0;
			goto RE_WAIT;
			//return 0;
		}
		else {
			return ret ;
		}

	}
	else {
		return nd_connector_update(m_objhandle,wait_time) ;
	}
}

int NDConnector::WaitMsg(nd_usermsgbuf_t*msgbuf, ndtime_t wait_time)
{
	return nd_connector_waitmsg(m_objhandle, (nd_packetbuf_t *)msgbuf,wait_time);
}
void NDConnector::InstallMsgFunc(nd_msg_func func, ndmsgid_t maxid, ndmsgid_t minid)
{
	if(m_objhandle)
		nd_msgentry_install(m_objhandle, (nd_usermsg_func)func,  maxid,  minid,EPL_CONNECT) ;
}

int NDConnector::CheckValid()
{
	if(!m_objhandle)
		return 0 ;
	return 	nd_connector_valid((nd_netui_handle)m_objhandle) ;
}

/*
 
int NDConnector::ExchangeKey()
{
	NDOStreamMsg omsg(PGMAXID_LOGIN, MSG_LOGIN_PKI_KEY_REQ) ;
	if(SendMsg(omsg,ESF_URGENCY) <= 0) {
		return -1 ;
	}
	//get public key 
	pg_usermsgbuf_t rmsg ;
	if(-1==WaitMsg(&rmsg, WAITMSG_TIMEOUT)) {
		return -1 ;
	}

	size_t data_len = ND_USERMSG_LEN(&rmsg) - ND_USERMSG_HDRLEN;

	if(ND_USERMSG_MAXID(&rmsg)!=PGMAXID_LOGIN || 
		ND_USERMSG_MINID(&rmsg) != MSG_LOGIN_PKI_KEY_ACK||
		data_len!=sizeof(R_RSA_PUBLIC_KEY)) {
			return -1 ;
	}	
	memcpy(&__rsa_contex.publicKey, ND_USERMSG_DATA(&rmsg), data_len) ;

	//get public key digest 
	NDOStreamMsg omsg_digest(PGMAXID_LOGIN, MSG_LOGIN_PKI_DIGEST_REQ) ;
	if(SendMsg(omsg_digest,ESF_URGENCY) <= 0) {
		return -1 ;
	}
	if(-1==WaitMsg(&rmsg, WAITMSG_TIMEOUT)) {
		return -1 ;
	}

	data_len = ND_USERMSG_LEN(&rmsg) - ND_USERMSG_HDRLEN;
	if(ND_USERMSG_MAXID(&rmsg)!=PGMAXID_LOGIN || 
		ND_USERMSG_MINID(&rmsg) != MSG_LOGIN_PKI_DIGEST_ACK||
		data_len!=16) {
			return -1 ;
	}	
	MD5Crypt16((char*)&__rsa_contex.publicKey, sizeof(R_RSA_PUBLIC_KEY ) , __rsa_digest);
	if(0==MD5cmp(__rsa_digest, ND_USERMSG_DATA(&rmsg))) {
		__pki_ok = 1 ;
	}
	//PKI success

	//exchange password
	if(0!=tea_key(&__crypt_key) ){
		nd_logmsg("error on create crypt-key\n") ;
		return -1 ;
	}
	struct {
		char  cryptKeymd5[16] ;
		tea_k k ;
	} symm_key;

	MD5Crypt16((char*)&__crypt_key,sizeof(__crypt_key),symm_key.cryptKeymd5) ;
	memcpy(&(symm_key.k), &__crypt_key, sizeof(__crypt_key)) ;

	int len ;
	//send symm crypt key
	if(0!=nd_RSAPublicEncrypt(ND_USERMSG_DATA(&rmsg) , &len, (char*)&symm_key,sizeof(symm_key),&__rsa_contex )) {
		nd_logmsg("rsa encrypt error data error\n") ;
		return -1 ;
	}
	ND_USERMSG_LEN(&rmsg) = len + ND_USERMSG_HDRLEN;
	ND_USERMSG_MAXID(&rmsg) = PGMAXID_LOGIN ;
	ND_USERMSG_MINID(&rmsg) = MSG_LOGIN_EXCH_CRYPTKEY_REQ ;
	if(SendMsg((pg_msg_base*)&rmsg, ESF_URGENCY) <= 0) {
		return -1 ;
	}

	if(-1==WaitMsg(&rmsg, WAITMSG_TIMEOUT)) {
		return -1 ;
	}
	NDIStreamMsg inpkimsg(&rmsg) ;

	if(inpkimsg.MsgMaxid()!=PGMAXID_LOGIN || 
		inpkimsg.MsgMinid() != MSG_LOGIN_EXCH_CRYPTKEY_ACK) {
			return -1 ;
	}
	NDUINT32 is_ok ;
	if(inpkimsg.Read(is_ok)==-1) {
		return -1 ;
	}
	if(0==is_ok) {
		nd_handle h = GetHandle() ;
		nd_connector_set_crypt(h,(void*)&__crypt_key, sizeof(__crypt_key)) ;
		return 0 ;
	}
	return -1 ;
}
*/

NDIConn * htoConnector(nd_handle h)
{
	return (NDIConn*) (((nd_netui_handle)h)->user_data  );
}


NDIConn* CreateConnectorObj(char *protocol_name) 
{
	NDConnector *pConn = new NDConnector() ;
	if(!pConn) {
		return NULL ;
	}
// 
// 	if(-1==pConn->Create(protocol_name)) {
// 		delete pConn ;
// 		return NULL ;
// 	}
	return pConn ;
}

void DestroyConnectorObj(NDIConn *pconn) 
{
	NDConnector *p = (NDConnector*)pconn ;
	delete p;
}

int InitNet() 
{
	char *config_file = NULL ;
	char *argv[] = {"pgnet"} ;
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


//œ˚œ¢»Îø⁄∫Ø ˝Ω⁄µ„
struct msg_entry_node
{
	int					level ;	//»®œﬁµ»º∂
	char *				script_name ;	//Ω≈±æ√˚◊÷
	nd_usermsg_func		entry ;	//»Îø⁄∫Ø ˝
};

/*÷˜œ˚œ¢Ω·π˚*/
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
    struct sub_msgentry sub_buf[0] ;
};

int pg_translate_message(nd_netui_handle connect_handle, nd_packhdr_t *msg ,nd_handle listen_handle) 
{
	ENTER_FUNC()
	int ret = 0 ;
	int data_len = nd_pack_len(msg);
	struct msgentry_root *root_entry= NULL;
	nd_usermsg_func  func = NULL ;
	nd_usermsghdr_t *usermsg =  (nd_usermsghdr_t *) msg ;

	nd_assert(msg) ;
	nd_assert(connect_handle) ;

	root_entry =(struct msgentry_root *) connect_handle->msg_handle ;

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

		if(func) {
			NDIConn *pc = htoConnector((nd_handle)connect_handle);
			ret = func((nd_handle)pc,(nd_usermsgbuf_t*)usermsg,NULL) ;
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