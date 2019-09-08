/* file NDVarType.cpp
 *
 * define variable type for net message 
 *
 * create by duan
 */

#include "ndapplib/nd_vartype.h"
#include "nd_common/nd_str.h"
#include <stdlib.h>

static const char* _varTypeName[] = {
    "int" , "float", "string", "int8", "int16", "long", "binary"
} ;



NDVarType::NDVTYPE_ELEMENT_TYPE NDVarType::getTypeByName(const char *name)
{
    if(name && *name){
        for (int i=0; i<ND_ELEMENTS_NUM(_varTypeName); i++) {
            if(0==ndstricmp(name, _varTypeName[i])) {
                return (NDVarType::NDVTYPE_ELEMENT_TYPE)i ;
            }
        }
    }
    return NDVarType::ND_VT_FLOAT;
}

const char* NDVarType::getNameBytype(int type)
{
    if(type >= ND_ELEMENTS_NUM(_varTypeName) || type < 0) {
        return _varTypeName[0] ;
    }
    return  _varTypeName[type] ;
}

bool NDVarType::isNumber(int type)
{
    if (type == NDVarType::ND_VT_BINARY || type == NDVarType::ND_VT_STRING) {
        return false;
    }
    return true;
}

NDVarType::NDVarType() : m_type(ND_VT_INT)
{
	m_data.i64_val = 0;
}
NDVarType::~NDVarType()
{
	destroy();
}
NDVarType::NDVarType(const NDVarType &r)
{
	m_type = ND_VT_INT;
	m_data.i_val = 0;

	*this = r;
}

NDVarType::NDVarType(NDVarType::NDVTYPE_ELEMENT_TYPE type)
{
    destroy();
    m_type = type;
}
//init
//init set vale
NDVarType::NDVarType(int a)
{
	m_type = ND_VT_INT;
	m_data.i_val = a;
}
NDVarType::NDVarType(NDUINT8 a)
{
	m_type = ND_VT_INT8;
	m_data.i_val = a;
}
NDVarType::NDVarType(NDUINT16 a)
{
	m_type = ND_VT_INT16;
	m_data.i_val = a;
}
NDVarType::NDVarType(NDUINT64 a)
{
	m_type = ND_VT_INT64;
	m_data.i64_val = a;
}
NDVarType::NDVarType(float a)
{
	m_type = ND_VT_FLOAT;
	m_data.f_val = a;
}

NDVarType::NDVarType(bool a)
{
	m_type = ND_VT_INT;
	m_data.i_val = a?1:0;
}

NDVarType::NDVarType(const char* text)
{
	initSet(text);
}

NDVarType::NDVarType(void *bindata, size_t size)
{
	initSet(bindata, size);
}

void NDVarType::InitType(NDVarType::NDVTYPE_ELEMENT_TYPE type)
{
    destroy();
    m_type = type;
}

void NDVarType::destroy()
{
	if (m_type == ND_VT_STRING || m_type == ND_VT_BINARY) {
		if (m_data.str_val) { free(m_data.str_val); }		
	}
	m_data.str_val = 0;
}

bool NDVarType::initSet(const char*text)
{
	destroy();
	m_type = ND_VT_STRING;
	if (text && *text) {
		size_t size = strlen(text) + 1;
		m_data.str_val = (char*)malloc(size);
		if (m_data.str_val) {
			strncpy(m_data.str_val, text, size);
			return true;
		}
		return false;
	}
	return true;
}

bool NDVarType::initSet(void *bindata, size_t size)
{
	destroy();
	m_type = ND_VT_BINARY;
	if (size > 0) {
		m_data.bin_val = (ndvtype_bin *)malloc(size + sizeof(ndvtype_bin) + 4);
		if (m_data.bin_val) {
			memcpy(m_data.bin_val->data, bindata, size);
			m_data.bin_val->size = size;
			return true;
		}
		return false;
	}
	return true;
}

NDVarType &NDVarType::operator =(int a)
{
	destroy();
	m_type = ND_VT_INT;
	m_data.i_val = a;
	return *this;
}
NDVarType &NDVarType::operator =(NDUINT8 a)
{
	destroy();
	m_type = ND_VT_INT8;
	m_data.i_val = a;
	return *this;
}
NDVarType &NDVarType::operator =(NDUINT16 a)
{
	destroy();
	m_type = ND_VT_INT16;
	m_data.i_val = a;
	return *this;
}
NDVarType &NDVarType::operator =(NDUINT64 a)
{
	destroy();
	m_type = ND_VT_INT64;
	m_data.i64_val = a;
	return *this;
}
NDVarType &NDVarType::operator =(bool a)
{
	destroy();
	m_type = ND_VT_INT;
	m_data.i_val = a?1:0;
	return *this;
}
NDVarType &NDVarType::operator =(float a)
{
	destroy();
	m_type = ND_VT_FLOAT;
	m_data.f_val = a;
	return *this;
}
NDVarType &NDVarType::operator =(const char* text)
{
	destroy();
	initSet(text);
	return *this;
}
NDVarType &NDVarType::operator =(const NDVarType&r)
{
	destroy();
	switch (r.m_type)
	{
	case NDVarType::ND_VT_INT:
		*this = r.m_data.i_val;
		break;
	case NDVarType::ND_VT_FLOAT:
		*this = r.m_data.f_val;
		break;
	case NDVarType::ND_VT_STRING:
		initSet(r.m_data.str_val);
		break;
	case NDVarType::ND_VT_INT8:
		*this = r.getInt8();
		break;
	case NDVarType::ND_VT_INT16:
		*this = r.getInt16();
		break;
	case NDVarType::ND_VT_INT64:
		*this = r.getInt64();
		break;
	case NDVarType::ND_VT_BINARY:
		initSet(m_data.bin_val, m_data.bin_val->size);
		break;
	default:
		break;
	}
	return *this;
}


bool NDVarType::checkValid()const
{
	switch (m_type)
	{
	case NDVarType::ND_VT_BINARY:		
		if (m_data.bin_val && m_data.bin_val->size > 0) {
			return true;
		}
		break;
	case NDVarType::ND_VT_STRING:
		if (m_data.str_val && *m_data.str_val) {
			return true;
		}
		break;
	default:
		return 0 != m_data.i64_val;
	}
	return false;
}
//
//bool NDVarType::isNumber(int type)
//{
//    if (type == NDVarType::ND_VT_BINARY || type == NDVarType::ND_VT_STRING) {
//        return false;
//    }
//    return true;
//}
//
//bool NDVarType::isNumber()const
//{
//    if (m_type == NDVarType::ND_VT_BINARY || m_type == NDVarType::ND_VT_STRING) {
//        return false;
//    }
//    return true;
//}

int NDVarType::getInt()const
{
	if (ND_VT_FLOAT == m_type) {
		return (int)m_data.f_val;
	}
	else if (m_type == ND_VT_STRING) {
		if (m_data.str_val && m_data.str_val[0]) {
			return atoi(m_data.str_val);
		}
		else {
			return 0;
		}
	}
	else {
		return m_data.i_val;
	}
}

NDUINT8 NDVarType::getInt8()const
{
	return (NDUINT8)getInt();
}
NDUINT16 NDVarType::getInt16()const
{
	return (NDUINT16)getInt();
}
NDUINT64 NDVarType::getInt64()const
{
	if (ND_VT_FLOAT == m_type) {
		return (NDUINT64)m_data.f_val;
	}
	else if (m_type == ND_VT_STRING) {
		if (m_data.str_val && m_data.str_val[0]) {
			return atoll(m_data.str_val);
		}
		else {
			return 0;
		}
	}
	else {
		return m_data.i64_val;
	}
}

bool NDVarType::getBool()const
{
	if (m_type == ND_VT_STRING) {
		if (m_data.str_val && m_data.str_val[0]) {
			if (0 == ndstricmp(m_data.str_val, "yes") || 0 == ndstricmp(m_data.str_val, "true")) {
				return true;
			}
		}
		return false;
		
	}
	else {
		return getInt()?true:false;
	}
}

float NDVarType::getFloat()const
{
	if (ND_VT_FLOAT == m_type) {
		return m_data.f_val;
	}
	else if (m_type == ND_VT_STRING) {
		if (m_data.str_val && m_data.str_val[0]) {
			return (float)atof(m_data.str_val);
		}
		else {
			return 0;
		}
	}
	else {
		return (float)m_data.i64_val;
	}
}
const char *NDVarType::getText()const
{
	if (m_type == ND_VT_STRING) {
		return m_data.str_val;
	}
	else {
		return NULL;
	}
}

#if defined(ND_USE_STD_STRING)
std::string NDVarType::getString()const
{
	char tmpbuf[32];
	std::string retval;
	switch (m_type)
	{
	case NDVarType::ND_VT_INT:
		snprintf(tmpbuf, sizeof(tmpbuf), "%d", m_data.i_val);
		retval = tmpbuf;
		break;
	case NDVarType::ND_VT_FLOAT:
		snprintf(tmpbuf, sizeof(tmpbuf), "%f", m_data.f_val);
		retval = tmpbuf;
		break;
	case NDVarType::ND_VT_STRING:
		if (m_data.str_val) {
			retval = m_data.str_val;
		}
		break;
	case NDVarType::ND_VT_INT8:
		snprintf(tmpbuf, sizeof(tmpbuf), "%d", (NDUINT8)m_data.i_val);
		retval = tmpbuf;
		break;
	case NDVarType::ND_VT_INT16:
		snprintf(tmpbuf, sizeof(tmpbuf), "%d", (NDUINT16)m_data.i_val);
		retval = tmpbuf;
		break;
	case NDVarType::ND_VT_INT64:
		snprintf(tmpbuf, sizeof(tmpbuf), "%lld", m_data.i64_val);
		retval = tmpbuf;
		break;
	case NDVarType::ND_VT_BINARY:
		if (m_data.bin_val && m_data.bin_val->size) {
			retval.insert(0, m_data.bin_val->data, m_data.bin_val->size);
		}
		break;
	default:
		break;
	}
	return retval;
}
#endif 

void *NDVarType::getBin()const
{
	if (m_type == NDVarType::ND_VT_BINARY && m_data.bin_val) {
		return m_data.bin_val->data;
	}
	return NULL;
}
size_t NDVarType::getBinSize()const
{

	if (m_type == NDVarType::ND_VT_BINARY && m_data.bin_val) {
		return m_data.bin_val->size;
	}
	return 0;
}

//OPERATE

#define ND_VARDATA_MATH_OP(r,_OP) \
	switch (m_type)	{				\
	case ND_VT_INT8:						\
	case ND_VT_INT16:						\
	case ND_VT_INT:						\
	{									\
		int val = getInt() _OP r.getInt();	\
		return NDVarType(val);			\
	}				\
	case ND_VT_INT64:	\
	{				\
		NDUINT64 val = getInt64() _OP r.getInt64();	\
		return NDVarType(val);					\
	}				\
	case ND_VT_FLOAT:	\
	{				\
		float val = getFloat() _OP r.getFloat();	\
		return NDVarType(val);				\
	}				\
	default:							\
		break;							\
	}


NDVarType  NDVarType::operator+(const NDVarType &r) const
{
	ND_VARDATA_MATH_OP(r, +);
	return *this;
}
NDVarType  NDVarType::operator-(const NDVarType &r) const
{
	ND_VARDATA_MATH_OP(r, -);
	return *this;
}
NDVarType  NDVarType::operator*(const NDVarType &r) const
{
	ND_VARDATA_MATH_OP(r, *);
	return *this;
}
NDVarType  NDVarType::operator/(const NDVarType &r) const
{
	float f1 = getFloat();
	float f2 = r.getFloat();
	float val = 0;
	if (f2 == 0) {
		val = f1;
	}
	else {
		val = f1 / f2;
	}

	switch (m_type)
	{
	case ND_VT_INT8:
		return NDVarType((NDUINT8)val);
	case ND_VT_INT16:
		return NDVarType((NDUINT16)val);
	case ND_VT_INT:
		return NDVarType((int)val);
	case ND_VT_INT64:
		return NDVarType((NDUINT64)val);
	case ND_VT_FLOAT:
		return NDVarType(val);
	default:
		break;
	}
	return *this;
}

NDVarType &NDVarType::operator+=(const NDVarType &r)
{
	NDVarType val = *this;
	*this = val + r;
	return *this;
}
NDVarType &NDVarType::operator-=(const NDVarType &r)
{
	NDVarType val = *this;
	*this = val - r;
	return *this;
}
NDVarType &NDVarType::operator*=(const NDVarType &r)
{
	NDVarType val = *this;
	*this = val * r;
	return *this;
}
NDVarType &NDVarType::operator/=(const NDVarType &r)
{
	NDVarType val = *this;
	*this = val / r;
	return *this;
}

NDVarType  NDVarType::operator+(const char *text) const
{
	if (m_type == ND_VT_STRING) {
		size_t size = 0;
		size_t s1 = 0;
		size_t s2 = 0;
		char *p = NULL;
		if (m_data.str_val && *m_data.str_val) {
			s1 = strlen(m_data.str_val);
			size += s1;
		}
		if (text && text) {
			s2 = strlen(text);
			size += s2;
		}

		if (size==0) {
			return NDVarType("");
		}

		p = (char*)malloc(size + 1);
		if (!p) { 
			return NDVarType(""); 
		}
		if (s1) {
			strncpy(p, m_data.str_val, s1+1);
		}
		if (s2) {
			strncpy(p+s1, text, s2 + 1);
		}
		NDVarType ret;
		ret.m_type = ND_VT_STRING;
		ret.m_data.str_val = p;
		return ret;

// 		std::string str1;
// 		if (m_data.str_val && *m_data.str_val) {
// 			str1 = m_data.str_val;
// 		}
// 		if (text && text) {
// 			str1 += text;
// 		}
// 		return NDVarType(str1.c_str());
	}
	return NDVarType(text);
}

NDVarType &NDVarType::operator+=(const char *text)
{
	NDVarType val = *this;
	*this = val + text;
	return *this;
}

bool  NDVarType::operator <(const NDVarType &r) const
{
	return m_data.i64_val < r.m_data.i64_val;
}

bool  NDVarType::operator >(const NDVarType &r) const
{
	return m_data.i64_val > r.m_data.i64_val;
}
bool  NDVarType::operator ==(const NDVarType &r) const
{
	return m_data.i64_val == r.m_data.i64_val;
}

bool  NDVarType::operator >=(const NDVarType &r) const
{
	return m_data.i64_val >= r.m_data.i64_val;
}
bool  NDVarType::operator <=(const NDVarType &r) const
{
	return m_data.i64_val <= r.m_data.i64_val;
}
bool  NDVarType::operator !=(const NDVarType &r) const
{
	return m_data.i64_val != r.m_data.i64_val;
}
