/* file : nd_connector.h
 * header file of connector class of nd engine 
 *
 * all right reserved by neil duan 
 * 2009-5-8 23:25 
 */

#ifndef _ND_CONNECTOR_H_
#define _ND_CONNECTOR_H_

#include "ndapplib/nd_baseConn.h"
#include "nd_msg.h"

#define CONNECT_INSTALL_MSG(connect, msgFunc, maxID, minID) \
	(connect)->InstallMsgFunc((nd_conn_msg_entry)msgFunc, maxID, minID, #maxID"-"#minID)

#define CONNECT_INSTALL_MSG_INT16(connect, msgFunc, msgID) \
	(connect)->InstallMsgFunc((nd_conn_msg_entry)msgFunc, ND_HIBYTE(msgID),ND_LOBYTE(msgID), #msgID)


//net connector 
class  ND_COMMON_CLASS NDConnector : public NDBaseConnector
{
public :		
	void Destroy(int flag = 0);
	int Create(const char *protocol_name=NULL) ;
	int Open(const char*host, int port,const char *protocol_name, struct nd_proxy_info *proxy=NULL);
	int Close(int force=0);

	int Update(ndtime_t wait_time);
	void InstallMsgFunc( nd_conn_msg_entry, ndmsgid_t maxid, ndmsgid_t minid,const char *msgname=NULL);
	void SetDftMsgHandler(nd_conn_msg_entry func);

	NDConnector(int maxmsg_num = ND_MAIN_MSG_CAPACITY, int maxid_start = ND_MSG_BASE_ID);
	void SetMsgNum(int maxmsg_num , int maxid_start=0) ;
	virtual~NDConnector() ;

protected:
	int m_open ;
	int msg_kinds ;
	int msg_base ;
};

// safe connect , the data will be stored when send error 
class  ND_COMMON_CLASS NDSafeConnector: public NDConnector
{
public:
	NDSafeConnector(int maxmsg_num = ND_MAIN_MSG_CAPACITY, int maxid_start = ND_MSG_BASE_ID);
	virtual ~NDSafeConnector ();


	int SendSafe(NDSendMsg &msg, int flag = ESF_URGENCY);
	int SendSafe(nd_usermsghdr_t *msghdr, int flag = ESF_URGENCY);

	int TrytoResennd();
private:
	struct resend_data {
		struct list_head list;
		int flag;
		nd_usermsghdr_t msg_hdr;
		char data[1];
	};

	struct list_head m_undeliveried_list;
};


#endif
