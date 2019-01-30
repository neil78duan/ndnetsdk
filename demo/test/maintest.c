/* file maintest.c
 * test module!
 * neil
 * 2007-9-28
 */


#include "nd_common/nd_common.h"

#if defined(ND_COMPILE_AS_DLL)
#if defined(ND_DEBUG)
#pragma comment(lib,"nd_common_dbg.lib")
#pragma comment(lib,"nd_net_dbg.lib")
#pragma comment(lib,"nd_crypt_dbg.lib")
#pragma comment(lib,"nd_srvcore_dbg.lib")
#else
#pragma comment(lib,"nd_common.lib")
#pragma comment(lib,"nd_net.lib")
#pragma comment(lib,"nd_crypt.lib")
#pragma comment(lib,"nd_srvcore.lib")
#endif

#else
#if defined(ND_DEBUG)
#pragma comment(lib,"srv_libs_dbg.lib")
#else
#pragma comment(lib,"srv_libs.lib")
#endif

#endif

#pragma comment(lib, "Ws2_32.lib")

/* file : maintest.c
 * test entry
 * 2007-10
 * author : neil
 */

#define TEST_ENTRY(name)						\
    extern int name();                          \
    fprintf(stderr, "start " #name " test!\n") ;			\
    if(name () ) {fprintf(stderr, "test " #name " error!\n"); return -1;}\
    else fprintf(stderr, "test " #name " OK!\n")

void inst_init()
{
    // #if defined( ND_USE_GPERF)
    // 		__tcmalloc() ;
    // #endif
    nd_common_init() ;
    nd_net_init() ;
    nd_srvcore_init() ;

    //nd_log_screen("init common lib end\n") ;
}

void inst_deinit()
{
    nd_srvcore_destroy() ;
    nd_net_destroy() ;

    nd_common_release() ;
    //nd_log_screen("RELEASE common lib end\n") ;
}

int run_test ()
{
    TEST_ENTRY(test_atomic);
    //TEST_ENTRY(mutex_test) ;
    //TEST_ENTRY(test_alloc) ;
    //TEST_ENTRY(crypt_test) ;
    //TEST_ENTRY(thmsg_test) ;
    //TEST_ENTRY(thsrv_test) ;
    return 0;
}
int main()
{
    inst_init() ;

    run_test () ;
    inst_deinit() ;
    printf("press any key to continue!\n") ;
    //getch() ;
    exit(0) ;
}

