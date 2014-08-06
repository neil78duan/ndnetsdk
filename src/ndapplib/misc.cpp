/*
 * file misc.cpp
 *
 * create by duan 
 * 2010/12/15 17:45:41
 */

//#include "nd_common/nd_common.h"
#include <stdarg.h>
#define PG_IMPLEMENTION_TIME
#include "ndapplib/applib.h"

#if defined(PG_INTERNAL_DEVELOP)


time_t app_inst_delta = 0;

void app_inst_set_hm(int _h, int _m)
{
	app_inst_delta += _h * 3600 + _m * 60 ;
}

time_t app_inst_time(time_t* _t)
{
	//porting to linux 
	time_t cur = ::time(NULL);
	cur += app_inst_delta ;
	if (_t){
		*_t = cur ;
	}
	return cur;
}

#endif



void destroy_stl_allocator() ;
static exit_app_func user_exit_entry ;

void nd_set_exit(exit_app_func func) 
{
	user_exit_entry = func ;
}

void exit_instance(int flag)
{
	NDInstanceBase *pib = getbase_inst() ;
	if (pib){
		pib->SetExitCode(flag) ;
	}

	if (user_exit_entry){

		user_exit_entry(flag) ;
	}
}

int nd_end_server(int force)
{
	ENTER_FUNC() ;
	nd_log_screen("end server\n") ;
	nd_host_eixt() ;

	nd_thsrv_release_all() ;

	LEAVE_FUNC() ;
	CALLSTACK_DESTROY() ;
	//destroy_module()  ;
#ifdef _MSC_VER
	//_CrtDumpMemoryLeaks();
#endif
	exit(force) ;
	return 0;
}

int wait_services()
{
#if !defined(ND_UNIX) 
	int ch;
	while( !nd_host_check_exit() ){
		if(kbhit()) {
			ch = getch() ;
			if(ND_ESC==ch){
				printf_dbg("you are hit ESC, program eixt\n") ;
				break ;
			}
		}
		else {
			nd_sleep(200) ;
		}
	}
#else

#ifdef     HANDLE_UNIX_SIGNAL

	while(!nd_host_check_exit() ){
		if(-1==wait_signal_entry() ) {
			printf_dbg("exit from wait signal!\n") ;
			break ;
		}
	}
	unblock_signal() ;
#else 
	int ch;
#ifndef DEBUG_WITH_GDB
	installed_sig_handle();
#endif
	while( !nd_host_check_exit() ){
		if(kbhit()) {
			ch = getch() ;
			if(ND_ESC==ch){
				printf_dbg("you are hit ESC, program eixt\n") ;
				break ;
			}
		}
		else {
			nd_sleep(2000*10) ;
		}
	}
#endif
#endif
	NDInstanceBase *pib = getbase_inst() ;
	if (pib){
		pib->SetExitCode(0) ;
	}
	return 0;
}

#ifndef ND_UNIX
BOOL WINAPI winclose_entry(DWORD dwevent)
{
	BOOL ret  = TRUE ;
	switch (dwevent) {

	case CTRL_C_EVENT:		//ctrl + c
	case CTRL_BREAK_EVENT:  //ctrl + break 
		break ;
	case CTRL_CLOSE_EVENT:	// program being closed
		nd_logmsg("program being closed\n") ;
		exit_instance(0) ;
		break ;
	case CTRL_LOGOFF_EVENT:		//windows user logout 
		nd_logmsg("windows user logout \n") ;
		//nd_end_server(0) ;
		exit_instance(0) ;
		break ;
	case CTRL_SHUTDOWN_EVENT:	//windows shut down
		nd_logmsg("windows shutdown \n") ;
		//end_server(0) ;
		exit_instance(0) ;
		break ;
	default :
		ret = FALSE ;
		break ;
	}
	return ret ;
}
#endif

void _error_exit(const char * file, int line, const char *stm,...)
{
	char buf[1024*4] ;
	char *p = buf;
	va_list arg;
	int done;

	buf[0] = 0 ;
	done = snprintf(buf,1024,"%s:%d ", file, line) ;
	p = buf + done ;

	va_start (arg, stm);
	done = vsnprintf (p, sizeof(buf),stm, arg);
	va_end (arg);

	fprintf(stderr, buf) ;	
	printf("press ANY key to continue\n") ;
	getch() ;
	exit(1) ;
}
