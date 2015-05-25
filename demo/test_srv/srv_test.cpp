/* file : srv_test.cpp
 * test app of cpp
 *
 * 2009-4-30 23:55
 */


#if defined(ND_DEBUG)
#pragma comment(lib,"nd_app_dbg.lib")
#pragma comment(lib,"nd_appcpp_dbg.lib")
#else
#pragma comment(lib,"nd_appcpp.lib")
#pragma comment(lib,"nd_app.lib")
#endif

#include "nd_appcpp/nd_instance.h"


int main(int argc, char *argv[])
{
	NDInstanceSrv nd_instance ;

	NDInstanceSrv::InitApp(argc, argv) ;

	ND_TRACE_FUNC() ;
	nd_instance.InitServer(sizeof(NDSession));

	nd_instance.StartServer() ;

	nd_instance.WaitServer() ;

	nd_instance.EndServer(0) ;

	NDInstanceSrv::DestroyApp() ;
	/*
	int EndServer(int force=0);		
	int WaitServer();
	int StartServer(void);
	int InitServer(int max_session, size_t session_size);

	//initialize application
	static void DestroyApp(void);				
	static int InitApp(int argc, char *argv[]);
	*/
	/*if(0!=start_server(argc, argv)) {
		exit(1) ;
	}	
	wait_services() ;
	printf_dbg("leave wait server\n") ;
	end_server(0) ;
	*/
	printf_dbg("program exit from main\n Press ANY KEY to continue\n") ;
	getch();
	return 0;
}