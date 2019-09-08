/* file nd_protoStream.h
 * 
 * define structure data to stream
 *
 * splite from nd_msgpack.h
 *
 * create by duan 
 *init create
 * 2009-4-25 11:05
 * last modified 2019.9.2
 */

#ifndef _ND_PROTO_STREAM_H_
#define _ND_PROTO_STREAM_H_

#include "nd_common/nd_common.h"
#include "ndapplib/nd_vartype.h"

#define NET_STREAM_WITH_FORMAT_MARKER 1		//do not user format marker in net message stream
#define NET_STREAM_STRUCT_END_MARK 0xff
enum eNDnetStreamMarker
{
	ENDSTREAM_MARKER_UNDEFINE,
	ENDSTREAM_MARKER_INT8,
	ENDSTREAM_MARKER_INT16,
	ENDSTREAM_MARKER_INT32,
	ENDSTREAM_MARKER_INT64,
	ENDSTREAM_MARKER_FLOAT,
	ENDSTREAM_MARKER_DOUBLE,
	ENDSTREAM_MARKER_TEXT,
	ENDSTREAM_MARKER_BIN,
	ENDSTREAM_MARKER_IP,
	ENDSTREAM_MARKER_IP6,
	ENDSTREAM_CMD_SKIP_STRUCT_MARK,
	ENDSTREAM_CMD_ENABLE_STRUCT_MARK,
};


class  ND_COMMON_CLASS  NDProtoWriteStream
{
public:
	NDProtoWriteStream();
	NDProtoWriteStream(char *startAddr,char *endAddr);
	NDProtoWriteStream(char *startAddr, size_t size);
	virtual ~NDProtoWriteStream();
	void Init(char *startAddr, char *endAddr);
	void Reset();
	int WriteForce(NDUINT32);
	int WriteForce(NDUINT16);
	int Write(NDUINT32);
	int Write(NDUINT16);
	int Write(NDUINT64);
	int Write(NDUINT8);
	int Write(float);
	int Write(double);
	int SetStructEnd();

	int Write(int);
	int Write(short);
	int Write(const NDUINT8 *text);
	int Write(const char *text);
	int WriteBin(void *data, size_t size);
	int WriteStream(char *stream_buf, size_t dataLen);

	//int WriteIp(ndip_t&);
	int WriteVar(const NDVarType &value);

	void SkipStructEndMark();
	void EnableStructEndMark();
	void *GetWriteAddr();
	size_t GetFreeLen();
	size_t GetStreamData();
	friend class NDProtoReadStream;
protected:

	int _WriteOrg(NDUINT32);
	int _WriteOrg(NDUINT16);
	int _WriteOrg(NDUINT64);
	int _WriteOrg(NDUINT8);
	int _WriteOrg(float);
	int _WriteOrg(double);
	int _writeMarker(eNDnetStreamMarker marker, size_t sizebytes);
	virtual void onWriteStream(size_t size);
private:
	char *_org_start_addr;
	char *_op_addr;
	char *_end;
};
///////////////////////////////////////////////////
//class read stream

class  ND_COMMON_CLASS  NDProtoReadStream 
{
public:

	NDProtoReadStream();
	NDProtoReadStream(char *dataStart, char *dataEnd);
	NDProtoReadStream(char *dataStart, size_t size);
	virtual ~NDProtoReadStream();
	void Init(char *dataStart, char *dataEnd);
	void Reset();

	int Read(NDUINT32 &a);
	int Read(NDUINT16 &a);
	int Read(NDUINT8 &a);
	int Read(NDUINT64 &a);
	int Read(float &a);
	int Read(double &a);
	int Read(int &a);
	int Read(short &a);
	int Read(NDProtoWriteStream &omsg);		//read all left data to omsg
	//int ReadIp(ndip_t &a);
	int ReadVar(NDVarType &var);
	size_t Read(NDUINT8 *buf, size_t size);
	size_t Read(char *buf, size_t size) { return	 Read((NDUINT8 *)buf, size); }
	size_t ReadBin(void *buf, size_t size_buf);
    size_t ReadStream(void *streamBuff, size_t readlen) ;

	int  PeekBinSize(); //return -1 error
	eNDnetStreamMarker PeekDataType();

	void BeginReadStruct() { m_bStruckEndMarker = false; }
	bool CheckStructEnd() { return m_bStruckEndMarker || LeftData() == 0; }
	bool TrytoMoveStructEnd();
	bool SetSkipMarker(bool bSkip);
	bool CheckReachedEnd() { return  LeftData() == 0; }

	//read all left data to stream_buf
	size_t ReadLeftStream(char *stream_buf, size_t buf_size);
	int LeftData() { return (int)(_end - _op_addr); }
	void *GetReadAddr() { return (void*)_op_addr; }
	int dumpText(char *buf, size_t size);

protected:
	int _ReadOrg(NDUINT32 &a);
	int _ReadOrg(NDUINT16 &a);
	int _ReadOrg(NDUINT8 &a);
	int _ReadOrg(NDUINT64 &a);
	int _ReadOrg(float &a);
	int _ReadOrg(double &a);
	int _dumpTobuf(char *buf, size_t size);

	int _ReadTypeSize(eNDnetStreamMarker &type, NDUINT8 &size);
	bool m_bStruckEndMarker;
	bool m_bSkipEndMarker;
	bool m_bSkipEndAllStream;

	char *_org_start_addr;
	char *_op_addr;
	char *_end;

};

#endif
