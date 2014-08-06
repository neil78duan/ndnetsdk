/* file nd_text.h
 *
 * define text template
 *
 * create by duan
 * 2012/8/30 13:53:57
 */

#ifndef _ND_TEXT_
#define _ND_TEXT_

template<class _Type, int _number>
class nd_text
{
public:
	typedef _Type             value_type;	
	typedef value_type*       pointer;	
	typedef const value_type* const_pointer;	
	typedef value_type&       reference;	
	typedef const value_type& const_reference;	
	typedef nd_text<_Type, _number> MyT ;
	
	_Type m_textbuf[_number] ;
	nd_text() 
	{
		m_textbuf[0] = 0 ;
	}
	nd_text(const MyT &r) 
	{
		for(int i=0;i<_number;i++) {
			m_textbuf[i] = r.m_textbuf[i] ;
			if(r.m_textbuf[i]==0)
				break ;
		}
	}
	nd_text(_Type* p) 
	{
		for(int i=0;i<_number;i++) {
			m_textbuf[i] = p[i] ;
			if(p[i]==0)
				break ;
		}
	}
	nd_text(const _Type* p) 
	{
		for(int i=0;i<_number;i++) {
			m_textbuf[i] = p[i] ;
			if(p[i]==0)
				break ;
		}
	}
	pointer c_str() 
	{
		return m_textbuf ;
	}
	
	size_t capacity() {return (size_t)_number;}
	size_t size() 
	{
		int i = 0;
		for(;i<_number;i++) {
			if(m_textbuf[i]==0 )
				break ;
		}
		return (size_t)i ;
	}
	void clear() {m_textbuf[0] = 0 ;}
	
	MyT& operator += (const _Type* r) 
	{
		int i = 0 ,j;
		for(;i<_number;i++) {
			if(m_textbuf[i]==0)
				break ;
		}
		j = 0;
		for(;i<_number;i++) {
			m_textbuf[i] = r[j++] ;
			if (m_textbuf[i]==0){
				break ;
			}
		}
		return *this;
	}
	MyT& operator = (const _Type* r) 
	{
		for(int i=0;i<_number;i++) {
			m_textbuf[i] = r[i] ;
			if (m_textbuf[i]==0){
				break ;
			}
		}
		return *this;
	}
	MyT& operator= (const MyT &r) 
	{
		for(int i=0;i<_number;i++) {
			m_textbuf[i] = r.m_textbuf[i] ;
			if(r.m_textbuf[i]==0)
				break ;
		}
		return *this ;
	}
	bool operator> (const MyT &r) const
	{
		int ret = 0;
		for(int i=0;i<_number;i++) {
			ret = m_textbuf[i] - r.m_textbuf[i] ;
			if(ret|| m_textbuf[i]==0)
				break ;
		}
		return (ret>0) ;
	}

	bool operator<(const MyT &r) const
	{
		int ret = 0;
		for(int i=0;i<_number;i++) {
			ret = m_textbuf[i] - r.m_textbuf[i] ;
			if(ret || m_textbuf[i]==0)
				break ;
		}
		return ret<0 ;
	}
	bool operator== (const MyT &r) const
	{
		int ret = 0;
		for(int i=0;i<_number;i++) {
			ret = m_textbuf[i] - r.m_textbuf[i] ;
			if(ret|| m_textbuf[i]==0)
				break ;
		}
		return ret==0 ;
	}
	bool operator!= (const MyT &r) const
	{
		return !(*this == r);
	}
};

#endif
