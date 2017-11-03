/* file nd_msgpack.cpp
 * net message packet implemention
 *
 2009-4-25 11:05
 */

//#include <exception>
//#include "nd_common/nd_common.h"
//#include "nd_net/nd_netlib.h"
//#include "nd_srvcore/nd_srvlib.h"

#include "nd_common/nd_common.h"
#include "ndapplib/nd_msgpacket.h"

static int _write_file(const char *file,const nd_usermsgbuf_t *msgdata)
{
	FILE *pf = fopen(file, "wb");
	if (!pf) {
		return -1;
	}
	size_t s = fwrite(msgdata, 1, msgdata->msg_hdr.packet_hdr.length, pf);
	fclose(pf);
	if (s != msgdata->msg_hdr.packet_hdr.length){
		return -1;
	}
	return 0;
}

static int _read_file(const char *file, nd_usermsgbuf_t *msgdata)
{
	FILE *pf = fopen(file, "rb");
	if (!pf) {
		return -1;
	}
	size_t s= fread(msgdata, 1, sizeof(nd_usermsgbuf_t) , pf);

	fclose(pf);
	if (s != msgdata->msg_hdr.packet_hdr.length) {
		return -1;
	}
	return 0;
}

NDSendMsg::NDSendMsg()
{
	_packet = new nd_usermsgbuf_t() ;
	nd_usermsghdr_init( (nd_usermsghdr_t*)_packet) ;
	_packet->data[0]= 0 ;
}

NDSendMsg::NDSendMsg(int maxid, int minid)  
{
	_packet = new nd_usermsgbuf_t() ;
	
	nd_usermsghdr_init( (nd_usermsghdr_t*)_packet) ;
	_packet->msg_hdr.maxid = (ndmsgid_t)maxid ;
	_packet->msg_hdr.minid = (ndmsgid_t)minid ;
	_packet->data[0]= 0 ;
}

NDSendMsg::~NDSendMsg() 
{
	if(_packet) {
		delete _packet ;
		_packet = 0 ;
	}
}

size_t NDSendMsg::GetSerialBin(void *buf, size_t bufsize)
{
	size_t len =(size_t ) MsgLength() ;
	if (len >bufsize){
		return 0 ;
	}
	memcpy(buf,&_packet,len) ;
	return len ;
}

size_t NDSendMsg::GetDataLen()	{ return ND_USERMSG_DATALEN(_packet); }


//////////////////////////////////////////////////////////////////////////
// class NDOStreamMsg
NDOStreamMsg::NDOStreamMsg() :  NDSendMsg() 
{
	_op_addr = NDSendMsg::MsgData()  ;
	_end = (char*) (_packet + 1) ;
}

NDOStreamMsg::NDOStreamMsg(int maxid, int minid) : NDSendMsg(maxid, minid)
{
	_op_addr = NDSendMsg::MsgData()  ;
	_end = (char*) (_packet + 1) ;
}

NDOStreamMsg::NDOStreamMsg(NDUINT16 msgID) : NDSendMsg(ND_HIBYTE(msgID), ND_LOBYTE(msgID))
{
	_op_addr = NDSendMsg::MsgData()  ;
	_end = (char*) (_packet + 1) ;
}
NDOStreamMsg::~NDOStreamMsg() 
{
}


void NDOStreamMsg::Init(int maxid, int minid)
{
	Reset() ;
	MsgMaxid() = maxid ;
	MsgMinid() = minid ;
}

void NDOStreamMsg::Init(NDUINT16 msgID)
{
	Reset();
	MsgMaxid() = ND_HIBYTE(msgID);
	MsgMinid() = ND_LOBYTE(msgID);
}

int NDOStreamMsg::ToFile(const char *file)const
{
	return _write_file(file, _packet);
}
int NDOStreamMsg::FromFile(const char *file)
{
	Reset();
	return _read_file(file, _packet);
}


//size_t NDOStreamMsg::GetDataLen() 
//{
//	//return _op_addr - (char*)(_packet.data) ;
//	return ND_USERMSG_DATALEN(&_packet) ;
//}
void *NDOStreamMsg::GetWriteAddr()
{
#ifdef NET_STREAM_WITH_FORMAT_MARKER
	return (void*)(_op_addr+1); 
#else 
	return (void*)_op_addr; 
#endif
}
size_t NDOStreamMsg::GetFreeLen()
{
	return _end - _op_addr ;
}


#ifdef NET_STREAM_WITH_FORMAT_MARKER

int NDOStreamMsg::WriteForce(NDUINT32 a)
{
	if (-1 == _writeMarker(ENDSTREAM_MARKER_INT32, 4)) {
		return -1;
	}
	return _WriteOrg(a);
}
int NDOStreamMsg::WriteForce(NDUINT16 a)
{
	if (-1 == _writeMarker(ENDSTREAM_MARKER_INT16, 2)) {
		return -1;
	}
	return _WriteOrg(a);
}


int NDOStreamMsg::Write(NDUINT32 a)
{
	if (a == 0) {
		return _writeMarker(ENDSTREAM_MARKER_INT32, 0);
	}
	else if (a <= 0xff) {
		if (-1 == _writeMarker(ENDSTREAM_MARKER_INT32, 1)) {
			return -1;
		}
		return _WriteOrg((NDUINT8)a);
	}
	else if (a <= 0xffff) {

		if (-1 == _writeMarker(ENDSTREAM_MARKER_INT32, 2)) {
			return -1;
		}
		return _WriteOrg((NDUINT16)a);
	}
	else {

		if (-1 == _writeMarker(ENDSTREAM_MARKER_INT32, 4)) {
			return -1;
		}
		return _WriteOrg(a);
	}
}
int NDOStreamMsg::Write(NDUINT16 a)
{
	if (a == 0) {
		return _writeMarker(ENDSTREAM_MARKER_INT16, 0);
	}
	else if (a <= 0xff) {
		if (-1 == _writeMarker(ENDSTREAM_MARKER_INT16, 1)) {
			return -1;
		}
		return _WriteOrg((NDUINT8)a);
	}
	else  {

		if (-1 == _writeMarker(ENDSTREAM_MARKER_INT16, 2)) {
			return -1;
		}
		return _WriteOrg(a);
	}
}

int NDOStreamMsg::Write(NDUINT64 a)
{
	if (a == 0) {
		return _writeMarker(ENDSTREAM_MARKER_INT64, 0);
	}
	else if (a <= 0xff) {
		if (-1 == _writeMarker(ENDSTREAM_MARKER_INT64, 1)) {
			return -1;
		}
		return _WriteOrg((NDUINT8)a);
	}
	else if (a <= 0xffff) {

		if (-1 == _writeMarker(ENDSTREAM_MARKER_INT64, 2)) {
			return -1;
		}
		return _WriteOrg((NDUINT16)a);
	}
	else if (a <= 0xffffffff) {

		if (-1 == _writeMarker(ENDSTREAM_MARKER_INT64, 4)) {
			return -1;
		}
		return _WriteOrg((NDUINT32)a);
	}
	else  {
		if (-1 == _writeMarker(ENDSTREAM_MARKER_INT64, 8)) {
			return -1;
		}
		return _WriteOrg(a);
	}
}


int NDOStreamMsg::Write(float a)
{
	if (a == 0)	{
		return _writeMarker(ENDSTREAM_MARKER_FLOAT, 0);
	}
	else {
		if (-1 == _writeMarker(ENDSTREAM_MARKER_FLOAT, 4)) {
			return -1;
		}
		return _WriteOrg(a);
	}
	
}

int NDOStreamMsg::Write(double a)
{
	if (a == 0)	{
		return _writeMarker(ENDSTREAM_MARKER_DOUBLE, 0);
	}
	else {
		if (-1 == _writeMarker(ENDSTREAM_MARKER_DOUBLE, 8)) {
			return -1;
		}
		return _WriteOrg(a);
	}

}


int NDOStreamMsg::Write(NDUINT8 a)
{
	if (a == 0)	{
		return _writeMarker(ENDSTREAM_MARKER_INT8, 0);
	}
	else {
		if (-1 == _writeMarker(ENDSTREAM_MARKER_INT8, 1)) {
			return -1;
		}
		return _WriteOrg(a);
	}
}


int NDOStreamMsg::SetStructEnd()
{
	return _WriteOrg((NDUINT8)0xff);
}

#endif


int NDOStreamMsg::WriteIp(ndip_t a)
{
#ifdef NET_STREAM_WITH_FORMAT_MARKER
	if (-1 == _writeMarker(ENDSTREAM_MARKER_IP32, 4)) {
		return -1;
	}
#endif 

	if (_op_addr + sizeof(a) <= _end) {
		memcpy(_op_addr, &a, sizeof(a));
		MsgLength() += sizeof(a);
		_op_addr += sizeof(a);
		return 0;
	}
	return -1;
}

int NDOStreamMsg::WriteIp(ndip_v6_t a)
{

#ifdef NET_STREAM_WITH_FORMAT_MARKER
	if (-1 == _writeMarker(ENDSTREAM_MARKER_IP64, 8)) {
		return -1;
	}
#endif 

	if (_op_addr + sizeof(a) <= _end) {
		memcpy(_op_addr, &a, sizeof(a));
		MsgLength() += sizeof(a);
		_op_addr += sizeof(a);
		return 0;
	}
	return -1;
}



int NDOStreamMsg::Write(const char *text)
{
	return Write((const NDUINT8 *)text);
}
int NDOStreamMsg::Write(const NDUINT8 *text)
{
	size_t n;
	size_t free_size = 0;

	if (_end <= _op_addr){
		return -1;
	}

	if (!text || text[0] == 0) {

#ifdef NET_STREAM_WITH_FORMAT_MARKER
		if (-1 == _writeMarker(ENDSTREAM_MARKER_TEXT, 0)) {
			return -1;
		}
#else
		_WriteOrg((NDUINT16)0);
#endif
		return 0;
	}
	else {

#ifdef NET_STREAM_WITH_FORMAT_MARKER
		if (-1 == _writeMarker(ENDSTREAM_MARKER_TEXT, 1)) {
			return -1;
		}
#endif 
		n = strlen((const char*)text);
		free_size = (_end - _op_addr);
		if (n + 3 <= free_size) {
			_WriteOrg((NDUINT16)n);
			strcpy(_op_addr, (const char*)text);
			_op_addr[n] = 0x7f;
			MsgLength() += (NDUINT16)n + 1;
			_op_addr += (n + 1);
			return 0;
		}
	}
	return -1;
}

int NDOStreamMsg::WriteBin(void *data, size_t size)
{
	if (_end <= _op_addr || size >= Capacity()){
		return -1;
	}

#ifdef NET_STREAM_WITH_FORMAT_MARKER
	if (size ==0){
		if (-1 == _writeMarker(ENDSTREAM_MARKER_BIN, 0)) {
			return -1;
		}
		return 0;
	}
	else {
		if (-1 == _writeMarker(ENDSTREAM_MARKER_BIN, 1)) {
			return -1;
		}
	}
#endif 

	size_t free_size = _end - _op_addr;
	if (size + 2 <= free_size) {
		_WriteOrg((NDUINT16)size);
		if (size > 0)
			memcpy(_op_addr, data, size);
		MsgLength() += (NDUINT16)size;
		_op_addr += size;
		return 0;
	}
	return -1;
}


int NDOStreamMsg::WriteStream(char *stream_buf, size_t dataLen)
{

	if (_end <= _op_addr){
		return -1;
	}
	size_t free_size = _end - _op_addr;
	if (free_size < dataLen) {
		return -1;
	}
	
	if (dataLen > 0) {
		memcpy(_op_addr, stream_buf, dataLen);
		MsgLength() += (NDUINT16)dataLen;
		_op_addr += dataLen;
	}
	return 0;

}

int NDOStreamMsg::_WriteOrg(NDUINT32 a)
{
	if (_op_addr + sizeof(a) <= _end) {
		//*((NDUINT32*)_op_addr) = htonl(a) ;
		nd_long_to_netstream(_op_addr, a);

		MsgLength() += sizeof(a);
		_op_addr += sizeof(a);
		return 0;
	}
	return -1;
}
int NDOStreamMsg::_WriteOrg(NDUINT16 a)
{
	if (_op_addr + sizeof(a) <= _end) {
		//*((NDUINT16*)_op_addr) = htons(a) ;
		nd_short_to_netstream(_op_addr, a);
		MsgLength() += sizeof(a);
		_op_addr += sizeof(a);
		return 0;
	}
	return -1;
}

int NDOStreamMsg::_WriteOrg(NDUINT64 a)
{
	if (_op_addr + sizeof(a) <= _end) {
		nd_longlong_to_netstream(_op_addr, a);
		MsgLength() += sizeof(a);
		_op_addr += sizeof(a);
		return 0;
	}
	return -1;
}


int NDOStreamMsg::_WriteOrg(float a)
{
	union {
		char buf[4];
		float f;
	} val;
	if (_op_addr + sizeof(a) <= _end) {
		val.f = a;
		_op_addr[0] = val.buf[0];
		_op_addr[1] = val.buf[1];
		_op_addr[2] = val.buf[2];
		_op_addr[3] = val.buf[3];
		//*((float*)_op_addr) = a ;
		MsgLength() += sizeof(a);
		_op_addr += sizeof(a);
		return 0;
	}
	return -1;
}

int NDOStreamMsg::_WriteOrg(double a)
{

	union {
		char buf[8];
		double d;
	} val;
	if (_op_addr + sizeof(a) <= _end) {
		val.d = a;
		_op_addr[0] = val.buf[0];
		_op_addr[1] = val.buf[1];
		_op_addr[2] = val.buf[2];
		_op_addr[3] = val.buf[3];
		_op_addr[4] = val.buf[4];
		_op_addr[5] = val.buf[5];
		_op_addr[6] = val.buf[6];
		_op_addr[7] = val.buf[7];
		//*((float*)_op_addr) = a ;
		MsgLength() += sizeof(a);
		_op_addr += sizeof(a);
		return 0;
	}
	return -1;
}


int NDOStreamMsg::_WriteOrg(NDUINT8 a)
{
	if (_op_addr + sizeof(a) <= _end) {
		*((char*)_op_addr) = a;
		MsgLength() += sizeof(a);
		_op_addr += sizeof(a);
		return 0;
	}
	return -1;
}

int NDOStreamMsg::_writeMarker(eNDnetStreamMarker marker, size_t sizebytes)
{
	NDUINT8 m = (NDUINT8)marker << 4 | (NDUINT8)(sizebytes & 0xf);
	return _WriteOrg(m);	
}



void NDOStreamMsg::SetID(int maxid, int minid) 
{
	_packet->msg_hdr.maxid = (ndmsgid_t)maxid ;
	_packet->msg_hdr.minid = (ndmsgid_t)minid ;
}
//////////////////////////////////////////////////////////////////////////

void NDOStreamMsg::Reset()
{
	ndmsgid_t maxid = MsgMaxid();
	ndmsgid_t minid = MsgMinid() ;
	nd_usermsghdr_init( (nd_usermsghdr_t*) GetMsgAddr()) ;
	MsgMaxid() = maxid ;
	MsgMinid() = minid ;
	_op_addr = NDSendMsg::MsgData()  ;
	_end = (char*) (_packet + 1) ;
}
//////////////////////////////////////////////////////////////////////////

size_t NDRecvMsg::GetSerialBin(void *buf, size_t bufsize)
{
	if (!recv_packet){
		return 0;
	}
	size_t len =(size_t ) MsgLength() ;
	if (len >bufsize){
		return 0 ;
	}
	memcpy(buf,recv_packet,len) ;
	return len ;
}

size_t NDRecvMsg::GetDataLen()	{ return ND_USERMSG_DATALEN(recv_packet); }



//////////////////////////////////////////////////////////////////////////
//class NDIStreamMsg

NDIStreamMsg::NDIStreamMsg() : NDRecvMsg(0)
{
#ifdef NET_STREAM_WITH_FORMAT_MARKER
	m_bStruckEndMarker = false;
	m_bSkipEndMarker = false ;
#endif
	_op_addr = NULL ;
	_end = NULL;
}
NDIStreamMsg::NDIStreamMsg(nd_usermsgbuf_t *pmsg) : NDRecvMsg(pmsg)
{
	Init(pmsg) ;
//	if(pmsg) {
//		_op_addr = ND_USERMSG_DATA(pmsg) ;//pmsg->data ;
//		_end = (char*)pmsg ;
//		_end += (size_t) ND_USERMSG_LEN(pmsg) ;
//	}
//	else {
//		_op_addr = NULL ;
//		_end = NULL ;
//	}

}

void NDIStreamMsg::Init(nd_usermsgbuf_t *pmsg)
{
#ifdef NET_STREAM_WITH_FORMAT_MARKER
	m_bStruckEndMarker = false;
	m_bSkipEndMarker = false ;
#endif
	recv_packet = pmsg ;
	if(pmsg) {
		_op_addr = ND_USERMSG_DATA(pmsg) ;//pmsg->data ;
		_end = (char*)pmsg ;
		_end += (size_t) ND_USERMSG_LEN(pmsg) ;
	}
	else {
		_op_addr = NULL ;
		_end = NULL ;
	}
	
}

int NDIStreamMsg::ToFile(const char *file)const
{
	return _write_file(file, recv_packet);
}

NDIStreamMsg::~NDIStreamMsg() 
{

}


#ifdef NET_STREAM_WITH_FORMAT_MARKER


int NDIStreamMsg::Read(NDUINT32 &a)
{
	eNDnetStreamMarker type; 
	NDUINT8 size;
	m_bStruckEndMarker = false;
	if (-1 == _ReadTypeSize(type, size) || m_bStruckEndMarker) {
		return -1;
	}
	if (type != ENDSTREAM_MARKER_INT32)	{
		return -1;
	}

	a = 0;
	if (size == 0)	{
		return 0;
	}
	else if (size == 1) {
		return _ReadOrg((NDUINT8&)a);
	}

	else if (size == 2) {
		return _ReadOrg((NDUINT16&)a);
	}

	else if (size == 4) {
		return _ReadOrg(a);
	}
	else {
		return -1;
	}
}


int NDIStreamMsg::Read(NDUINT16 &a)
{
	eNDnetStreamMarker type;
	NDUINT8 size;
	m_bStruckEndMarker = false;
	if (-1 == _ReadTypeSize(type, size) || m_bStruckEndMarker) {
		return -1;
	}
	if (type != ENDSTREAM_MARKER_INT16)	{
		return -1;
	}

	a = 0;
	if (size == 0)	{
		return 0;
	}
	else if (size == 1) {
		return _ReadOrg((NDUINT8&)a);
	}

	else if (size == 2) {
		return _ReadOrg(a);
	}

	else {
		return -1;
	}
}


int NDIStreamMsg::Read(NDUINT8 &a)
{
	eNDnetStreamMarker type;
	NDUINT8 size;
	m_bStruckEndMarker = false;
	if (-1 == _ReadTypeSize(type, size) || m_bStruckEndMarker) {
		return -1;
	}
	if (type != ENDSTREAM_MARKER_INT8)	{
		return -1;
	}

	a = 0;
	if (size == 0)	{
		return 0;
	}
	else if (size == 1) {
		return _ReadOrg(a);
	}
	else {
		return -1;
	}
}
int NDIStreamMsg::Read(NDUINT64 &a)
{
	eNDnetStreamMarker type;
	NDUINT8 size;
	m_bStruckEndMarker = false;
	if (-1 == _ReadTypeSize(type, size) || m_bStruckEndMarker) {
		return -1;
	}
	if (type != ENDSTREAM_MARKER_INT64)	{
		return -1;
	}

	a = 0;
	if (size == 0)	{
		return 0;
	}
	else if (size == 1) {
		return _ReadOrg((NDUINT8&)a);
	}

	else if (size == 2) {
		return _ReadOrg((NDUINT16&)a);
	}

	else if (size == 4) {
		return _ReadOrg((NDUINT32&)a);
	}

	else if (size == 8) {
		return _ReadOrg(a);
	}
	else {
		return -1;
	}
}
int NDIStreamMsg::Read(float &a)
{
	eNDnetStreamMarker type;
	NDUINT8 size;
	m_bStruckEndMarker = false;
	if (-1 == _ReadTypeSize(type, size) || m_bStruckEndMarker) {
		return -1;
	}
	if (type != ENDSTREAM_MARKER_FLOAT)	{
		return -1;
	}

	a = 0;
	if (size == 0)	{
		return 0;
	}
	else if (size == 4) {
		return _ReadOrg(a);
	}
	else {
		return -1;
	}
}
int NDIStreamMsg::Read(double &a)
{
	eNDnetStreamMarker type;
	NDUINT8 size;
	m_bStruckEndMarker = false;
	if (-1 == _ReadTypeSize(type, size) || m_bStruckEndMarker) {
		return -1;
	}
	if (type != ENDSTREAM_MARKER_DOUBLE)	{
		return -1;
	}

	a = 0;
	if (size == 0)	{
		return 0;
	}
	else if (size == 8) {
		return _ReadOrg(a);
	}
	else {
		return -1;
	}
}

int NDIStreamMsg::_ReadTypeSize(eNDnetStreamMarker &type, NDUINT8 &size)
{
	NDUINT8 marker=0;
	
BEGIN_READ_MARKER:
	if (-1 == _ReadOrg(marker)) {
		return -1;
	}
	if (marker == NET_STREAM_STRUCT_END_MARK) {
		if (m_bSkipEndMarker) {
			goto BEGIN_READ_MARKER ;
		}
		m_bStruckEndMarker = true;
		return 0;
	}
	type = (eNDnetStreamMarker)((marker & 0xf0) >> 4);
	size = marker & 0xf;
	return 0;
}

bool NDIStreamMsg::TrytoMoveStructEnd()
{
	char *orgAddr = _op_addr;
	do 	{
		eNDnetStreamMarker type;
		NDUINT8 size = 0;
		m_bStruckEndMarker = false;
		if (-1 == _ReadTypeSize(type, size)) {
			break;
		}
		if (m_bStruckEndMarker)	{
			return true;
		}
		NDUINT16 data_size=0;


		switch (type)
		{
		case ENDSTREAM_MARKER_INT8:
		case ENDSTREAM_MARKER_INT16:
		case ENDSTREAM_MARKER_INT32:
		case ENDSTREAM_MARKER_INT64:
		case ENDSTREAM_MARKER_FLOAT:
		case ENDSTREAM_MARKER_DOUBLE:
		case ENDSTREAM_MARKER_IP32:
		case ENDSTREAM_MARKER_IP64:
			_op_addr += size;
			break;
		case ENDSTREAM_MARKER_TEXT:
			if (size == 0) {
				break;
			}
			if (-1 == _ReadOrg(data_size)) {
				_op_addr = orgAddr;
				return false;
			}
			_op_addr += data_size +1;
			break;
		case ENDSTREAM_MARKER_BIN:
			if (size == 0) {
				break;
			}

			if (-1 == _ReadOrg(data_size)) {
				_op_addr = orgAddr;
				return false;
			}
			_op_addr += data_size ;
			break;
		default:
			_op_addr = orgAddr;
			return false;
			break;
		}

	} while (LeftData() > 0);
	 
	_op_addr = orgAddr;
	return false;
}


bool NDIStreamMsg::SetSkipMarker(bool bSkip)
{
	bool ret = m_bSkipEndMarker ;
	m_bSkipEndMarker = bSkip ;
	return ret ;
}

#endif 

int NDIStreamMsg::_ReadOrg(NDUINT32 &a)
{
	if (_end >= _op_addr + sizeof(a)) {
		a = nd_netstream_to_long(_op_addr);
		_op_addr += sizeof(a);
		return 0;
	}
	return -1;
}


int NDIStreamMsg::_ReadOrg(NDUINT16 &a)
{
	if (_end >= _op_addr + sizeof(a)) {

		a = nd_netstream_to_short(_op_addr);
		//a =*((NDUINT16*) _op_addr) ;
		//a = ntohs(a) ;
		_op_addr += sizeof(a);
		return 0;

	}
	return -1;
}


int NDIStreamMsg::_ReadOrg(NDUINT8 &a)
{
	if (_end >= _op_addr + sizeof(a)) {

		a = *((char*)_op_addr);
		_op_addr += sizeof(a);
		return 0;

	}
	return -1;
}

int NDIStreamMsg::_ReadOrg(NDUINT64 &a)
{
	if (_end >= _op_addr + sizeof(a)) {


		a = nd_netstream_to_longlong(_op_addr);
		//a =*((NDUINT64*) _op_addr) ;
		//a = nd_ntoh64(a) ;
		_op_addr += sizeof(a);
		return 0;

	}
	return -1;
}

int NDIStreamMsg::_ReadOrg(float &a)
{
	union {
		char buf[4];
		float f;
	} val;
	if (_op_addr + sizeof(a) <= _end) {
		val.buf[0] = _op_addr[0];
		val.buf[1] = _op_addr[1];
		val.buf[2] = _op_addr[2];
		val.buf[3] = _op_addr[3];
		a = val.f;
		_op_addr += sizeof(a);
		return 0;

	}
	return -1;
}

int NDIStreamMsg::_ReadOrg(double &a)
{
	union {
		char buf[8];
		double d;
	} val;
	if (_op_addr + sizeof(a) <= _end) {
		val.buf[0] = _op_addr[0];
		val.buf[1] = _op_addr[1];
		val.buf[2] = _op_addr[2];
		val.buf[3] = _op_addr[3];
		val.buf[4] = _op_addr[4];
		val.buf[5] = _op_addr[5];
		val.buf[6] = _op_addr[6];
		val.buf[7] = _op_addr[7];

		a = val.d;
		_op_addr += sizeof(a);
		return 0;

	}
	return -1;
}


size_t NDIStreamMsg::Read (NDUINT8 *a, size_t size_buf) 
{
#ifdef NET_STREAM_WITH_FORMAT_MARKER
	eNDnetStreamMarker type;
	NDUINT8 size;
	m_bStruckEndMarker = false;
	if (-1 == _ReadTypeSize(type, size) || m_bStruckEndMarker) {
		return 0;
	}
	if (type != ENDSTREAM_MARKER_TEXT)	{
		return 0;
	}
	if (size ==0){
		a[0] = 0;
		return 0;
	}
#endif 

	NDUINT16 data_size ;
	
	if (-1 == _ReadOrg(data_size) || 0 == data_size) {
		return 0 ;
	}

	if(data_size>0 && _op_addr+ data_size <_end) {
		if(_op_addr[data_size]==0x7f && size_buf>data_size) {
			//_op_addr[data_size] = 0 ;
			memcpy(a,_op_addr, data_size ) ;
			_op_addr += data_size + 1 ;	
			a[data_size] = 0 ;
			return (size_t)data_size ;
		}
	}
	return 0;
}


size_t NDIStreamMsg::ReadBin (void *buf, size_t size_buf) 
{

#ifdef NET_STREAM_WITH_FORMAT_MARKER
	eNDnetStreamMarker type;
	NDUINT8 size;
	m_bStruckEndMarker = false;
	if (-1 == _ReadTypeSize(type, size) || m_bStruckEndMarker) {
		return 0;
	}
	if (type != ENDSTREAM_MARKER_BIN)	{
		return 0;
	}
	if (size ==0){
		return 0;
	}
#endif 


	NDUINT16 data_size ;

	if (-1 == _ReadOrg(data_size) || 0 == data_size) {
		return 0 ;
	}

	if(data_size>0 && _op_addr+ data_size <=_end) {
		if(size_buf>=data_size) {
			memcpy(buf,_op_addr, data_size ) ;
			_op_addr += data_size  ;
			return (size_t)data_size ;
		}
	}
	return 0;
}
int NDIStreamMsg::PeekBinSize()
{
	char *curAddr = _op_addr ;
#ifdef NET_STREAM_WITH_FORMAT_MARKER
	NDUINT8 size = *curAddr & 0xf;
	curAddr++;
	if (size == 0) {
		return 0;
	}
#endif 

	if (_end >= curAddr + sizeof(NDUINT16)) {
		 return nd_netstream_to_short(curAddr);
	}
	return -1;
}


eNDnetStreamMarker NDIStreamMsg::PeekDataType()
{
	char *orgAddr = _op_addr;

	eNDnetStreamMarker type;
	NDUINT8 size = 0;
	m_bStruckEndMarker = false;
	if (-1 == _ReadTypeSize(type, size)) {
		_op_addr = orgAddr;
		return ENDSTREAM_MARKER_UNDEFINE;
	}
	_op_addr = orgAddr;
	return type;
		
}

int NDIStreamMsg::Read(NDOStreamMsg &omsg) 
{
	size_t data_size = _end - _op_addr ;
	size_t free_size = omsg._end - omsg._op_addr ;
	if (data_size == 0) {
		return 0 ;
	}
	if (free_size == 0 || free_size < data_size) {
		return -1 ;
	}
	memcpy(omsg._op_addr, _op_addr, data_size) ;
	omsg.MsgLength() += (NDUINT16)data_size ;
	omsg._op_addr += data_size;

	_op_addr = _end ;

	return (int) data_size;

}

int NDIStreamMsg::ReadIp(ndip_t &a)
{

#ifdef NET_STREAM_WITH_FORMAT_MARKER
	eNDnetStreamMarker type;
	NDUINT8 size;
	m_bStruckEndMarker = false;
	if (-1 == _ReadTypeSize(type, size) || m_bStruckEndMarker) {
		return -1;
	}
	if (type != ENDSTREAM_MARKER_IP32)	{
		return -1;
	}
#endif 


	if(_end >= _op_addr + sizeof(a) ) {		
		memcpy(&a, _op_addr, sizeof(a)) ;
		_op_addr += sizeof(a) ;
		return 0 ;
		
	}
	return -1;
}
int NDIStreamMsg::ReadIp(ndip_v6_t &a )
{

#ifdef NET_STREAM_WITH_FORMAT_MARKER
	eNDnetStreamMarker type;
	NDUINT8 size;
	m_bStruckEndMarker = false;
	if (-1 == _ReadTypeSize(type, size) || m_bStruckEndMarker) {
		return -1;
	}
	if (type != ENDSTREAM_MARKER_IP64)	{
		return -1;
	}
#endif 
	if(_end >= _op_addr + sizeof(a) ) {
		
		memcpy(&a, _op_addr, sizeof(a)) ;
		_op_addr += sizeof(a) ;
		return 0 ;
		
	}
	return -1;
}


//read all left data to stream_buf
size_t NDIStreamMsg::ReadLeftStream(char *stream_buf, size_t buf_size)
{
	size_t data_size = _end - _op_addr ;
	if (data_size == 0) {
		return 0 ;
	}
	if (buf_size < data_size) {
		return 0 ;
	}
	memcpy(stream_buf, _op_addr, data_size) ;
	_op_addr = _end ;
	
	return  data_size;
}


