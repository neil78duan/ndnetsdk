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
extern int name();					\
int main()										\
{											\
	nd_common_init() ;							\
	printf("start " #name " test!\n") ;			\
	if(name () )printf("test " #name " error!\n");			\
	else printf("test " #name " OK!\n") ;			\
	nd_common_release() ;							\
	printf("press any key to continue!\n") ;	\
	getch() ;									\
	exit(0);									\
}

int test_bit()
{
	int a ;
	union{
		struct {
			char a : 3 ;
			char b: 4 ;
		};
		char c ;
	}t ;

	t.c = 0 ;
	t.a = 2 ;
	t.b =1 ;
	a = t.c ;
	printf(" %d\n" , t.c) ;
	return 0 ;
}
TEST_ENTRY(run_test_pool)
//TEST_ENTRY(xml_test)
/*int main() 
{
	while (!nd_key_esc()){
		nd_sleep(100) ;
		ndprintf("please hit!\n") ;	
	}
	ndprintf("you are hitting ESC!\n") ;
	getch() ;
}
*/