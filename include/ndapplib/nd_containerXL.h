/* file nd_containerXL.h
 *
 * from s -> m -> l -> xl container
 *
 * create by duan 
 * 2012/1/3 15:46:29
 */

#ifndef _ND_CONTAINER_XL_H_
#define _ND_CONTAINER_XL_H_

#include "nd_common/nd_common.h"

#pragma warning (disable : 4200)


#include "ndstl/nd_safemap.h"
#include "ndapplib/applib.h"
template<class _Tkey, class _Ttype>
class NDContainerXL: public nd_safemap<_Tkey,_Ttype>
{
	typedef  nd_safemap<_Tkey,_Ttype> _MyBase ;
public:
// 	typedef _Tkey				value_key;
// 	typedef _Ttype				value_type;	
// 	typedef value_type*			pointer;	
// 	typedef const value_type*	const_pointer;	
// 	typedef value_type&			reference;	
// 	typedef const value_type&	const_reference;	
	typedef NDContainerXL<_Tkey,_Ttype> _MyT ;
	typedef typename _MyBase::iterator iterator ;
	typedef _Ttype*			pointer;	
	typedef _Tkey			value_key;
	typedef typename _MyBase::value_node value_node;


	NDContainerXL() :m_create(0){  }
	virtual ~NDContainerXL() {_MyT::Destroy();}

	class NodeGetHelper
	{
	public :
		//typedef typename _Ttype* pointer ;	
		//typedef typename _Ttype& reference ;	
		typedef typename _MyT::iterator iterator ;	

		NodeGetHelper(_MyT &owner,const value_key& id ,pointer &paddr)  : m_owner(owner)
		{
			paddr = NULL ;
			m_it = m_owner.find(id) ;
			if (m_it != m_owner.end()){
				paddr = &(m_it->second) ;
			}
		}
		~NodeGetHelper() 
		{
		}

	private:
		iterator m_it ;
		_MyT &m_owner ;
	};

	bool CheckValid() {return m_create?true : false ;}

	int Create(const char *name, nd_handle pool=NULL, bool with_multithread=true) 
	{
		if (m_create)
			return 0 ;
		_MyBase::_set_pool(pool) ;
		m_create = 1;
		return 0;

	}
	void Destroy() 
	{
		m_create = 0;
		_MyBase::_set_pool(0) ;
	}

	bool Del(_Tkey &id, _Ttype* backup)
	{
		iterator it = _MyBase::find(id) ;
		if (it == _MyBase::end()){
			return false ;
		}
		if (backup) {
			*backup = it->second;
		}
		_MyBase::erase(it) ;
		return true ;

	}
	bool Del(_Ttype* node)
	{
		if (!node){
			return false ;
		}
		value_node *paddr = _MyBase::ptr2treenode( node) ;
		if (paddr){
			return _MyBase::_set_erase_node(paddr);
		}
		return false;
	}
	bool Add(const _Tkey &f, const _Ttype& val)
	{
		typename _MyBase::_Pairib ret = _MyBase::insert( nd_make_pair(f,val)) ;
		return ret.second ;
	}
	bool Add(_Tkey &f, _Ttype& val)
	{
		typename _MyBase::_Pairib ret = _MyBase::insert( nd_make_pair(f,val)) ;
		return ret.second ;
	}
	bool fetchData(const _Tkey &k, _Ttype &outVal)
	{
		iterator it = _MyBase::find(k);
		if (it == _MyBase::end()){
			return false;
		}
		outVal = it->second;
		return true;
	}
	bool setData(const _Tkey &k, const _Ttype &inVal)
	{
		iterator it = _MyBase::find(k);
		if (it == _MyBase::end()){
			return false;
		}
		it->second = inVal;
		return true;
	}
// 	pointer LockNode(value_key& id) 
// 	{
// 		nd_assert(m_create) ;
// 		pointer  ret = NULL;
// 		_lock() ;
// 		tree_iterator it = m_base_tree.search(id) ;
// 		_unlock() ;
// 		if (it != m_base_tree.end()) {
// 			if (IncRef(&(*it))){
// 				ret = &(it->second);
// 			}
// 		}
// 
// 		return ret ;
// 	}
// 	bool UnlockNode(pointer p) 
// 	{
// 		nd_assert(m_create) ;
// 		value_node *node = get_node_addr( p );
// 		if (node)
// 			return DecRef(node) ;
// 		return false;
// 	}
// 
// 	pointer LockFirst() 
// 	{
// 		pointer pnode = NULL;
// 		_lock() ;
// 		tree_iterator it = m_base_tree.begin() ;
// 		do 	{
// 			if (it == m_base_tree.end())	{
// 				break ;
// 			}
// 			else if(IncRef(&(*it))) {
// 				pnode = &(it->second);
// 				break ;
// 			}
// 			else 
// 				it++ ;
// 		} while ( it !=m_base_tree.end() );
// 
// 		_unlock() ;
// 		return pnode ;
// 	}
// 
// 	pointer LockNext(pointer p) 
// 	{
// 		if (!p){
// 			return NULL;
// 		}
// 		value_node *cur_node = get_node_addr( p );
// 		value_node *pnode = _lock_next(cur_node) ;
// 		DecRef(cur_node) ;
// 		if (!pnode){
// 			return NULL;
// 		}
// 		else 
// 			return &(pnode->second);
// 	}


protected:

	NDUINT8 m_create ;
};


//////////////////////////////////////////////////////////////////////////
/*
#include "nd_rbtree.h"
template<class _Tkey, class _Ttype>
class NDContainerXL  
{
	typedef nd_base_rbtree<_Tkey, _Ttype>xltree;
	typedef typename xltree::iterator tree_iterator;
	typedef typename xltree::rbtree_node value_node ;
public:
	typedef _Tkey				value_key;
	typedef _Ttype				value_type;	
	typedef value_type*			pointer;	
	typedef const value_type*	const_pointer;	
	typedef value_type&			reference;	
	typedef const value_type&	const_reference;	
	typedef size_t				size_type;	
	typedef ptrdiff_t			difference_type;
	typedef NDContainerXL<value_key, value_type> _MyT ;
	

	NDContainerXL() :m_create(0),m_lock(0),m_mmpool(0),m_selfpool(0){  }
	virtual ~NDContainerXL() {Destroy();}

	class NodeGetHelper
	{
	public :
		typedef typename _MyT::pointer pointer ;	
		NodeGetHelper(_MyT &owner,value_key id ,pointer &paddr)  : m_owner(owner),addr(paddr)
		{
			addr = owner.LockNode(id) ;
		}
		~NodeGetHelper() 
		{
			if (addr)	{
				m_owner.UnlockNode(addr) ;
				addr = 0 ;
			}
		}

	private:
		pointer &addr ;
		_MyT &m_owner ;
	};

	bool CheckValid() {return m_create?true : false ;}

	int Create(const char *name, nd_handle pool=NULL, bool with_multithread=true) 
	{
		if (m_create)
			return 0 ;
		if (pool){
			m_selfpool = 0 ;
			m_mmpool = pool ;
		}
		else {
			m_selfpool = 1 ;
			m_mmpool = nd_pool_create(EMEMPOOL_UNLIMIT,(char*)name) ;
			if (!m_mmpool)
				return -1 ;
		}

		if (with_multithread){
			m_lock =(nd_mutex *) nd_pool_alloc(m_mmpool,sizeof(nd_mutex)) ;
			nd_mutex_init(m_lock) ;
		}
		else {
			m_lock = 0 ;
		}
		m_create = true ;
		return 0;

	}
	void Destroy() 
	{
		if (m_create) {
			clear() ;
			if (m_lock){
				nd_mutex_destroy(m_lock) ;
				nd_pool_free(m_mmpool, m_lock) ;
				m_lock = 0 ;
			}
			if (m_selfpool){
				nd_pool_destroy(m_mmpool,0) ;
				m_selfpool = 0;
			}
			else {
				m_mmpool = 0 ;
			}
		}
		m_create = 0;
	}

	bool Del(value_key &id, pointer backup)
	{
		nd_assert(m_create) ;
		bool ret = false ;
		tree_iterator it ;
		_lock() ;
		it = m_base_tree.search(id) ;
		_unlock() ;

		if (it != m_base_tree.end()) {
			if(backup)
				*backup = it->second ;
			ret = DecRef(&(*it)) ;
		}
		return ret ;

	}
	bool Del(pointer node)
	{
		nd_assert(m_create) ;
		if (node) {
			return DecRef(get_node_addr(node) ) ;
		}
		return false ;
	}
	bool Add(value_key &f, const_reference val)
	{
		nd_assert(m_create) ;
		typename xltree::rbtree_node* insert_data = Construct() ;
		if (!insert_data){
			return false ;
		}
		m_base_tree.initilize_node(*insert_data, &f, (pointer)&val ) ;
		nd_atomic_set(&insert_data->ref_count, 1) ;

		_lock() ;
		bool ins_ok = m_base_tree.insert(insert_data) ;
		_unlock() ;
		if(!ins_ok){
			Destruct(insert_data) ;
			return false ;
		}
		return true;
	}
	bool Add(value_key &f, reference val)
	{
		return Add(f,(const_reference)val) ;
	}
	pointer LockNode(value_key& id) 
	{
		nd_assert(m_create) ;
		pointer  ret = NULL;
		_lock() ;
		tree_iterator it = m_base_tree.search(id) ;
		_unlock() ;
		if (it != m_base_tree.end()) {
			if (IncRef(&(*it))){
				ret = &(it->second);
			}
		}

		return ret ;
	}
	bool UnlockNode(pointer p) 
	{
		nd_assert(m_create) ;
		value_node *node = get_node_addr( p );
		if (node)
			return DecRef(node) ;
		return false;
	}

	pointer LockFirst() 
	{
		pointer pnode = NULL;
		_lock() ;
		tree_iterator it = m_base_tree.begin() ;
		do 	{
			if (it == m_base_tree.end())	{
				break ;
			}
			else if(IncRef(&(*it))) {
				pnode = &(it->second);
				break ;
			}
			else 
				it++ ;
		} while ( it !=m_base_tree.end() );
		
		_unlock() ;
		return pnode ;
	}

	pointer LockNext(pointer p) 
	{
		if (!p){
			return NULL;
		}
		value_node *cur_node = get_node_addr( p );
		value_node *pnode = _lock_next(cur_node) ;
		DecRef(cur_node) ;
		if (!pnode){
			return NULL;
		}
		else 
			return &(pnode->second);
	}

	int size() 
	{
		return m_base_tree.size() ;
	}

	void clear() 
	{
		_lock() ;
		for(tree_iterator it =m_base_tree.begin(); it!=m_base_tree.end(); ) {
			tree_iterator next_it = m_base_tree.erase(it);
			Destruct(&*it) ;
			if (m_base_tree.size()==0 || next_it == m_base_tree.end() ){
				break ;
			}
			it = next_it ;
		}
		_unlock() ;
	}

protected:

	bool IncRef(value_node* cur_node) 
	{
		nd_assert(m_create) ;
		ndatomic_t tmp ;
		while((tmp = nd_atomic_read(&cur_node->ref_count)) > 0) {
			if(nd_compare_swap(&cur_node->ref_count,tmp,tmp+1)) {
				return true;
			}
		}
		return false ;
	}

	bool DecRef(value_node* cur_node) 
	{
		nd_assert(m_create) ;

		ndatomic_t tmp ;
		while((tmp = nd_atomic_read(&cur_node->ref_count)) > 0) {
			if(nd_compare_swap(&cur_node->ref_count,tmp,tmp-1)) {
				if (tmp==1) {
					erase(cur_node) ;
				}
				return true ;
			}
		}
		return false ;
	}


	value_node* Construct() 
	{
		nd_assert(m_create) ;
		value_node* ins_node =(value_node*) nd_pool_alloc(m_mmpool,sizeof(*ins_node))  ;
		if (!ins_node){
			return NULL;
		}
		ins_node = new(ins_node) value_node ;
		rb_init_node(&ins_node->hdr);
		ins_node->ref_count = 0 ;
		return ins_node;
	}

	void Destruct(value_node* ins_node) 
	{
		ins_node->~value_node();
		nd_pool_free(m_mmpool,(void*)ins_node) ;
	}
	

	bool erase(value_node* cur_node)
	{
		nd_assert(m_create) ;
		
		nd_assert(cur_node->ref_count==0) ;
		_lock() ;
		m_base_tree.erase(tree_iterator(cur_node)) ;
		_unlock() ;

		Destruct(cur_node) ;

		return true ;
	}

	value_node* _lock_next(value_node* cur_node) 
	{
		value_node* ret = NULL;
		nd_assert(cur_node->ref_count > 0) ;

		_lock() ;
		rb_node *rbcur = &cur_node->hdr ;
		do{
			rb_node  *node = rb_next(rbcur) ;
			if (node){
				ret = rb_entry(node,value_node, hdr) ;
				if(IncRef(ret)) 
					break ;

			}else {
				ret = NULL;
			}
			rbcur = node ;
			
		}while(rbcur) ;
		_unlock() ;
		return ret;
	}
	inline void _lock() {	if (m_lock)nd_mutex_lock(m_lock) ;	}
	inline void _unlock()  {	if (m_lock)nd_mutex_unlock(m_lock) ;	}

	value_node *get_node_addr(pointer cur_node)
	{
		return rb_entry(cur_node, value_node, second) ;
	}

	NDUINT8 m_create, m_selfpool ;
	nd_mutex *m_lock ;
	nd_handle m_mmpool;
	//listbase_map m_datas;
	//struct list_head m_header ;
	
	xltree m_base_tree;
};
*/
#endif
