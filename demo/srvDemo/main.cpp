/* file : gatemain.cpp
 * gate of game server
 *
 * 2009-4-30 23:55
 */

#if defined(ND_DEBUG)
#pragma comment(lib,"ndapplib_dbg.lib")
#pragma comment(lib,"srv_libs_dbg.lib")
#else
#pragma comment(lib,"ndapplib.lib")
#pragma comment(lib,"srv_libs.lib")
#endif

#include "ndapplib/applib.h"
#include "nd_msg.h"


int InitMsgHandler(NDInstanceBase &inst) ;

class TestInst : public NDInstance
{
public:
	
	void OnListenerCreate() {
		GetDeftListener()->AttachPort(9000);
	} ;
	
};

TestInst g_instance_srv ;

NDInstanceBase &get_instance()
{
    return g_instance_srv ;
}

int main(int argc, const char *argv[])
{
    NDInstance &inst = g_instance_srv ;
    
    if(-1==inst.Start(argc, argv) ) {
        printf("start server error\n Press ANY KEY to continue\n") ;
        getch();
        exit(1) ;
    }
    
    ND_TRACE_FUNC() ;
    
    InitMsgHandler(inst) ;
    
    /*inst.GetDeftListener()->SetEmptyConnTimeout(30) ;// timeout 30s
     nd_handle h = inst.GetDeftListener()->GetHandle() ;
     nd_net_set_unregmsg_handler(h,1) ;
     nd_net_set_unauthorize_handler(h,1) ;
     */
    inst.WaitServer() ;
    
    inst.End(0) ;
    
    printf_dbg("program exit from main\n Press ANY KEY to continue\n") ;
    getch();
    return 0;
}

NDUINT32 getversion()
{
    //return (NDUINT32)nd_get_certificate_version() ;
    return 1 ;
}


MSG_ENTRY_INSTANCE(check_version_handler)
{
    ND_TRACE_FUNC() ;
    NDOStreamMsg omsg(ND_MAIN_ID_SYS,ND_MSG_SYS_GETVERSION) ;
    NDUINT16 ver = 1 ;
    
    omsg.Write(ver) ;
    //ND_MSG_SEND( nethandle, omsg.GetMsgAddr(), h_listen) ;
	netconn->SendMsg(omsg) ;
    return 0;
}


MSG_ENTRY_INSTANCE(echo_handler)
{
    //ND_MSG_SEND( nethandle,(nd_usermsghdr_t *)msg, h_listen) ;
	netconn->SendMsg(msg) ;
    return 0 ;
}

MSG_ENTRY_INSTANCE(broadcast_handler)
{
    //ND_BROAD_CAST( h_listen,(nd_usermsghdr_t *)msg);
    
    NDIStreamMsg inmsg(msg) ;
    NDUINT8 buf[1024] ;
    buf[0] = 0 ;
    inmsg.Read(buf,sizeof(buf)) ;
    printf("recv (%d,%d) : %s \n",inmsg.MsgMaxid(), inmsg.MsgMinid(), buf ) ;

    
    return 0 ;
}

MSG_ENTRY_INSTANCE(loging_hander)
{
    size_t len ;
    NDIStreamMsg inmsg(msg) ;
    NDUINT8 pwd[32];
    NDUINT8 name[100] ;
    
    len = inmsg.Read(name, sizeof(name)) ;
    
    if (len <=0) {
        return -2;
    }
    
    len = inmsg.Read(pwd, sizeof(pwd)) ;
    
    if (len <=0) {
        return -2;
    }
    
    return 0 ;
}



#define MSG_INSTALLER_SYS(f, sub_msg) \
inst.GetDeftListener()->InstallMsgFunc((nd_conn_msg_entry)f, ND_MAIN_ID_SYS, sub_msg,EPL_CONNECT)


#define MSG_INSTALLER(f,main_msg, sub_msg) \
inst.GetDeftListener()->InstallMsgFunc((nd_conn_msg_entry)f, main_msg, sub_msg,EPL_CONNECT)
int InitMsgHandler(NDInstanceBase &inst)
{
    MSG_INSTALLER_SYS(check_version_handler,ND_MSG_SYS_GETVERSION) ;
    MSG_INSTALLER_SYS(echo_handler,ND_MSG_SYS_ECHO) ;
    MSG_INSTALLER_SYS(broadcast_handler,ND_MSG_SYS_BROADCAST) ;
    
    MSG_INSTALLER(loging_hander,ND_MAIN_ID_LOGIN, EFRMSG_LOGIN);
    return 0 ;
}

		
