/* file nd_msgpack.h
 * header file of net message packet 
 *
 2009-4-25 11:05
 */

#ifndef _NDMSGPACK_H_
#define _NDMSGPACK_H_

#ifndef BUILD_AS_THIRD_PARTY
#include "nd_net/nd_netlib.h"
#include "nd_common/nd_common.h"
#include "ndcli/nd_api_c.h"
#endif
#include "nd_common/nd_export_def.h"

class ND_CONNCLI_CLASS NDSendMsg
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
	size_t GetSerialBin(void *buf, size_t bufsize) ;	//∞—œ˚œ¢ ‰≥ˆ≥…∂˛Ω¯÷∆
	
	size_t GetDataLen();
protected:
	nd_usermsgbuf_t  _packet ;
};

class NDIStreamMsg;
// ‰≥ˆ¡˜ Ω–≠“È∞¸
class ND_CONNCLI_CLASS NDOStreamMsg :public NDSendMsg
{
public :
	void Reset();
	NDOStreamMsg() ;
	NDOStreamMsg(int maxid, int minid) ;
	
	NDOStreamMsg(NDUINT16 msgID) ;
	void Init(int maxid, int minid) ;
	
	virtual ~NDOStreamMsg() ;
	int Write(NDUINT32 ) ;
	int Write(NDUINT16 ) ;
	int Write(NDUINT64 ) ;
	int Write(NDUINT8 ) ;
	int Write(const NDUINT8 *text ) ;
	int Write(const char *text);
	int Write(float) ;
	int Write(double ) ;
	int WriteBin(void *data, size_t size) ;
	
	int WriteStream(char *stream_buf, size_t dataLen);

//    
//	int WriteByte(int ) ;
//	int WriteInt(int ) ;
//	int WriteShort(int ) ;
//	int WriteText(char* ) ;
	void SetID(int maxid, int minid) ;
	friend class NDIStreamMsg ;
	void *GetWriteAddr() {return (void*)_op_addr;}
	//size_t GetDataLen() ;
	size_t GetFreeLen() ;
private:
	char *_op_addr ;
	char *_end ;
};

class ND_CONNCLI_CLASS NDRecvMsg
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
	
	size_t GetDataLen();
	size_t GetSerialBin(void *buf, size_t bufsize) ;	//∞—œ˚œ¢ ‰≥ˆ≥…∂˛Ω¯÷∆
protected:
	nd_usermsgbuf_t  *recv_packet ;
};
// ‰»Î¡˜ Ω–≠“È∞¸
class ND_CONNCLI_CLASS NDIStreamMsg : public NDRecvMsg
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
	
	//read all left data to stream_buf
	size_t ReadLeftStream(char *stream_buf, size_t buf_size) ;

//	int ReadByte() ;
//	int ReadShort() ;
//	int ReadInt() ;
//	char* ReadText() ;
	//int ReadStream(NDOStreamMsg &omsg) ;
	int LeftData() {return (int)(_end - _op_addr) ;}
	void *GetReadAddr() {return (void*)_op_addr;}

	NDIStreamMsg(nd_usermsgbuf_t *pmsg) ;
	virtual ~NDIStreamMsg() ;

	void Init(nd_usermsgbuf_t *pmsg) ;
private:
	NDIStreamMsg() ;
	char *_op_addr ;
	char *_end ;
};
#endif
