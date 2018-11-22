/* file unix_sig.c
 * handle Linux/Unix OS signal

 * neil duan  2008-8-4
 * version 1.0
 */

#include "nd_common/nd_common.h"
#ifdef ND_UNIX
//#include "nd_app.h"
#include <signal.h>


//extern void nd_instance_exit(int flag) ;

extern void nd_sys_exit(int exitcode) ;
char *sig_desc[] = {
								 " ",
/*#define	SIGHUP		1	*/   " Hangup (POSIX).  ",
/*#define	SIGINT		2	*/   " Interrupt (ANSI).  ",
/*#define	SIGQUIT		3	*/   " Quit (POSIX).  ",
/*#define	SIGILL		4	*/   " Illegal instruction (ANSI).  ",
/*#define	SIGTRAP		5	*/   " Trace trap (POSIX).  ",
/*#define	SIGABRT		6	*/   " Abort (ANSI).  ",
/*#define	SIGBUS		7	*/   " BUS error (4.2 BSD).  ",
/*#define	SIGFPE		8	*/   " Floating-point exception (ANSI).  ",
/*#define	SIGKILL		9	*/   " Kill, unblockable (POSIX).  ",
/*#define	SIGUSR1		10	*/   " User-defined signal 1 (POSIX).  ",
/*#define	SIGSEGV		11	*/   " Segmentation violation (ANSI).  ",
/*#define	SIGUSR2		12	*/   "  User-defined signal 2 (POSIX).  ",
/*#define	SIGPIPE		13	*/   "  Broken pipe (POSIX).  ",
/*#define	SIGALRM		14	*/   "  Alarm clock (POSIX).  ",
/*#define	SIGTERM		15	*/   "  Termination (ANSI).  ",
/*#define	SIGSTKFLT	16	*/   "  Stack fault.  ",
/*#define	SIGCHLD		17	*/   "  Child status has changed (POSIX).or Same as SIGCHLD (System V).    ",
/*#define	SIGCONT		18	*/   "  Continue (POSIX).  ",
/*#define	SIGSTOP		19	*/   "  Stop, unblockable (POSIX).  ",
/*#define	SIGTSTP		20	*/   "  Keyboard stop (POSIX).  ",
/*#define	SIGTTIN		21	*/   "  Background read from tty (POSIX).  ",
/*#define	SIGTTOU		22	*/   "  Background write to tty (POSIX).  ",
/*#define	SIGURG		23	*/   "  Urgent condition on socket (4.2 BSD).  ",
/*#define	SIGXCPU		24	*/   "  CPU limit exceeded (4.2 BSD).  ",
/*#define	SIGXFSZ		25	*/   "  File size limit exceeded (4.2 BSD).  ",
/*#define	SIGVTALRM	26	*/   "  Virtual alarm clock (4.2 BSD).  ",
/*#define	SIGPROF		27	*/   "  Profiling alarm clock (4.2 BSD).  ",
/*#define	SIGWINCH	28	*/   "  Window size change (4.3 BSD, Sun).  ",
/*#define	SIGIO		29	*/   "  I/O now possible (4.2 BSD).  or Pollable event occurred (System V).",
/*#define	SIGPWR		30	*/   "  Power failure restart (System V).  ",
/*#define SIGSYS		31	*/   "  Bad system call.  "
/*#define SIGUNUSED	31 */

};

char *get_signal_desc(int signo)
{
    char *p = "undefined message " ;
    if( signo <= 31 && signo >=0)
        return sig_desc[signo] ;
    else
        return p ;
}

/* if server program received a signal
 * it will run here
 * so I can do some to handle error
 */
void _terminate_server(int signo)
{
	char *msg = get_signal_desc(signo);

	nd_logmsg("sigaction() -> _terminate_server() received signed %s server exit\n",msg);

	nd_instance_exit(0) ;
}

//the signals list, when received program will terminate
static int _s_terminate_sigs[] = {/*SIGHUP,*/SIGINT,SIGQUIT,SIGTERM,SIGTSTP} ;
#define terminate_sigs_num  ND_ELEMENTS_NUM(_s_terminate_sigs)

// ignore signals list
static int _s_ignore_sigs[] = {SIGHUP,SIGALRM,SIGCHLD,SIGIO,SIGCONT,SIGPIPE,SIGVTALRM,SIGUSR1,SIGUSR2} ;
#define ignore_sigs_num      ND_ELEMENTS_NUM(_s_ignore_sigs)

//install terminate signal handler and ignore some signals
int nd_signals_init()
{
    ENTER_FUNC()
    int i ;
    sigset_t blockmask, oldmask  ;
    struct sigaction sact ,oldact;

    sigemptyset(&blockmask);
    sigemptyset(&oldmask);

    sigaddset(&blockmask, SIGINT);
    sigaddset(&blockmask, SIGTERM);

    sact.sa_handler = _terminate_server ;

    sigemptyset(&sact.sa_mask );
    sigaddset(&sact.sa_mask, SIGINT);

    sigprocmask(SIG_BLOCK ,&blockmask, &oldmask );

    //set terminate function
    for(i=0; i<terminate_sigs_num; i++){
        if(-1==sigaction(_s_terminate_sigs[i], &sact, &oldact))	{
            nd_logmsg("Installed %d signal function error!", _s_terminate_sigs[i] );
            //LEAVE_FUNC();
            //return -1 ;
        }
    }

    sact.sa_handler = SIG_IGN ;
    //ignore signals
    for(i=0; i<ignore_sigs_num; i++){
        if(-1==sigaction(_s_ignore_sigs[i], &sact, &oldact))	{
        //if(-1== signal(_s_ignore_sigs[i], SIG_IGN)) {
            nd_logmsg("sigaction() Installed %d signal function error!", _s_terminate_sigs[i] );
            //LEAVE_FUNC();
            //return -1 ;
        }
    }

    sigprocmask(SIG_SETMASK ,&blockmask, NULL );
    LEAVE_FUNC();

    return  0;
}

//wait signal in main thread
int nd_wait_terminate_signals()
{
    ENTER_FUNC()
    int i = 0 , intmask=-1, ret=-1;
    sigset_t blockmask ;

    sigemptyset(&blockmask) ;

    for(i=0; i<terminate_sigs_num; i++){
        sigaddset(&blockmask,_s_terminate_sigs[i]) ;
    }

    ret = sigwait(&blockmask, &intmask) ;
    if (ret == 0) {
        if(intmask <=0 || intmask > 32){
            LEAVE_FUNC();
            return 0;
        }
    }
    else {
        nd_logerror("sigwait() : %s", nd_last_error()) ;
        LEAVE_FUNC();
        nd_sys_exit(1) ;
	}
	if (intmask==SIGINT || intmask==SIGTERM){
		nd_host_set_error(NDERR_HOST_SHUTDOWN) ;
	}
	else {
		nd_host_set_error(NDERR_HOST_CRASH) ;
	}
	nd_logmsg("sigwait() received %d signed %s program would exit\n"  AND  intmask AND get_signal_desc(intmask));

    LEAVE_FUNC();
    return -1;
}

#endif
