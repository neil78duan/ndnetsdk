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

NDSendMsg::NDSendMsg() 
{
	nd_usermsghdr_init( (nd_usermsghdr_t*)&_packet.msg_hdr) ;
	_packet.data[0]= 0 ;
}

NDSendMsg::NDSendMsg(int maxid, int minid)  
{
	nd_usermsghdr_init( (nd_usermsghdr_t*)&_packet.msg_hdr) ;
	_packet.msg_hdr.maxid = (ndmsgid_t)maxid ;
	_packet.msg_hdr.minid = (ndmsgid_t)minid ;
	_packet.data[0]= 0 ;
}

NDSendMsg::~NDSendMsg() 
{
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

size_t NDSendMsg::GetDataLen()	{ return ND_USERMSG_DATALEN(&_packet); }

//////////////////////////////////////////////////////////////////////////
// class NDOStreamMsg
NDOStreamMsg::NDOStreamMsg() :  NDSendMsg() 
{
	_op_addr = NDSendMsg::MsgData()  ;
	_end = (char*) ((&_packet) + 1) ;
}

NDOStreamMsg::NDOStreamMsg(int maxid, int minid) : NDSendMsg(maxid, minid)
{
	_op_addr = NDSendMsg::MsgData()  ;
	_end = (char*) ((&_packet) + 1) ;
}

NDOStreamMsg::NDOStreamMsg(NDUINT16 msgID) : NDSendMsg(ND_HIBYTE(msgID), ND_LOBYTE(msgID))
{
	_op_addr = NDSendMsg::MsgData()  ;
	_end = (char*) ((&_packet) + 1) ;
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

//size_t NDOStreamMsg::GetDataLen() 
//{
//	//return _op_addr - (char*)(_packet.data) ;
//	return ND_USERMSG_DATALEN(&_packet) ;
//}
size_t NDOStreamMsg::GetFreeLen()
{
	return _end - _op_addr ;
}

int NDOStreamMsg::Write(NDUINT32 a) 
{
	if(_op_addr + sizeof(a) <= _end) {
		//*((NDUINT32*)_op_addr) = htonl(a) ;
		nd_long_to_netstream(_op_addr, a) ;
		
		MsgLength() += sizeof(a) ;
		_op_addr+= sizeof(a) ;
		return 0 ;
	}
	return -1 ;
}
int NDOStreamMsg::Write(NDUINT16 a) 
{
	if(_op_addr + sizeof(a) <= _end) {
		//*((NDUINT16*)_op_addr) = htons(a) ;
		nd_short_to_netstream(_op_addr, a) ;
		MsgLength() += sizeof(a) ;
		_op_addr+= sizeof(a) ;
		return 0 ;
	}
	return -1 ;
}

int NDOStreamMsg::Write(NDUINT64 a) 
{
	if(_op_addr + sizeof(a) <= _end) {
		//*((NDUINT64*)_op_addr) = nd_hton64(a) ; //why  htonll is undefine?
		nd_longlong_to_netstream(_op_addr, a) ;
		MsgLength() += sizeof(a) ;
		_op_addr+= sizeof(a) ;
		return 0 ;
	}
	return -1 ;
}


int NDOStreamMsg::Write(float a)
{
    union {
        char buf[4] ;
        float f;
    } val ;
    if(_op_addr + sizeof(a) <= _end) {
        val.f = a;
        _op_addr[0] = val.buf[0] ;
        _op_addr[1] = val.buf[1] ;
        _op_addr[2] = val.buf[2] ;
        _op_addr[3] = val.buf[3] ;
        //*((float*)_op_addr) = a ;
        MsgLength() += sizeof(a) ;
        _op_addr+= sizeof(a) ;
        return 0 ;
    }
    return -1 ;
}

int NDOStreamMsg::Write(double a)
{
    
    union {
        char buf[8] ;
        double d;
    } val ;
    if(_op_addr + sizeof(a) <= _end) {
        val.d = a;
        _op_addr[0] = val.buf[0] ;
        _op_addr[1] = val.buf[1] ;
        _op_addr[2] = val.buf[2] ;
        _op_addr[3] = val.buf[3] ;
        _op_addr[4] = val.buf[4] ;
        _op_addr[5] = val.buf[5] ;
        _op_addr[6] = val.buf[6] ;
        _op_addr[7] = val.buf[7] ;
        //*((float*)_op_addr) = a ;
        MsgLength() += sizeof(a) ;
        _op_addr+= sizeof(a) ;
        return 0 ;
    }
    return -1 ;
}


int NDOStreamMsg::Write(NDUINT8 a) 
{
	if(_op_addr + sizeof(a) <= _end) {
		*((char*)_op_addr) = a ;
		MsgLength() += sizeof(a) ;
		_op_addr+= sizeof(a) ;
		return 0 ;
	}
	return -1 ;
}

int NDOStreamMsg::Write(const char *text)
{
	return Write((const NDUINT8 *)text);
}
int NDOStreamMsg::Write(const NDUINT8 *text)
{
	size_t n ;
	size_t free_size = 0 ;
	
	if (_end <= _op_addr ){
		return -1 ;
	}

	if(!text || text[0]==0) {
		Write((NDUINT16)0) ;
		return 0;
	}
	else {
		n = strlen((const char*)text) ;
		free_size = (_end - _op_addr );
		if(n + 3 <= free_size) {
			Write((NDUINT16)n) ;
			strcpy(_op_addr, (const char*)text) ;
			_op_addr[n] = 0x7f ;
			MsgLength() += (NDUINT16)n+1 ;
			_op_addr += (n+1) ;
			return 0 ;
		}
	}
	return -1;
}

int NDOStreamMsg::WriteBin(void *data, size_t size) 
{
	if (_end <= _op_addr ){
		return -1 ;
	}
	size_t free_size = _end - _op_addr ;
	if(size + 2 <= free_size) {
		Write((NDUINT16)size) ;
		if (size > 0)
			memcpy(_op_addr, data, size) ;
		MsgLength() += (NDUINT16) size ;
		_op_addr += size ;
		return 0 ;
	}
	return -1;
}


int NDOStreamMsg::WriteStream(char *stream_buf, size_t dataLen)
{
	
	if (_end <= _op_addr ){
		return -1 ;
	}
	size_t free_size = _end - _op_addr ;
	if (free_size < dataLen) {
		return -1;
	}
	
	if(dataLen>0) {
		memcpy(_op_addr, stream_buf, dataLen) ;
		MsgLength() += (NDUINT16) dataLen ;
		_op_addr += dataLen ;
	}
	return 0;
	
}
//
////在脚本中使用
//int NDOStreamMsg::WriteByte(int a) 
//{
//	return Write((NDUINT8)a) ;
//}
//int NDOStreamMsg::WriteInt(int a) 
//{
//	return Write((NDUINT32)a) ;
//}
//int NDOStreamMsg::WriteShort(int a) 
//{
//	return Write((NDUINT16)a) ;
//}
//
//int NDOStreamMsg::WriteText(char* a) 
//{
//	return Write((NDUINT8*)a) ;
//}

void NDOStreamMsg::SetID(int maxid, int minid) 
{
	_packet.msg_hdr.maxid = (ndmsgid_t)maxid ;
	_packet.msg_hdr.minid = (ndmsgid_t)minid ;
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
	_end = (char*) ((&_packet) + 1) ;
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

NDIStreamMsg::~NDIStreamMsg() 
{

}

int NDIStreamMsg::Read (NDUINT32 &a) 
{
	if(_end >= _op_addr + sizeof(a) ) {

		a = nd_netstream_to_long(_op_addr);
		//a =*((NDUINT32*) _op_addr) ;
        //a = ntohl(a) ;
		_op_addr += sizeof(a) ;
		return 0 ;

	}
	return -1;
}


int NDIStreamMsg::Read (NDUINT16 &a) 
{
	if(_end >= _op_addr + sizeof(a) ) {

		a = nd_netstream_to_short(_op_addr);
		//a =*((NDUINT16*) _op_addr) ;
        //a = ntohs(a) ;
		_op_addr += sizeof(a) ;
		return 0 ;

	}
	return -1;
}

int NDIStreamMsg::Read (NDUINT8 &a) 
{
	if(_end >= _op_addr + sizeof(a) ) {

		a =*((char*) _op_addr) ;
		_op_addr += sizeof(a) ;
		return 0 ;

	}
	return -1;
}
int NDIStreamMsg::Read (NDUINT64 &a) 
{
	if(_end >= _op_addr + sizeof(a) ) {

		
		a = nd_netstream_to_longlong(_op_addr);
		//a =*((NDUINT64*) _op_addr) ;
        //a = nd_ntoh64(a) ;
		_op_addr += sizeof(a) ;
		return 0 ;

	}
	return -1;
}
int NDIStreamMsg::Read (float &a) 
{
    union {
        char buf[4] ;
        float f;
    } val ;
    if(_op_addr + sizeof(a) <= _end) {
        val.buf[0] = _op_addr[0]  ;
        val.buf[1] =_op_addr[1]  ;
        val.buf[2] =_op_addr[2]  ;
        val.buf[3] =_op_addr[3]  ;
        a = val.f;
        _op_addr += sizeof(a) ;
        return 0 ;
        
    }
    return -1;
}
int NDIStreamMsg::Read (double &a) 
{
    union {
        char buf[8] ;
        double d;
    } val ;
    if(_op_addr + sizeof(a) <= _end) {
        val.buf[0] = _op_addr[0]  ;
        val.buf[1] =_op_addr[1]  ;
        val.buf[2] =_op_addr[2]  ;
        val.buf[3] =_op_addr[3]  ;
        val.buf[4] =_op_addr[4]  ;
        val.buf[5] =_op_addr[5]  ;
        val.buf[6] =_op_addr[6]  ;
        val.buf[7] =_op_addr[7]  ;
        
        a = val.d;
		_op_addr += sizeof(a) ;
		return 0 ;

	}
	return -1;
}

size_t NDIStreamMsg::Read (NDUINT8 *a, size_t size_buf) 
{
	NDUINT16 data_size ;
	
	if(-1==Read(data_size)|| 0==data_size) {
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
	NDUINT16 data_size ;

	if(-1==Read(data_size)|| 0==data_size) {
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

//
//int NDIStreamMsg::ReadByte() 
//{
//	NDUINT8 a ;
//	if(Read(a)!=-1) {
//		return a ;
//	}
//	return 0 ;
//}
//int NDIStreamMsg::ReadShort() 
//{
//	NDUINT16 a ;
//	if(Read(a)!=-1) {
//		return a ;
//	}
//	return 0 ;
//}
//int NDIStreamMsg::ReadInt() 
//{
//	NDUINT32 a ;
//	if(Read(a)!=-1) {
//		return a ;
//	}
//	return 0 ;
//}
//char* NDIStreamMsg::ReadText() 
//{
//	NDUINT8* p ;
//
//	NDUINT16 data_size ;
//	if(-1==Read(data_size)|| 0==data_size) {
//		return NULL ;
//	}
//
//
//	if(data_size>0 && _op_addr+ data_size <_end) {
//		if(_op_addr[data_size]==0x7f ) {
//			//_op_addr[data_size] = 0 ;
//			//memcpy(a,_op_addr, data_size ) ;
//			p = (NDUINT8*)_op_addr ;
//			_op_addr += data_size + 1 ;	
//			p[data_size] = 0 ;
//			return (char*)p ;
//		}
//	}
//	return 0;
//
//}
//
//int NDIStreamMsg::ReadStream(NDOStreamMsg &omsg) 
//{
//	return Read(omsg) ;
//}