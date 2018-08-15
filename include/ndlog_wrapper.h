/* file ndlog_wrapper.h
 *
 * redefine log output
 *
 * create by duan
 *
 * 2017.2.17
 */

#ifndef _NDLOG_WRAPPER_H_
#define _NDLOG_WRAPPER_H_


template <class T>
class NDLogWrapperBase
{
	typedef T                 value_type;
	typedef value_type*       pointer;

public:
	NDLogWrapperBase(pointer host)
	{
		m_oldLogFunction = nd_setlog_func((nd_log_entry)NDLogWrapperBase< T>::log);
		nd_log_no_file(1);
		nd_log_no_date(1);
		m_hostWindows = host;
	}

	~NDLogWrapperBase()
	{
		nd_setlog_func(m_oldLogFunction);
	}
	static int print(void *pf, const char *stm, ...)
	{
		char buf[1024 * 4];
		char *p = buf;
		va_list arg;
		int done;

		va_start(arg, stm);
		done = vsnprintf(p, sizeof(buf), stm, arg);
		va_end(arg);

		log(buf);
		return done;
	}

	static int log(const char *text)
	{
		if (m_hostWindows)		{
			m_hostWindows->WriteLog(text);
			return (int)strlen(text);
		}
		return 0;
	}

private:
	static pointer m_hostWindows;
	nd_log_entry m_oldLogFunction;
};

#define ND_LOG_WRAPPER_PRINT(_type) &(NDLogWrapperBase<_type>::print)
#define ND_LOG_WRAPPER_LOG(_type) &(NDLogWrapperBase<_type>::log)

#define ND_LOG_WRAPPER_IMPLEMENTION(_type) \
        template <typename _type> _type* NDLogWrapperBase< _type>::m_hostWindows = 0;	\
	static NDLogWrapperBase<_type> *_localLogHelperOf_##_type

#define ND_LOG_WRAPPER_NEW(_type) _localLogHelperOf_##_type = new NDLogWrapperBase<_type>(this)
#define ND_LOG_WRAPPER_DELETE(_type) if(_localLogHelperOf_##_type) {delete _localLogHelperOf_##_type ; _localLogHelperOf_##_type  =0;}
#endif
