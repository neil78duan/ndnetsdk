/* file nd_ibaseObj.h
 * 
 * define virtual base interface 
 *
 * create by duan 
 *
 * 2018.4.24
 */

#ifndef _ND_IBASE_OBJ_H_
#define _ND_IBASE_OBJ_H_
#include "nd_common/nd_export_def.h"

class ND_COMMON_CLASS NDIBaseObj
{
public:

	virtual int Create(const char *name=0) = 0;
	virtual void Destroy(int flag = 0)= 0;
protected:

	NDIBaseObj() {}
	virtual~NDIBaseObj() {}
};

#endif // !_ND_IBASE_OBJ_H_
