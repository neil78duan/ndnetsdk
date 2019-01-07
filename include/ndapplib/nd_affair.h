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
#include <vector>
#include <map>

//affair switch helper
template<class TAfair>
class ND_COMMON_CLASS  EnableAffairHelper
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
class  ND_COMMON_CLASS SyndbFlagHelper
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
		if (m_affair && m_affair->GetAffairStat()){
			m_affair->Commit() ;
		}
	}
	void Rollback() 
	{
		m_affair->Rollback();
		m_affair = NULL;
	}
private: 
	TAfair *m_affair ;
};

template<class TAfair>
class NotifyFlagHelper
{
public:
	NotifyFlagHelper(TAfair *pa, bool flag) 
	{
		m_oldflag = pa->SetNotify(flag) ;
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

template<class TAfair>
class NDSetCommitNtfHelper
{
public:
	NDSetCommitNtfHelper(TAfair *pa, bool flag)
	{
		m_oldflag = pa->SetCommitNtf(flag);
		m_pa = pa;
	}
	~NDSetCommitNtfHelper()
	{
		if (m_pa){
			m_pa->SetCommitNtf(m_oldflag);
		}
	}
private:
	TAfair *m_pa;
	bool m_oldflag;
};


//#include "nd_common/nd_common.h"
template<class TIndex ,  class TValue >
class NDAffair
{
	typedef NDAffair<TIndex ,  TValue > _MyType ;
public:
	typedef SyndbFlagHelper<_MyType> FlagSaveHelper ;
	typedef EnableAffairHelper<_MyType> FlagEnableHelper ;
	typedef SyndbFlagHelper<_MyType> FlagSyndbHelper ;
	
	typedef NotifyFlagHelper<_MyType> FlagNtfClientHelper ;
	typedef NDAffairHelper<_MyType> NDAffairBeginHelper ;

	typedef NDSetCommitNtfHelper<_MyType> FlagCommitNtfHelper;
	
	
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
        //: //m_num(0)
		: m_nCount(0)
        , m_affair_stat(0)
        , m_notify(0)
        , m_enable(0)
        , m_syncdb(0)
		, m_commitNtf(0)
		, m_dataChanged(0)
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
			//m_num = 0;
			m_buf.clear() ;
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

		if (m_affair_stat && m_commitNtf)	{

			typedef std::map<TIndex, TValue> changedOperates_map;
			changedOperates_map changeAffair;

			m_enable = 0 ;
			for(int i=0; i<(int)m_buf.size(); i++) {
				m_dataChanged = 1;
				int op = m_buf[i].optype;
				if (EAO_ADD==op)
					op = EAO_DEL;
				else if (op==EAO_DEL)
					op = EAO_ADD;
				else {
					changeAffair[m_buf[i].first] = m_buf[i].second;
					continue; 
				}
				AffairDo(m_buf[i].first, m_buf[i].second, op);
			}
			for (typename changedOperates_map::iterator it = changeAffair.begin(); it !=changeAffair.end(); ++it) {
				AffairDo(it->first, it->second, EAO_MODIFIED);
			}
			m_enable = 1 ;
		}
		m_affair_stat = 0 ;
		//m_num = 0;
		m_buf.clear() ;
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
			
			if (m_buf.size() > 0) {
				for(int i= (int)(m_buf.size()-1); i>=0; i--) {
					Undo(m_buf[i].first,m_buf[i].second,m_buf[i].optype) ;
				}
			}
			
			m_enable = 1 ;
		}
		m_affair_stat = 0 ;
		//m_dataChanged = 0;
		//m_num = 0;
		m_buf.clear() ;
	}
	virtual void Undo(const TIndex &index, const  TValue &old_val, int optype)
	{

	}
	virtual void AffairDo(const TIndex &index, const  TValue &old_val, int optype)
	{

	}
	void Reset() 
	{
		m_buf.clear() ;
		//m_num = 0;
		m_affair_stat = 0;
		m_nCount = 0 ;
	}
	void AffairAdd(const TIndex &index,const TValue &old_val)
	{
		if (m_affair_stat)	{			
			AffairSet(index, old_val ,EAO_DEL) ;
		}
		else {
			AffairDo(index, old_val, EAO_ADD);
			m_dataChanged = 1;
		}
	}
	void AffairDel(const TIndex &index,const TValue &old_val)
	{
		if (m_affair_stat)	{			
			AffairSet(index, old_val ,EAO_ADD) ;
		}
		else {
			AffairDo(index, old_val, EAO_DEL);
			m_dataChanged = 1;
		}
	}
	void AffairModify(const TIndex &index,const  TValue &old_val)
	{
		if (m_affair_stat)	{
			AffairSet(index, old_val ,EAO_MODIFIED) ;
		}
		else {
			AffairDo(index, old_val, EAO_MODIFIED);
			m_dataChanged = 1;
		}
	}
	void AffairSet(const TIndex &index,const  TValue &val ,int affair_op) 
	{
		if (m_affair_stat)	{
			back_op op ;
			op.first = index ;
			op.second = val ;
			op.optype = affair_op ;
			m_buf.push_back(op) ;

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


	bool SetCommitNtf(bool bflag)
	{
		bool ret = m_commitNtf ? true : false;
		if (m_affair_stat){
			return ret;
		}

		m_commitNtf = bflag ? 1 : 0;
		return ret;
	}

	void EnableAll()
	{
		m_enable =1 ;
		m_notify =1 ;
		m_syncdb =1 ;
		m_commitNtf = 1;
	}
	void DisableAll()
	{
		m_enable =0 ;
		m_notify =0 ;
		m_syncdb =0 ;
		m_commitNtf = 0;
		
	}

	bool CheckInAffair() 
	{
		return m_enable && m_affair_stat ;
	}
	bool CheckChanged()
	{
		return m_dataChanged ? true : false;
	}
	void SetDataChanged() {
		m_dataChanged = 1;
	}
	void ClearDataChange() {
		m_dataChanged = 0;
	}
	bool CheckSyncDB() {return m_syncdb?true :false; }
	bool CheckNotify() {return m_notify?true :false; }

	typedef std::vector<back_op> affair_vct;
	
	void FetchAffairs(affair_vct &affairVects)
	{
		affairVects = m_buf;
	}
protected:

	int m_nCount;           // 提交计数, 解决多次begin, 多次commit的问题.(这是暂行方案)
	//NDUINT32 m_num:16 ;
	NDUINT32 m_affair_stat:1 ;   //0 not set 1 begin 
	NDUINT32 m_enable:1 ;		//是否打开事务
	NDUINT32 m_notify:1 ;
	NDUINT32 m_syncdb:1 ;
	NDUINT32 m_commitNtf : 1;  //callback when commit 
	NDUINT32 m_dataChanged : 1;  //data change 
	//back_op m_buf[number] ;
	affair_vct m_buf ;
};


#endif
