/* 
 * file ndsingleton.h
 * define singleton
 *
 * create by duan
 * 2015-7-27
*/

#ifndef _ND_SINGLETON_H_
#define _ND_SINGLETON_H_

template <typename  T>
class NDSingleton {
public:
	NDSingleton() : _addr(0)
	{

	}
	static T *Get() 
	{
		if (!_addr)
		{
			_addr = new T;
		}
		return _addr;
	}
	static void Destroy()
	{
		if (_addr)
		{
			delete _addr;
			_addr = NULL;
		}
	}
	static bool Check()
	{
		return _addr ? true : false;
	}
	virtual ~NDSingleton()
	{
		if (_addr)
		{
			delete _addr;
			_addr = 0; 
		}
	}
private:
	static T *_addr;
};
template<typename T> T * NDSingleton<T>::_addr = NULL;

#endif
