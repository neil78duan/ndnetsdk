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
//#include "ndcli/nd_api_c.h"
#else
#include "nd_net/byte_order.h"
#include "nd_net/nd_netbyte.h"
#endif
#include "nd_net/nd_netpack.h"
#include "ndapplib/nd_protoStream.h"

// 
// #define NET_STREAM_WITH_FORMAT_MARKER 1		//do not user format marker in net message stream
// #define NET_STREAM_STRUCT_END_MARK 0xff
// enum eNDnetStreamMarker
// {
// 	ENDSTREAM_MARKER_UNDEFINE,
// 	ENDSTREAM_MARKER_INT8,
// 	ENDSTREAM_MARKER_INT16,
// 	ENDSTREAM_MARKER_INT32,
// 	ENDSTREAM_MARKER_INT64,
// 	ENDSTREAM_MARKER_FLOAT,
// 	ENDSTREAM_MARKER_DOUBLE,
// 	ENDSTREAM_MARKER_TEXT,
// 	ENDSTREAM_MARKER_BIN,
// 	ENDSTREAM_MARKER_IP,
// 	ENDSTREAM_MARKER_IP6,
// 	ENDSTREAM_CMD_SKIP_STRUCT_MARK,
// 	ENDSTREAM_CMD_ENABLE_STRUCT_MARK,
// 
// 
// };
class  ND_COMMON_CLASS  NDSendMsg : public NDProtoWriteStream
{
public:
	NDSendMsg() ;
	NDSendMsg(int maxid, int minid);
	NDSendMsg(NDUINT16 msgID);
	virtual ~NDSendMsg() ;

	void Reset();
	void Init(int maxid, int minid);
	void Init(NDUINT16 msgID);
	void SetID(int maxid, int minid);

	inline ndmsgid_t &MsgMinid() {return _packet->msg_hdr.minid ; }
	inline ndmsgid_t &MsgMaxid() {return _packet->msg_hdr.maxid ; }
	inline NDUINT16 &MsgLength() {return _packet->msg_hdr.packet_hdr.length;}
	inline NDUINT16 Capacity() {return sizeof(_packet->data) ;}
	inline NDUINT16 GetId() { return ND_MAKE_WORD(_packet->msg_hdr.maxid, _packet->msg_hdr.minid); }
	inline char *MsgData() {return _packet->data ;}
	inline nd_usermsgbuf_t *GetMsgAddr() {return _packet ;}
	size_t GetSerialBin(void *buf, size_t bufsize) ;	//get all message data as stream-data	
	size_t GetDataLen();

    int WriteIp(ndip_t& ) ;
    
	int ToFile(const char *file)const;
	int FromFile(const char *file);

protected:
	bool create();
	virtual void onWriteStream(size_t size);
	nd_usermsgbuf_t  *_packet ;
};
//typedef NDSendMsg NDOStreamMsg;

class  ND_COMMON_CLASS NDOStreamMsg :public  NDSendMsg
{
public:
	NDOStreamMsg() :NDSendMsg() {}
	NDOStreamMsg(int maxid, int minid) :NDSendMsg(maxid, minid) {}
	NDOStreamMsg(NDUINT16 msgID):NDSendMsg(msgID) {}
};
// 
// class NDIStreamMsg;
// class  ND_COMMON_CLASS  NDOStreamMsg :public NDSendMsg
// {
// public :
// 	void Reset();
// 	NDOStreamMsg() ;
// 	NDOStreamMsg(int maxid, int minid) ;
// 	
// 	NDOStreamMsg(NDUINT16 msgID) ;
// 	void Init(int maxid, int minid) ;
// 	void Init(NDUINT16 msgID);	
// 	virtual ~NDOStreamMsg() ;
// 
// 	int WriteForce(NDUINT32);
// 	int WriteForce(NDUINT16);
// 	int Write(NDUINT32 ) ;
// 	int Write(NDUINT16 ) ;
// 	int Write(NDUINT64 ) ;
// 	int Write(NDUINT8 ) ;
// 	int Write(float) ;
// 	int Write(double);
// 	int SetStructEnd() ;	
// 
// 	int Write(int);
// 	int Write(short);
// 	int Write(const NDUINT8 *text);
// 	int Write(const char *text);
// 	int WriteBin(void *data, size_t size) ;	
// 	int WriteStream(char *stream_buf, size_t dataLen);
// 	
// 	int WriteIp(ndip_t& ) ;
// 
// 	void SkipStructEndMark();
// 	void EnableStructEndMark();
// 	void SetID(int maxid, int minid) ;
// 	friend class NDIStreamMsg ;
// 	void *GetWriteAddr();// {return (void*)_op_addr; }
// 	size_t GetFreeLen() ;
// 
// 	int ToFile(const char *file)const;
// 	int FromFile(const char *file);
// 
// protected:
// 
// 	int _WriteOrg(NDUINT32);
// 	int _WriteOrg(NDUINT16);
// 	int _WriteOrg(NDUINT64);
// 	int _WriteOrg(NDUINT8);
// 	int _WriteOrg(float);
// 	int _WriteOrg(double);
// 	int _writeMarker(eNDnetStreamMarker marker, size_t sizebytes);
// private:
// 	char *_op_addr ;
// 	char *_end ;
// };

class  ND_COMMON_CLASS  NDRecvMsg : public NDProtoReadStream
{
public:

	NDRecvMsg(nd_usermsgbuf_t *pmsg);
	virtual ~NDRecvMsg();

	void Init(nd_usermsgbuf_t *pmsg);
	int ToFile(const char *file)const;

	inline ndmsgid_t &MsgMinid() {return recv_packet->msg_hdr.minid ; }
	inline ndmsgid_t &MsgMaxid() {return recv_packet->msg_hdr.maxid ; }
	inline NDUINT16 &MsgLength() {return recv_packet->msg_hdr.packet_hdr.length;}
	inline NDUINT16 GetId() { return ND_MAKE_WORD(recv_packet->msg_hdr.maxid, recv_packet->msg_hdr.minid); }
	inline char *MsgData() {return recv_packet->data ;}
	inline nd_usermsgbuf_t *GetMsgAddr() {return recv_packet ;}
	
    int ReadIp(ndip_t &a) ;
	size_t GetDataLen();
	size_t GetSerialBin(void *buf, size_t bufsize);	
protected:
	NDRecvMsg();
	nd_usermsgbuf_t  *recv_packet ;
};
class  ND_COMMON_CLASS  NDIStreamMsg : public NDRecvMsg
{
public:
	NDIStreamMsg(nd_usermsgbuf_t *pmsg):NDRecvMsg(pmsg) {}
private:
	NDIStreamMsg() {}
};
// 
// class  ND_COMMON_CLASS  NDIStreamMsg : public NDRecvMsg
// {
// public :
// 
// 	int Read(NDUINT32 &a);
// 	int Read(NDUINT16 &a);
// 	int Read(NDUINT8 &a);
// 	int Read(NDUINT64 &a);
// 	int Read(float &a);
// 	int Read(double &a);
// 	void BeginReadStruct(){ m_bStruckEndMarker = false; }
// 	bool CheckStructEnd() { return m_bStruckEndMarker || LeftData() == 0; }
// 	bool TrytoMoveStructEnd();
// 	bool SetSkipMarker(bool bSkip) ;
// 	
// 	int Read(int &a);
// 	int Read(short &a);
// 	bool CheckReachedEnd() { return  LeftData() == 0; }
// 
// 	size_t Read(NDUINT8 *buf, size_t size) ;
// 	size_t Read(char *buf, size_t size) {return	 Read((NDUINT8 *)buf, size) ;}
// 	size_t ReadBin (void *buf, size_t size_buf) ;
// 	int  PeekBinSize(); //return -1 error
// 	eNDnetStreamMarker PeekDataType();
// 	
// 	int Read(NDOStreamMsg &omsg) ;
// 	
// 	int ReadIp(ndip_t &a) ;
// 	
// 	//read all left data to stream_buf
// 	size_t ReadLeftStream(char *stream_buf, size_t buf_size) ;
// 	int LeftData() {return (int)(_end - _op_addr) ;}
// 	void *GetReadAddr() {return (void*)_op_addr;}
// 
// 	NDIStreamMsg(nd_usermsgbuf_t *pmsg) ;
// 	virtual ~NDIStreamMsg() ;
// 
// 	void Init(nd_usermsgbuf_t *pmsg);
// 	int ToFile(const char *file)const;
// 	int dumpText(char *buf, size_t size);
// 
// protected:
// 	int _ReadOrg(NDUINT32 &a);
// 	int _ReadOrg(NDUINT16 &a);
// 	int _ReadOrg(NDUINT8 &a);
// 	int _ReadOrg(NDUINT64 &a);
// 	int _ReadOrg(float &a);
// 	int _ReadOrg(double &a);
// 	int _dumpTobuf(char *buf, size_t size);
// 
// 	int _ReadTypeSize(eNDnetStreamMarker &type, NDUINT8 &size);
// 	bool m_bStruckEndMarker;
// 	bool m_bSkipEndMarker ;
// 	bool m_bSkipEndAllStream;
// 
// private:
// 	NDIStreamMsg() ;
// 	char *_op_addr ;
// 	char *_end ;
// };
#endif
