/* file nd_instance.h
 * header file of class ndCInstance
 * 
 * all right reserved by neil duan
 *
 * 2009-4-24 22:41
 */

#ifndef _ND_INSTANCE_H_
#define _ND_INSTANCE_H_

#include "nd_appcpp/nd_object.h"
//#include "nd_appcpp/"
#include "nd_app/nd_app.h"
#include "nd_appcpp/nd_listener.h"
#include "nd_appcpp/nd_msgpacket.h"
#include "nd_appcpp/nd_session.h"
#include "nd_appcpp/nd_cmmgr.h"

class NDListener ;
//服务器实例
class NDInstanceSrv : public NDObject
{
public :
	//initialize server
	int EndServer(int force=0);		
	int WaitServer();
	int StartServer(void);
	int InitServer( size_t session_size);

	NDListener *GetDeftListener() {return pListen ;	}	//得到默认监听器
	//initialize application
	static void DestroyApp(void);				
	static int InitApp(int argc, char *argv[]);

	NDInstanceSrv() ;
	virtual ~NDInstanceSrv() ;
public :
	int CreateTimer(int tmval=100);
	static char *config_file ;
	static srv_config srvcfg ;
	int tminterval ;
protected :
	virtual NDListener *CreateListener() ;
	virtual void DestroyListener() ;
	NDListener *pListen ;
#ifdef SINGLE_THREAD_MOD
	ndtimer_t timer_thid ;
#else 
	nd_thsrvid_t timer_thid ;
#endif 
	//nd_handle m_hListen ;
};
#endif
