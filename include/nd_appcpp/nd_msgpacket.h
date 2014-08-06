/* file nd_msgpack.h
 * header file of net message packet 
 *
 2009-4-25 11:05
 */

#ifndef _NDMSGPACK_H_
#define _NDMSGPACK_H_

#include "nd_appcpp/nd_object.h"
#include "nd_net/nd_netlib.h"
#include "nd_common/nd_common.h"

class NDSendMsg : public NDObject
{
public:
	NDSendMsg() ;
	NDSendMsg(ndmsgid_t maxid, ndmsgid_t minid)  ;
	virtual ~NDSendMsg() ;
	//inline ndmsgparam_t &MsgParam() {return _packet.msg_hdr.param ; }
	inline ndmsgid_t &MsgMinid() {return _packet.msg_hdr.minid ; }
	inline ndmsgid_t &MsgMaxid() {return _packet.msg_hdr.maxid ; }
	inline NDUINT16 &MsgLength() {return _packet.msg_hdr.packet_hdr.length;}
	inline NDUINT16 Capacity() {return sizeof(_packet.data) ;} 
	//size_t MsgLength() {return (size_t)(msg_hdr.packet_hdr.length) ;}
	//void SetLength(size_t s) {msg_hdr.packet_hdr.length = s ;	}
	inline char *MsgData() {return _packet.data ;}
	inline nd_usermsgbuf_t *GetMsgAddr() {return &_packet ;}
	
protected:
	nd_usermsgbuf_t  _packet ;
};

//输出流式协议包
class NDOStreamMsg :public NDSendMsg
{
public :
	void Reset();
	NDOStreamMsg() ;
	NDOStreamMsg(ndmsgid_t maxid, ndmsgid_t minid) ;
	virtual ~NDOStreamMsg() ;
	int Write(NDUINT32 ) ;
	int Write(NDUINT16 ) ;
	int Write(NDUINT64 ) ;
	int Write(char ) ;
	int Write(char *text ) ;
	int Write(float) ;
	int Write(double ) ;
	int WriteBin(void *data, size_t size) ;
private:
	char *_op_addr ;
	char *_end ;
};


class NDRecvMsg : public NDObject
{
public:
	NDRecvMsg(nd_usermsgbuf_t *pmsg) :recv_packet(pmsg){}
	virtual ~NDRecvMsg() {}
	//inline ndmsgparam_t &MsgParam() {return recv_packet->msg_hdr.param ; }
	inline ndmsgid_t &MsgMinid() {return recv_packet->msg_hdr.minid ; }
	inline ndmsgid_t &MsgMaxid() {return recv_packet->msg_hdr.maxid ; }
	inline NDUINT16 &MsgLength() {return recv_packet->msg_hdr.packet_hdr.length;}
	inline char *MsgData() {return recv_packet->data ;}
	inline nd_usermsgbuf_t *GetMsgAddr() {return recv_packet ;}
	
protected:
	nd_usermsgbuf_t  *recv_packet ;
};
//输入流式协议包
class NDIStreamMsg : public NDRecvMsg
{
public :
	int Read(NDUINT32 &a) ;
	int Read(NDUINT16 &a) ;
	int Read(char &a) ;
	int Read(NDUINT64 &a) ;
	int Read(float &a) ;
	int Read(double &a) ;
	size_t Read(char *buf, size_t size) ;
	size_t ReadBin (void *buf, size_t size_buf) ;

	NDIStreamMsg(nd_usermsgbuf_t *pmsg) ;
	virtual ~NDIStreamMsg() ;

private:
	NDIStreamMsg() ;
	char *_op_addr ;
	char *_end ;
};
#endif
