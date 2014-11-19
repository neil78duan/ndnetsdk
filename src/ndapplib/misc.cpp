/*
 * file misc.cpp
 *
 * create by duan 
 * 2010/12/15 17:45:41
 */

//#include "nd_common/nd_common.h"
#include <stdarg.h>
#include "ndapplib/applib.h"


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


void nd_sys_exit(int exitcode)
{
    exit(exitcode) ;
}
void destroy_stl_allocator() ;
static exit_app_func user_exit_entry ;

void nd_set_exit_callback(exit_app_func func)
{
	user_exit_entry = func ;
}

void nd_instance_exit(int flag)
{
    ENTER_FUNC() ;
    nd_log_screen("end server\n") ;
    
    if (user_exit_entry){
        user_exit_entry(flag) ;
    }
    
    nd_host_eixt() ;
    nd_thsrv_release_all() ;
    
    LEAVE_FUNC() ;
    CALLSTACK_DESTROY() ;
    //destroy_module()  ;
#ifdef _MSC_VER
    //_CrtDumpMemoryLeaks();
#endif
    exit(flag) ;

}

int wait_key_esc()
{
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
    return 0 ;
}

int wait_services()
{
#ifdef _MSC_VER
    return wait_key_esc() ;
#else
    //wait_key_esc();
    
    while(!nd_host_check_exit() ){
        if(-1==nd_wait_terminate_signals() ) {
            printf_dbg("exit from wait signal!\n") ;
            break ;
        }
    }
    
#endif
    return 0;
}

int system_signals_init()
{
    
#ifdef _MSC_VER
    if (!SetConsoleCtrlHandler((PHANDLER_ROUTINE)winclose_entry,TRUE)) {
        nd_logfatal("install close callback function error\n") ;
    }
#ifdef ND_DEBUG
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    //
    // 	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
    // 	_CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);
    // 	_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
    // 	_CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDOUT);
    // 	_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
    // 	_CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDOUT);
#endif  //debug
    
#else
    return nd_signals_init() ;
#endif
    return 0;
}

#ifdef _MSC_VER
BOOL WINAPI winclose_entry(DWORD dwevent)
{
	BOOL ret  = TRUE ;
	switch (dwevent) {

	case CTRL_C_EVENT:		//ctrl + c
	case CTRL_BREAK_EVENT:  //ctrl + break 
		break ;
	case CTRL_CLOSE_EVENT:	// program being closed
		nd_logmsg("program being closed\n") ;
		nd_instance_exit(0) ;
		break ;
	case CTRL_LOGOFF_EVENT:		//windows user logout 
		nd_logmsg("windows user logout \n") ;
		//nd_end_server(0) ;
		nd_instance_exit(0) ;
		break ;
	case CTRL_SHUTDOWN_EVENT:	//windows shut down
		nd_logmsg("windows shutdown \n") ;
		//end_server(0) ;
		nd_instance_exit(0) ;
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

	fprintf(stderr, "%s",buf) ;	
	//printf("press ANY key to continue\n") ;
	//getch() ;
	exit(1) ;
}

#include "nd_msg.h"
int send_error_ack(nd_handle hconnect, int errcode)
{
	NDOStreamMsg omsg(ND_MAIN_ID_SYS, ND_MSG_SYS_ERROR) ;
	omsg.Write((NDUINT32)errcode) ;


	ND_MSG_SEND( hconnect, omsg.GetMsgAddr(), NULL) ;

	return 0;
}

