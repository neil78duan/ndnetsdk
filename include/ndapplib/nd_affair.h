/* file nd_affair.h
 *
 * define affair 
 *
 * create by duan 
 *
 * 2011/3/24 11:13:42
 */

#ifndef _ND_AFFAIR_H_
#define _ND_AFFAIR_H_

#include "ndapplib/applib.h"
//事务打开,保存,同步开关
template<class TAfair>
class EnableAffairHelper
{
public:
	EnableAffairHelper(TAfair *pa, int flag) 
	{
		m_oldflag = pa->SetEable(flag?true:false) ;
		m_pa = pa ;
	}
	~EnableAffairHelper() 
	{
		if (m_pa){
			m_pa->SetEable(m_oldflag) ;
		}
	}
private:
	TAfair *m_pa ;
	bool m_oldflag ;
};

template<class TAfair>
class SyndbFlagHelper
{
public:
	SyndbFlagHelper(TAfair *pa, int flag) 
	{
		m_oldflag = pa->SetSyncDB(flag?true:false) ;
		m_pa = pa ;
	}
	~SyndbFlagHelper() 
	{
		if (m_pa){
			m_pa->SetSyncDB(m_oldflag) ;
		}
	}
private:
	TAfair *m_pa ;
	bool m_oldflag ;
};

template<class TAfair>
class NDAffairHelper
{
public:
	NDAffairHelper(TAfair *ta):m_affair(ta) 
	{
		m_affair->Begin() ;
	}
	virtual ~NDAffairHelper() 
	{
		if (m_affair->GetAffairStat()){
			m_affair->Commit() ;
		}
	}
	void Rollback() 
	{
		m_affair->Rollback();
	}
private: 
	TAfair *m_affair ;
};

template<class TAfair>
class NotifyFlagHelper
{
public:
	NotifyFlagHelper(TAfair *pa, int flag) 
	{
		m_oldflag = pa->SetNotify(flag?true:false) ;
		m_pa = pa ;
	}
	~NotifyFlagHelper() 
	{
		if (m_pa){
			m_pa->SetNotify(m_oldflag) ;
		}
	}
private:
	TAfair *m_pa ;
	bool m_oldflag ;
};

//#include "nd_common/nd_common.h"
template<class TIndex ,  class TValue , int number>
class NDAffair
{
	typedef NDAffair<TIndex ,  TValue , number> _MyType ;
public:
	typedef SyndbFlagHelper<_MyType> FlagSaveHelper ;
	typedef EnableAffairHelper<_MyType> FlagEnableHelper ;
	typedef SyndbFlagHelper<_MyType> FlagSyndbHelper ;
	
	typedef NotifyFlagHelper<_MyType> FlagNtfClientHelper ;
	typedef NDAffairHelper<_MyType> NDAffairBeginHelper ;
	
	
	enum eAffairOp{
		EAO_ADD,
		EAO_DEL,
		EAO_MODIFIED
	};
	struct back_op{
		int optype ;
		TIndex first ;
		TValue second;
	};
	NDAffair() 
        : m_num(0)
        , m_nCount(0)
        , m_affair_stat(0)
        , m_notify(1)
        , m_enable(1)
        , m_syncdb(1)
	{

	}
	virtual ~NDAffair()
	{

	}
	int _affair_begin()
	{
		if (!m_enable){
			return -1;
		}
		if (m_nCount < 0 ){
			m_nCount = 0 ;
		}
		if (m_nCount++ == 0 )	{
			nd_assert(m_affair_stat==0) ;
			m_num = 0;
			m_affair_stat = 1 ;
		}
		return m_nCount ;
	}
	int _affair_commit() 
	{
		if (!m_enable || m_nCount <= 0) {
			return -1;
		}
		--m_nCount ;
		return m_nCount ;
	}
	
	virtual void Begin() 
	{
		_affair_begin() ;
// 		if (!m_enable){
// 			return ;
// 		}
// 		if (m_nCount++ == 0 )	{
// 			nd_assert(m_affair_stat==0) ;
// 			m_num = 0;
// 			m_affair_stat = 1 ;
// 		}
	
	}
	virtual void Commit() 
	{
		if (!m_enable || m_nCount < 0) {
			return ;
		}
		--m_nCount ;

        if (m_nCount > 0)  {
            return ;
        }

		if (m_affair_stat )	{

			m_enable = 0 ;
			for(int i=0; i<(int)m_num; i++) {
				int op = m_buf[i].optype;
				if (EAO_ADD==op)
					op = EAO_DEL;
				else if (op==EAO_DEL)
					op = EAO_ADD;
				AffairDo(m_buf[i].first, m_buf[i].second, op);
			}
			m_enable = 1 ;
		}
		m_affair_stat = 0 ;
		m_num = 0;
	}
	virtual void Rollback() 
	{
		if (!m_enable || m_nCount <= 0) {
			return ;
		}
		--m_nCount ;

		if (m_nCount > 0)  {
			return ;
		}

		if (m_affair_stat )	{
			m_enable = 0 ;
			for(int i=m_num-1; i>=0; i--) {
				Undo(m_buf[i].first,m_buf[i].second,m_buf[i].optype) ;
			}
			m_enable = 1 ;
		}
		m_affair_stat = 0 ;
		m_num = 0;
	}
	virtual void Undo(TIndex &index, TValue &old_val,int optype )
	{

	}
	virtual void AffairDo(TIndex &index, TValue &old_val, int optype)
	{

	}
	void Reset() 
	{
		m_num = 0;
		m_affair_stat = 0;
		m_nCount = 0 ;
	}
	void AffairAdd(TIndex &index, TValue &old_val)
	{
		if (m_affair_stat)	{
			nd_assert(m_num<=number);
			m_buf[m_num].first = index ;
			m_buf[m_num].second = old_val;
			m_buf[m_num].optype = EAO_DEL ;
			m_num++;
		}
	}
	void AffairDel(TIndex &index, TValue &old_val)
	{
		if (m_affair_stat)	{
			nd_assert(m_num<=number);
			m_buf[m_num].first = index ;
			m_buf[m_num].second = old_val;
			m_buf[m_num].optype = EAO_ADD ;
			m_num++;
		}
	}
	void AffairModify(TIndex &index, TValue &old_val)
	{
		if (m_affair_stat)	{
			nd_assert(m_num<=number);
			m_buf[m_num].first = index ;
			m_buf[m_num].second = old_val;
			m_buf[m_num].optype = EAO_MODIFIED ;
			m_num++;
		}
	}
	void AffairSet(TIndex &index, TValue &val ,int affair_op) 
	{
		if (m_affair_stat)	{
			nd_assert(m_num<=number);
			m_buf[m_num].first = index ;
			m_buf[m_num].second = val;
			m_buf[m_num].optype = affair_op ;
			m_num++;
		}
	}
	int GetAffairStat() {return m_affair_stat;}

	//设置是否通知客户端 flag = 0 不通知,返回原来的值
	bool SetNotify(bool bflag )
	{
		bool ret =	m_notify ? true : false ;
		if (m_affair_stat){
			return ret ;
		}

		m_notify = bflag ? 1:0;
		return ret ;
	}
	
	bool SetEable(bool bflag )
	{
		bool ret =	m_enable ? true : false ;
		if (m_affair_stat){
			return ret ;
		}

		m_enable = bflag ? 1:0;
		return ret ;
	}

	bool SetSyncDB(bool bflag) 
	{
		bool ret =	m_syncdb ? true : false ;
		if (m_affair_stat){
			return ret ;
		}

		m_syncdb = bflag ? 1:0;
		return ret ;
	}

	void EnableAffair(bool bflag = true)
	{
		if (m_affair_stat){
			return  ;
		}
		m_enable = bflag ? 1:0;
	}


	bool CheckInAffair() 
	{
		return m_enable && m_affair_stat ;
	}
	bool CheckSyncDB() {return m_syncdb?true :false; }
	bool CheckNotify() {return m_notify?true :false; }

protected:

	int m_nCount;           // 提交计数, 解决多次begin, 多次commit的问题.(这是暂行方案)
	NDUINT32 m_num:16 ;
	NDUINT32 m_affair_stat:1 ;   //0 not set 1 begin 
	NDUINT32 m_enable:1 ;		//是否打开事务
	NDUINT32 m_notify:1 ;
	NDUINT32 m_syncdb:1 ;
	back_op m_buf[number] ;
};


#endif
