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
#include "ndapplib/nd_objfactory.h"
#include "ndstl/nd_new.h"

#ifdef WIN32
	#include "Psapi.h"
#include <crtdbg.h>
#endif


typedef int (*exit_app_func)(int flag) ;
ND_APPLIB_API int nd_end_server(int force);

ND_APPLIB_API int system_signals_init() ;
ND_APPLIB_API int wait_services() ;
ND_APPLIB_API void exit_instance(int flag) ;
#ifdef ND_UNIX
ND_APPLIB_API int nd_wait_terminate_signals();
ND_APPLIB_API int nd_signals_init();
#else
ND_APPLIB_API BOOL WINAPI winclose_entry(DWORD dwevent);
#endif //_WINDOWS

ND_APPLIB_API void _error_exit(const char *file, int line,const char *stm,...) ;
ND_APPLIB_API void nd_set_exit_callback(exit_app_func func) ;
ND_APPLIB_API void nd_instance_exit(int flag);

ND_APPLIB_API void nd_sys_exit(int exitcode) ;

#define ND_CUR_VERSION 0x1
class NDListener ;
class NDConnector;
class NDSafeListener;

extern int create_stl_allocator();
extern  void destroy_stl_allocator();



template< class T>
class NDAppInstanCreator
{
public:
	typedef T inst_type;
	NDAppInstanCreator()
	{
		nd_common_init();
		nd_net_init();
		nd_srvcore_init();

		nd_logmsg("init common lib end\n");

		__inst = new T();
	}

	~NDAppInstanCreator()
	{
		delete __inst;
		nd_srvcore_destroy();
		nd_net_destroy();

		nd_common_release_ex(1);
		nd_logmsg("RELEASE common lib end\n");
	}

	T* getInstant() {
		return __inst;
	}
protected:
	inst_type *__inst;
};



class  ND_COMMON_CLASS NDInstanceBase : public NDObject, public NDAlarm
{
public :
	int Create(int argc, const char *argv[]) ;
	int CreateEx(int argc, ...);
	void Destroy(int flag) ;
	virtual int Close(int flag=0) ;
	virtual int Open(int session_size=0) ;

    int WaitServer();
	
	NDListener *GetDeftListener() {return pListen ;	}	//get default listener
	
	NDInstanceBase() ;
	virtual ~NDInstanceBase() ;

	//int CreateTimer(int tmval=100);
	int AddSysTimer(nd_timer_entry tmfunc, void *param, ndtime_t interval ) ;
	void DelSysTimer(int timerid) ;
    
    int EnableAlarm(bool bEnable =true) ;
    
	server_config *GetInstConfig() {return &m_config;}
	bool CheckReliableHost(ndip_t& peerip) ;
	bool CheckReliable(NDSession *psession) ;
	bool CheckReliableConn(nd_handle hsession);
	virtual bool CheckIsDeveVer() ;
	int GetVersionID() ;
	const char *GetVersionDesc() ;
	bool CheckNormalExit() {return m_bNormalExit==0 ? true : false ;}
	void SetExitCode(int exitcode = 0) {m_bNormalExit = exitcode ;}
	void trytoDaemon() ;

	const char *Getcfgfile() { return config_file; }
protected :
	int _create();

	int _openConnector(const char *name, NDConnector *inconnect);
	int connectServer(const char *connectorName, NDConnector &connector, NDOStreamMsg *firstMsg=NULL);
	bool createConnect(const char*name, NDConnector &connector);
	connect_config *getConnectorInfo(const char *name) ;
	int ReadConfig(const char *configname) ;		//read config from file
	virtual NDListener*ConstructListener() ;
	virtual void DestructListener() ;
	virtual void OnListenerCreate() {} ;
	virtual size_t getSessionAllocSize() { return 0; }
	virtual	void onConnecteServer(NDConnector &connector) {}
	
	NDListener *pListen ;
	
	//nd_handle m_objhandle ;
	int tminterval;
	int m_alarm_id;
	server_config m_config ;
	const char *m_config_name ;
	const char *config_file ;
	int m_un_develop:1 ;
	int m_bNormalExit:1 ;
	int m_bDaemon:1 ;
	int m_bCreated : 1;
public:
	void StartStaticsMem() ;
	void EndStaticsMem() ;

	void StartStaticsMem2();
	void EndStaticsMem2();

#ifdef _MSC_VER
	_PROCESS_MEMORY_COUNTERS   m_Stat;
#ifdef ND_DEBUG
	_CrtMemState m_s1;
#endif
#endif
};

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

	size_t getSessionAllocSize() { return sizeof(TSession); }
	TListener* GetMyListener() {return (TListener*)pListen;}
};

typedef nd_instance<NDSession, NDListener> NDInstance ;
typedef nd_instance<NDSession, NDSafeListener> NDSafeInstance ;

ND_APPLIB_API NDInstanceBase *nd_get_appinst();
#define getbase_inst nd_get_appinst

#ifdef ND_USE_VLD
#include "vld/vld.h"
#define INIT_MM_LEAK_REPORT(filename) VLDSetReportOptions(VLD_OPT_REPORT_TO_FILE, filename);
#else 
#define INIT_MM_LEAK_REPORT 
#endif

#endif
