/* file nd_objmgr.h
 *
 * object manager of ndproject
 *
 * create by duan
 * 2011/2/13 11:48:47
 */

#ifndef _ND_OBJMGR_H_
#define _ND_OBJMGR_H_

//#include "nd_common/nd_common.h"
#include "ndapplib/nd_object.h"
#include "nd_common/nd_node_mgr.h"

//INSTANCE ID / objectID CLASSID
typedef NDUINT16 OBJECTID_T;

typedef node_root* nd_manager_handle;
typedef node_iterator nd_object_iterator ;
class NDObjectIterator ;

//manager session in listenor
class  ND_COMMON_CLASS NDObjectMgrBase
{
public:
	typedef NDObjectIterator iterator ;
	void Unlock(OBJECTID_T oid );
	void * Lock(OBJECTID_T oid);
	void * TryLock(OBJECTID_T oid);

	int GetCapacity();			//
	int GetActiveNum();			//

	bool IsValid();

	iterator begin() ;
	iterator end() ;

	//open allocator
	//int OpenAlloc(size_t size, int number) ;
	void SetMgr(nd_manager_handle cmmgr);

	NDObjectMgrBase(nd_manager_handle  cmmgr) ;
	NDObjectMgrBase() ;
	virtual ~NDObjectMgrBase() ;
	
	void CheckUnrelease(OBJECTID_T exceptid =0) ;
	ndthread_t GetOwner(OBJECTID_T oid) ;
	void SetOwner(OBJECTID_T oid,ndthread_t ownerid) ;
protected:
	nd_manager_handle handle;
	//nd_handle alloc_handle ;
};

class ND_COMMON_CLASS  NDObjectIterator
{
public:
	NDObjectIterator()  ;
	virtual ~NDObjectIterator() ;
	const NDObjectIterator& operator = (const NDObjectIterator &) ;
	bool operator == (const NDObjectIterator &) ;
	bool operator != (const NDObjectIterator &) ;
	NDObjectIterator& operator ++ () ;
	NDObjectIterator operator++ (int) ;

public:
	NDObjectIterator(nd_object_iterator &f, void *s, nd_manager_handle h) ;	
	nd_object_iterator first;
	void *second;
	friend class NDObjectMgrBase;
private:
	int m_needrelease ;
	nd_manager_handle handle;
};

template <class T>
class NDObjectMgr :public NDObjectMgrBase
{
public:
	typedef T                 value_type;	
	typedef value_type*       pointer;	
	typedef const value_type* const_pointer;	
	typedef value_type&       reference;	
	typedef const value_type& const_reference;	
	typedef size_t       size_type;	
	typedef ptrdiff_t    difference_type;	

	NDObjectMgr() 
	{
		memset(&m_root , 0, sizeof(m_root)) ;
	}
	virtual ~NDObjectMgr() 
	{
	}
	pointer GetObject(NDObjectMgrBase::iterator &it) 
	{
		return static_cast<pointer>(it.second) ;
	}

	int Create(int objnum, nd_handle mmpool=NULL) 
	{
		if (-1==nd_node_create(&m_root,objnum,sizeof(value_type),1,mmpool) ) {
			return -1 ;
		}
		handle = &m_root;
		return 0 ;
	}
	void Destroy()
	{
		nd_node_destroy(&m_root) ;
		handle = 0 ;
		memset(&m_root , 0, sizeof(m_root)) ;
	}
	pointer Lock(OBJECTID_T oid)
	{
		void *addr = NDObjectMgrBase::Lock(oid) ;
		if(!addr){return NULL;}
		return static_cast<pointer>(addr) ;
	}

	pointer TryLock(OBJECTID_T oid)
	{
		void *addr = NDObjectMgrBase::TryLock(oid) ;
		if(!addr){return NULL;}
		return static_cast<pointer>(addr) ;
	}

	OBJECTID_T Construct() 
	{
		void *addr = m_root.alloc(m_root.node_alloctor) ;
		if(!addr) {return 0;}
		pointer p = new(addr) value_type ;
		OBJECTID_T oid = m_root.accept(&m_root,(void*)p) ;
		if(0==oid) {
			p->~T() ;
			m_root.dealloc(p,m_root.node_alloctor) ;
			return 0 ;
		}
		m_root.unlock(&m_root,oid) ;
		return oid;
	}
	void Destruct(OBJECTID_T oid)
	{
		pointer p = Lock(oid);
		if(!p) return ;
		if(-1!=m_root.deaccept(&m_root,oid) ){
			p->~T() ;
			m_root.dealloc((void*)p,m_root.node_alloctor) ;
		}
		Unlock(oid) ;
	}

protected:
	node_root m_root;
};

#endif
