/* file NDVarType.h
 *
 * define variable type for net message 
 *
 * create by duan
 */

#ifndef _ND_VAR_TYPE_H_
#define _ND_VAR_TYPE_H_

#include "nd_common/nd_export_def.h"
#include "nd_common/nd_define.h"

//#include <string>

struct ndvtype_bin
{
	size_t size;
	char data[0];
};

union ndvtype_data{
	int i_val;
	float f_val;
	NDUINT64 i64_val;
	char *str_val;
	ndvtype_bin *bin_val;
};

class ND_COMMON_CLASS NDVarType
{

public:
	enum NDVTYPE_ELEMENT_TYPE
	{
		ND_VT_INT = 0x0,
		ND_VT_FLOAT = 0x1,
		ND_VT_STRING = 0x2,
		ND_VT_INT8 = 0x3,
		ND_VT_INT16 = 0x4,
		ND_VT_INT64 = 0x5,
		ND_VT_BINARY = 0x7
	};

	NDVarType();
	virtual ~NDVarType();

	//init
	//init set vale
	NDVarType(int a);
	NDVarType(NDUINT8 a);
	NDVarType(NDUINT16 a);
	NDVarType(NDUINT64 a);
	NDVarType(bool a);
	NDVarType(float a);
	NDVarType(const char* text);
	NDVarType(void *bindata, size_t size);

	NDVTYPE_ELEMENT_TYPE getDataType()const { return m_type; }

	NDVarType &operator =(int a);
	NDVarType &operator =(NDUINT8 a);
	NDVarType &operator =(NDUINT16 a);
	NDVarType &operator =(NDUINT64 a);
	NDVarType &operator =(bool a);
	NDVarType &operator =(float a);
	NDVarType &operator =(const char* text);
	NDVarType &operator =(const NDVarType&r);
	
	NDVarType  operator+(const NDVarType &r) const;
	NDVarType  operator-(const NDVarType &r) const;
	NDVarType  operator*(const NDVarType &r) const;
	NDVarType  operator/(const NDVarType &r) const;

	NDVarType &operator+=(const NDVarType &r);
	NDVarType &operator-=(const NDVarType &r) ;
	NDVarType &operator*=(const NDVarType &r) ;
	NDVarType &operator/=(const NDVarType &r) ;

	NDVarType  operator+(const char *text) const;
	NDVarType &operator+=(const char *text);


	int getInt()const;
	NDUINT8 getInt8()const;
	NDUINT16 getInt16()const;
	NDUINT64 getInt64()const;
	bool getBool()const;
	float getFloat()const;
	const char *getText()const;
	//std::string getString()const;
	void *getBin()const;
	size_t getBinSize()const;
	bool initSet(void *bindata, size_t size);
protected:
	bool initSet(const char*);
	void destroy();
	NDVTYPE_ELEMENT_TYPE m_type;
	ndvtype_data m_data;
};

#endif
