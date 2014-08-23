/* file nd_array.h
 * 
 * define  array 
 *
 * create by duan 
 * 2011/3/28 15:34:49
 */

#ifndef _ND_ARRAY_H_
#define _ND_ARRAY_H_

template< class T , int number>
class NDArray
{
public:
	typedef T                           value_type      ;	
	typedef value_type*                 pointer         ;	
	typedef const value_type*           const_pointer   ;	
	typedef value_type&                 reference       ;	
	typedef const value_type&           const_reference ;	
	typedef size_t                      size_type       ;	
	typedef ptrdiff_t                   difference_type ;
	typedef value_type*	                iterator        ;
    typedef const value_type*           const_iterator  ;
	typedef NDArray<value_type, number> MyT             ;

public:
	NDArray() : m_num(0) {}
	virtual ~NDArray(){}

public:
	iterator push_back(const_reference val)
	{
		ND_TRACE_FUNC() ;

		if (m_num < number)
        {
			m_buf[m_num++] = val;
			return &m_buf[m_num-1];
		}
		return end();
	}
	bool pop_back(pointer outval)
	{
		ND_TRACE_FUNC() ;
		if (m_num > 0)
        {
			*outval = m_buf[m_num--];
			return true ;
		}
		return false;
	}
	bool erase(iterator it) 
	{
		ND_TRACE_FUNC() ;

		if ( m_num < 1 )
        {
			return false;
		}
		if ( it < m_buf || it >= (m_buf + m_num) )
        {
			return false;
		}

		for(; it < &m_buf[m_num-1]; it++) 
        {
			*it = *(it + 1) ;
		}

		--m_num;

		return true;
	}
	bool erase(int index) 
	{
		ND_TRACE_FUNC() ;

		if (m_num<1 || index <0 || index >= m_num)
        {
			return false ;
		}

		for(; index<m_num-1; index++)
        {
			m_buf[index] = m_buf[index+1];
		}

		--m_num;

		return true;
	}

    iterator eraseIterator(iterator it)
	{
		ND_TRACE_FUNC() ;
        nd_assert(m_num >= 1);
        nd_assert(m_buf <= it && it < m_buf + m_num);

        --m_num;
        for (int i = (int)(it - m_buf); i < m_num; ++i)
        {
            m_buf[i] = m_buf[i+1];
        }
        return it;
    }

    int         capacity()  { return number;        }
    int         size()      { return m_num;         }
    void        clear()     { m_num = 0 ;           }
    iterator    begin()     { return m_buf;         }
    iterator    end()       { return m_buf + m_num; }

	const_reference operator[](size_type pos) const
	{
		nd_assert(pos < (size_type)m_num) ;
		return m_buf[pos] ;
	}

    reference operator[](size_type pos)
    {
        nd_assert(pos < (size_type)m_num) ;
        return m_buf[pos] ;
    }

	MyT& operator = (const MyT &r) 
	{
		ND_TRACE_FUNC() ;

		for(int i=0; i<r.m_num; i++)
        {
			m_buf[i]= r.m_buf[i] ;
		}

		m_num = r.m_num ;

		return *this ;
	}

protected:
	int m_num ;
	value_type m_buf[number] ;
};

#endif
