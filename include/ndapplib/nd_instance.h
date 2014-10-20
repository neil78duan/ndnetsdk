/* file nd_instance.h
 * header file of class ndCInstance
 * 
 * all right reserved by neil duan
 *
 * 2009-4-24 22:41
 */

#ifndef _ND_INSTANCE_H_
#define _ND_INSTANCE_H_

#include "ndapplib/nd_alarms.h"

#include "ndapplib/nd_listener.h"
#include "ndapplib/nd_msgpacket.h"
#include "ndapplib/nd_session.h"
#include "ndapplib/nd_cmmgr.h"
#include "ndapplib/readcfg.h"
#include "ndstl/nd_new.h"

#ifdef WIN32
	#include "Psapi.h"
#include <crtdbg.h>
#endif

#define ND_CUR_VERSION 0x1
class NDListener ;
class NDConnector;
class NDSafeListener;
//服务器实例


class NDInstanceBase : public NDAlarm
{
public :
	int Create(int argc, const char *argv[]) ;
	void Destroy(int flag) ;
	virtual int Close(int flag=0) ;
	virtual int Open(int session_size) ;

    int WaitServer();
	
	NDListener *GetDeftListener() {return pListen ;	}	//得到默认监听器
	
	NDInstanceBase() ;
	virtual ~NDInstanceBase() ;

	//int CreateTimer(int tmval=100);
	int AddSysTimer(nd_timer_entry tmfunc, void *param, ndtime_t interval ) ;
	void DelSysTimer(int timerid) ;
    
    int EnableAlarm(bool bEnable =true) ;
    
	server_config *GetInstConfig() {return &m_config;}
	bool CheckReliableHost(ndip_t peerip) ;
	bool CheckReliable(NDSession *psession) ;
	bool CheckReliableConn(nd_handle hsession);
	virtual bool CheckIsDeveVer() ;
	bool CheckNormalExit() {return m_bNormalExit==0 ? true : false ;}
	void SetExitCode(int exitcode = 0) {m_bNormalExit = exitcode ;}
    
protected :
    
	int ReadConfig(const char *configname) ;		//read config from file
	virtual NDListener*ConstructListener() ;
	virtual void DestructListener() ;
	
	NDListener *pListen ;
	
	//nd_handle m_objhandle ;
	int tminterval ;
	server_config m_config ;
	const char *m_config_name ;
	const char *config_file ;
	int m_un_develop ;
	int m_bNormalExit ;
    int m_alarm_id ;
public:
	void StartStaticsMem() ;
	void EndStaticsMem() ;

	void StartStaticsMem2();
	void EndStaticsMem2();

#ifdef _WIN32
	_PROCESS_MEMORY_COUNTERS   m_Stat; 
#endif
#ifndef ND_UNIX
#ifdef ND_DEBUG
	_CrtMemState m_s1;
#endif
#endif
};

CPPAPI void nd_instance_exit(int flag);
template < class TSession,class TListener=NDListener>
class nd_instance:public NDInstanceBase
{
public:
	nd_instance() {} ;
	virtual ~nd_instance() {} 
	int Start(int argc, const char *argv[])
	{
		if(-1==Create(argc, argv))
			return -1 ;
		return Open(sizeof(TSession)) ;
	}
    
 	int End(int flag)
 	{
 		NDInstanceBase::Destroy(flag) ;
        
        nd_instance_exit(flag) ;
 
 		return 0;
 	}
    
	TListener*ConstructListener()
	{	
		return new TListener(new nd_fectory<TSession>()) ;	
	}

	void DestructListener() 
	{ 
		if(pListen)	{
			delete pListen;	 pListen = 0 ;
		} 
	}

	const char *Getcfgfile() {return config_file ;}
};

typedef nd_instance<NDSession, NDListener> NDInstance ;
typedef nd_instance<NDSession, NDSafeListener> NDSafeInstance ;

NDInstanceBase *getbase_inst() ;

#ifdef ND_USE_VLD
#include "vld/vld.h"
#define INIT_MM_LEAK_REPORT(filename) VLDSetReportOptions(VLD_OPT_REPORT_TO_FILE, filename);
#else 
#define INIT_MM_LEAK_REPORT 
#endif

#endif
