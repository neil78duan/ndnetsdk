
/* file nd_objfactory.h
 *
 * create by duan 
 *
 * 2019.5.28
 */

#ifndef _ND_OBJFACTORY_H_
#define _ND_OBJFACTORY_H_

#include "ndstl/nd_utility.h"
#include "ndstl/nd_new.h"
#include "ndapplib/nd_object.h"

class nd_fectory_base : public NDObject
{
public:
	nd_fectory_base() {} ;
	virtual ~nd_fectory_base() {} 
	virtual NDObject *construct(void *p) = 0;
	//virtual NDObject *construct() = 0;
	virtual void destruct(void *p) = 0;
	virtual void selfdestroy() = 0 ;
};
template<class T>
class nd_fectory : public  nd_fectory_base
{
public:
	typedef T                 value_type;	
	typedef value_type*       pointer;	
	typedef const value_type* const_pointer;	
	typedef value_type&       reference;	
	typedef const value_type& const_reference;	
	typedef size_t       size_type;	
	typedef ptrdiff_t    difference_type;	

	nd_fectory() {}
	virtual ~nd_fectory() {}
	pointer construct(void *p){	return new(p) value_type;}
	//pointer construct() {return new value_type;}
	void destruct(void* obj) {
		pointer p = static_cast<pointer>(obj) ;
		_NDDestroy(p) ;
	}
	void selfdestroy(){delete this;}

	pointer address(void*p) {return (void*) (static_cast<pointer>(p));}
};


#endif
