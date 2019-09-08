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

#define ND_USE_STD_STRING 1
#ifdef ND_USE_STD_STRING
#include <string>
#endif 

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
		ND_VT_BINARY = 0x6
	};

	NDVarType();
	~NDVarType();

	//init
	//init set vale
	NDVarType(const NDVarType &r);
    NDVarType(NDVarType::NDVTYPE_ELEMENT_TYPE type);
	NDVarType(int a);
	NDVarType(NDUINT8 a);
	NDVarType(NDUINT16 a);
	NDVarType(NDUINT64 a);
	NDVarType(bool a);
	NDVarType(float a);
	NDVarType(const char* text);
	NDVarType(void *bindata, size_t size);

    
    void InitType(NDVarType::NDVTYPE_ELEMENT_TYPE type);
    void destroy();
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

	bool  operator <(const NDVarType &r) const;
	bool  operator >(const NDVarType &r) const;
	bool  operator ==(const NDVarType &r) const;
	bool  operator >=(const NDVarType &r) const;
	bool  operator <=(const NDVarType &r) const;
	bool  operator !=(const NDVarType &r) const;

	operator float() const { return getFloat(); }
	operator int() const { return getInt(); }
	operator const char*() const { return getText(); }

	bool checkValid()const;// check value is unzero
    bool isNumber()const {return NDVarType::isNumber(m_type);}

	int getInt()const;
	NDUINT8 getInt8()const;
	NDUINT16 getInt16()const;
	NDUINT64 getInt64()const;
	bool getBool()const;
	float getFloat()const;
	const char *getText()const;
    
    static NDVarType::NDVTYPE_ELEMENT_TYPE getTypeByName(const char *teypName) ;
    static const char* getNameBytype(int type) ;
    static bool isNumber(int type) ;

#ifdef ND_USE_STD_STRING
	std::string getString()const;
#endif 
	void *getBin()const;
	size_t getBinSize()const;
	bool initSet(void *bindata, size_t size);
protected:
	bool initSet(const char*);
	NDVTYPE_ELEMENT_TYPE m_type;
	ndvtype_data m_data;
};

#endif
