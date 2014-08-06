/* file mainserver.c
 * test server function
 * neil duan
 * 2008-6
 */

#ifdef ND_DEBUG
#pragma comment(lib,"nd_app_dbg.lib")
#else 
#pragma comment(lib,"nd_app.lib")
#endif

#include "nd_app/nd_app.h"

int main(int argc, char *argv[])
{
	if(0!=start_server(argc, argv)) {
		exit(1) ;
	}	
	wait_services() ;
	printf_dbg("leave wait server\n") ;
	end_server(0) ;
	printf_dbg("program exit from main\n Press ANY KEY to continue\n") ;
	getch();
	return 0;
}
