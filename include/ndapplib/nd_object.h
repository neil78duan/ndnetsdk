/* file ndobject.h
 * header file of class NDObject 
 *
 * 2009-4-24 23:04
 */

#ifndef _NDOBJECT_H_
#define _NDOBJECT_H_

//#include <stdexcept>
#include "ndapplib/nd_iBaseObj.h"
#include "nd_common/nd_handle.h"
//#include <sys/types.h>


using namespace std ;
//#pragma  warning(push)
#pragma  warning (disable : 4290 )
#pragma  warning (disable : 4291 )

class  ND_COMMON_CLASS  NDObject : public NDIBaseObj
{
protected :
	NDObject();
	virtual ~NDObject() {}
public :

	virtual nd_handle GetHandle() ;
	virtual NDObject* GetParent() ;
	virtual int LastError() ;
	virtual void SetLastError(NDUINT32 errcode) ;

	virtual int Create(const char *name) ;
	virtual void Destroy(int flag=0) ;
	
#ifndef DND_CLIENT_ONLY
	virtual void OnCreate() ;			//call on create
	virtual void OnDestroy() ;

	virtual int Close(int flag=0) ;
	virtual int Open(int param) ;
	virtual void OnClose() ;
	virtual void OnInitilize() ;		// call on open
#endif
	
    virtual nd_handle GetMmpool() ;
    virtual int SetMmpool(nd_handle pool) ;
	virtual void *getScriptEngine();
	
	const char *getName();
	void setName(const char *name);

	static NDObject * FromHandle(nd_handle h);

protected:
    NDUINT8 m_bPoolOwner ;
    nd_handle m_pool ;
    
    nd_handle m_objhandle ;
};

typedef void (NDObject::*NDObjectFunc)();


//#pragma warning (pop)

#endif 
