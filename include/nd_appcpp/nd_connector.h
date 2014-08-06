/* file : nd_connector.h
 * header file of connector class of nd engine 
 *
 * all right reserved by neil duan 
 * 2009-5-8 23:25 
 */

#ifndef _ND_CONNECTOR_H_
#define _ND_CONNECTOR_H_

#include "nd_appcpp/nd_object.h"
#include "nd_appcpp/nd_msgpacket.h"
#include "nd_net/nd_netlib.h"
#include "nd_app/app_msgid.h"

//net connector 
class NDConnector : public NDObject 
{
public :

	int CheckValid();
	int WaitMsg(nd_usermsgbuf_t *msgbuf, ndtime_t wait_time=100);
	int Update(ndtime_t wait_time=100);
	void InstallMsgFunc( nd_usermsg_func, ndmsgid_t maxid, ndmsgid_t minid);
	
	void Destroy();
	int SendMsg(NDSendMsg *pmsg, int flag=ESF_ENCRYPT);
	int SendRawData(void *data , size_t size) ;
	int RecvRawData(void *buf, size_t size, ndtime_t waittm) ;
	int Open(char*host, int port, char *protocol_name=NULL,struct nd_proxy_info *proxy=NULL);
	int Close(int force=0);
	nd_handle GetHandle() {return _h_connector ;}
	NDConnector(int maxmsg_num =MSG_CLASS_NUM, int maxid_start=MAXID_BASE) ;
	virtual~NDConnector() ;

private:
	nd_handle _h_connector ;
	int msg_kinds ;
	int msg_base ;
};

NDConnector * htoConnector(nd_handle h);
#endif
