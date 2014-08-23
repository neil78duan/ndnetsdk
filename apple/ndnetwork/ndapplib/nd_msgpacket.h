/* file nd_msgpack.h
 * header file of net message packet 
 *
 2009-4-25 11:05
 */

#ifndef _NDMSGPACK_H_
#define _NDMSGPACK_H_

#include "nd_net/nd_netlib.h"
#include "nd_common/nd_common.h"
#include "ndapplib/nd_object.h"

class NDSendMsg 
{
public:
	NDSendMsg() ;
	NDSendMsg(int maxid, int minid)  ;
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
	size_t GetSerialBin(void *buf, size_t bufsize) ;	//把消息输出成二进制
	
protected:
	nd_usermsgbuf_t  _packet ;
};

class NDIStreamMsg;
//输出流式协议包
class NDOStreamMsg :public NDSendMsg
{
public :
	void Reset();
	NDOStreamMsg() ;
	NDOStreamMsg(int maxid, int minid) ;
	virtual ~NDOStreamMsg() ;
	int Write(NDUINT32 ) ;
	int Write(NDUINT16 ) ;
	int Write(NDUINT64 ) ;
	int Write(NDUINT8 ) ;
	int Write(NDUINT8 *text ) ;
	int Write(float) ;
	int Write(double ) ;
	int WriteBin(void *data, size_t size) ;

//    
//	int WriteByte(int ) ;
//	int WriteInt(int ) ;
//	int WriteShort(int ) ;
//	int WriteText(char* ) ;
	void SetID(int maxid, int minid) ;
	friend class NDIStreamMsg ;
	void *GetWriteAddr() {return (void*)_op_addr;}
	size_t GetDataLen() ;
	size_t GetFreeLen() ;
private:
	char *_op_addr ;
	char *_end ;
};

class NDRecvMsg 
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

	size_t GetSerialBin(void *buf, size_t bufsize) ;	//把消息输出成二进制
protected:
	nd_usermsgbuf_t  *recv_packet ;
};
//输入流式协议包
class NDIStreamMsg : public NDRecvMsg
{
public :
	int Read(NDUINT32 &a) ;
	int Read(NDUINT16 &a) ;
	int Read(NDUINT8 &a) ;
	int Read(NDUINT64 &a) ;
	int Read(float &a) ;
	int Read(double &a) ;
	size_t Read(NDUINT8 *buf, size_t size) ;
	size_t ReadBin (void *buf, size_t size_buf) ;
	int Read(NDOStreamMsg &omsg) ;

//	int ReadByte() ;
//	int ReadShort() ;
//	int ReadInt() ;
//	char* ReadText() ;
	//int ReadStream(NDOStreamMsg &omsg) ;
	int LeftData() {return (int)(_end - _op_addr) ;}
	void *GetReadAddr() {return (void*)_op_addr;}

	NDIStreamMsg(nd_usermsgbuf_t *pmsg) ;
	virtual ~NDIStreamMsg() ;

private:
	NDIStreamMsg() ;
	char *_op_addr ;
	char *_end ;
};
#endif
