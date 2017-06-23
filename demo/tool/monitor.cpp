/* file : monitor.cpp
* monitor program function call stack
*/


#include "nd_common/nd_common.h"

void run_monitor() ;
void exit_mon(int flag);
int run_cmd(char *linebuf, char **run_addr) ;

int main(int argc, char *argv[])
{
	nd_common_init() ;
	if (argc >= 2){
		int i ;
		char *p  ;
		ndchar_t buf[1024] ;
		p = buf ;
		for(i=1; i<argc; i++ ) {
			int n = ndsnprintf(p ,1024-(p-buf), _NDT("%s "), argv[i]) ;
			p += n;
		}

		p = buf ;
		while(run_cmd(p,&p) == 0 ) {
			if(!p)
				break ;
		}
	}

	run_monitor() ;
	exit_mon(0) ;
}

void exit_mon(int flag)
{
	nd_common_release() ;
	CALLSTACK_DESTROY() ;
	exit(flag) ;
}

static char *cmd_list[] = 
{
	"trace" ,
	"untrace" ,
	"dump" ,
	"help" ,
	"quit"
};
enum {
	E_TRACE ,
	E_UNTRACE,
	E_DUMP,
	E_HELP,
	E_EXIT,
	E_CMD_NUM
};

int find_cmd(char *cmd)
{
	int i=0; 
	for(i=0; i<E_CMD_NUM; i++) {
		if(0==ndstricmp(cmd,cmd_list[i]))
			return i ;
	}
	return -1 ;
}

//static FILE *dump_file ;

int run_cmd(char *linebuf, char **run_addr)
{
	int cmd ;
	char buf[128] ;
	const char *addr = ndstr_first_valid(linebuf) ;
	
	if(!addr) {
		return -1 ;
	}

	buf[0] = 0 ;
	addr = ndstr_parse_word_n(addr, buf, 128) ;
	if(!addr) {
		return -1 ;
	}

	cmd =  find_cmd(buf) ;
	if(-1==cmd) {
		return -1 ;
	}
	
	switch(cmd) {
	case E_TRACE:
		addr = ndstr_first_valid(addr) ;
		if (addr) {
			buf[0] = 0 ;
			addr = ndstr_parse_string(addr, buf) ;
			if(!buf[0]) {
				fprintf(stdout, "usage: \n" 
					"\ttrace map_file_name\n" 
					"\tuntrace \n"
					"\tdump [result_out.txt] \n" 
					"\thelp\n" 
					"\tquit\n") ;
				break ;
			}
			nd_callstack_monitor_end() ;
			if(nd_callstack_monitor_init(buf) ) {
				fprintf(stdout, "error in open %s memory_map_file\n", buf) ;
			}
		}
		else {
			fprintf(stdout, "usage: \n" 
				"\ttrace map_file_name\n" 
				"\tuntrace \n"
				"\tdump [result_out.txt] \n" 
				"\thelp\n" 
				"\tquit\n") ;
		}
		break ;
	case E_UNTRACE:
		nd_callstack_monitor_end() ;
		break ;
	case E_DUMP:
		addr = ndstr_first_valid(addr) ;
		if (addr) {
			buf[0] = 0 ;
			addr = ndstr_parse_string(addr, buf) ;
			if(buf[0]) {
				FILE *fp = fopen(buf, "w") ;
				if (fp) {
					nd_callstack_monitor_dump(fprintf, fp);
					fclose(fp) ;
				}
				else {
					fprintf(stdout, "open %s file error\n", buf);
				}
			}
			else {
				nd_callstack_monitor_dump(fprintf, stdout);
			}
		}
		else {
			nd_callstack_monitor_dump(fprintf, stdout);
		}
		break ;
	case E_HELP:
		fprintf(stdout, "usage: \n" 
			"\ttrace map_file_name\n" 
			"\tuntrace \n"
			"\tdump [result_out.txt] \n" 
			"\thelp\n" 
			"\tquit\n") ;
		break ;
	case E_EXIT:
		exit_mon(0) ;
		break ;
	}
	if (run_addr)
		*run_addr = (char*) addr ;
	return 0 ;
}

void run_monitor()
{
	char *p ;
	int ret=0 ;
	char buf[1024] ;

	for (;;)
	{
		fprintf(stdout, "monitor>" ) ;

		memset(buf, 0, sizeof(buf)) ;

		if(fgets( buf, sizeof(buf), stdin ) ) {
			p = (char*)ndstr_first_valid(buf) ;
			if(!p) 
				continue ;
			ret = run_cmd(buf, NULL) ;
			if(ret == -1) {
				fprintf(stderr, "bad command: %s\n", buf ) ;
			}

		}
		else {
			break ;
		}
	}

}