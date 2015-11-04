//
//  nd_netbyte.h
//  ndMacStatic
//
//  Created by duanxiuyun on 15-7-16.
//  Copyright (c) 2015 duanxiuyun. All rights reserved.
//

#ifndef _ND_NETBYTE_H_
#define _ND_NETBYTE_H_

#include "nd_net/byte_order.h"
#include "nd_common/nd_define.h"

#if (ND_BYTE_ORDER==ND_L_ENDIAN)

static __inline void nd_short_to_netstream(char *buf, NDUINT16 val)
{
	buf[0] = (val>>8) & 0xff ;
	buf[1] = val & 0xff ;
}

static __inline void nd_long_to_netstream(char *buf, NDUINT32 val)
{
	buf[0] = (val>>24) & 0xff ;
	buf[1] = (val>>16) & 0xff ;
	buf[2] = (val>>8) & 0xff ;
	buf[3] = val & 0xff ;
}

static __inline void nd_longlong_to_netstream(char *buf, NDUINT64 val)
{
	
	buf[0] = (val>>56) & 0xff ;
	buf[1] = (val>>48) & 0xff ;
	buf[2] = (val>>40) & 0xff ;
	buf[3] = (val>>32) & 0xff ;
	
	buf[4] = (val>>24) & 0xff ;
	buf[5] = (val>>16) & 0xff ;
	buf[6] = (val>>8) & 0xff ;
	buf[7] = val & 0xff ;
}
//////////

static __inline NDUINT16 nd_netstream_to_short(const char *buf)
{
	NDUINT16 val = buf[0] ;
	return (NDUINT16) ( val<< 8 | (buf[1]&0xff) ) ;
}

static __inline NDUINT32 nd_netstream_to_long(const char *buf)
{
	union {
		NDUINT32 val ;
		char _data[4] ;
	}v1 ;
	
	v1._data[3] = buf[0] ;
	v1._data[2] = buf[1] ;
	v1._data[1] = buf[2] ;
	v1._data[0] = buf[3] ;
	return v1.val ;
}

static __inline NDUINT64 nd_netstream_to_longlong(const char *buf)
{
	union {
		NDUINT64 val ;
		char _data[8] ;
	}v1 ;
	
	v1._data[7] = buf[0] ;
	v1._data[6] = buf[1] ;
	v1._data[5] = buf[2] ;
	v1._data[4] = buf[3] ;
	
	v1._data[3] = buf[4] ;
	v1._data[2] = buf[5] ;
	v1._data[1] = buf[6] ;
	v1._data[0] = buf[7] ;
	
	return v1.val ;
}


#else

static __inline void nd_short_to_netstream(char *buf, NDUINT16 val)
{
	buf[0] = (char) val  ;
	buf[1] = (val >> 8) & 0xff ;
}

static __inline void nd_long_to_netstream(char *buf, NDUINT32 val)
{
	buf[3] = (val>>24) & 0xff ;
	buf[2] = (val>>16) & 0xff ;
	buf[1] = (val>>8) & 0xff ;
	buf[0] = val & 0xff ;
}

static __inline void nd_longlong_to_netstream(char *buf, NDUINT64 val)
{
	
	buf[7] = (val>>56) & 0xff ;
	buf[6] = (val>>48) & 0xff ;
	buf[5] = (val>>40) & 0xff ;
	buf[4] = (val>>32) & 0xff ;
	
	buf[3] = (val>>24) & 0xff ;
	buf[2] = (val>>16) & 0xff ;
	buf[1] = (val>>8) & 0xff ;
	buf[0] = val & 0xff ;
}

static __inline NDUINT16 nd_netstream_to_short(const char *buf)
{
	return (NDUINT16) (buf[0]  | buf[1] << 8 ) ;
}

static __inline NDUINT32 nd_netstream_to_long(const char *buf)
{
	union {
		NDUINT32 val ;
		char _data[4] ;
	}v1 ;
	
	v1._data[0] = buf[0] ;
	v1._data[1] = buf[1] ;
	v1._data[2] = buf[2] ;
	v1._data[3] = buf[3] ;
	return v1.val ;
}

static __inline NDUINT64 nd_netstream_to_longlong(const char *buf)
{
	union {
		NDUINT64 val ;
		char _data[8] ;
	}v1 ;
	
	v1._data[0] = buf[0] ;
	v1._data[1] = buf[1] ;
	v1._data[2] = buf[2] ;
	v1._data[3] = buf[3] ;
	
	v1._data[4] = buf[4] ;
	v1._data[5] = buf[5] ;
	v1._data[6] = buf[6] ;
	v1._data[7] = buf[7] ;
	
	return v1.val ;
}
#endif


static __inline void nd_float_to_netstream(char *buf, float fval)
{
	union {
		char buf[4] ;
		float f;
	} val ;
	val.f = fval;
	buf[0] = val.buf[0] ;
	buf[1] = val.buf[1] ;
	buf[2] = val.buf[2] ;
	buf[3] = val.buf[3] ;
	
}

static __inline void nd_double_to_netstream(char *buf, double dval)
{
	
	union {
		char buf[8] ;
		double d;
	} val ;
	val.d = dval;
	buf[0] = val.buf[0] ;
	buf[1] = val.buf[1] ;
	buf[2] = val.buf[2] ;
	buf[3] = val.buf[3] ;
	buf[4] = val.buf[4] ;
	buf[5] = val.buf[5] ;
	buf[6] = val.buf[6] ;
	buf[7] = val.buf[7] ;
	
}

////

static __inline float nd_netstream_to_float(const char *buf)
{
	union {
		char buf[4] ;
		float f;
	} val ;
	val.buf[0] = buf[0]  ;
	val.buf[1] =buf[1]  ;
	val.buf[2] =buf[2]  ;
	val.buf[3] =buf[3]  ;
	return  val.f;
	
}

static __inline double nd_netstream_to_double(const char *buf)
{
	union {
		char buf[8] ;
		double d;
	} val ;
	val.buf[0] = buf[0]  ;
	val.buf[1] =buf[1]  ;
	val.buf[2] =buf[2]  ;
	val.buf[3] =buf[3]  ;
	val.buf[4] =buf[4]  ;
	val.buf[5] =buf[5]  ;
	val.buf[6] =buf[6]  ;
	val.buf[7] =buf[7]  ;
	
	return val.d;
	
}

#endif
