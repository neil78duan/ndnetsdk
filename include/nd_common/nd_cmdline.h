//
//  nd_cmdline.h
//  ndMacStatic
//
//  Created by duanxiuyun on 14-12-16.
//  Copyright (c) 2014 duanxiuyun. All rights reserved.
//

#ifndef ndMacStatic_nd_cmdline_h
#define ndMacStatic_nd_cmdline_h


#define ND_COMMAND_LINE_SIZE 64
#define ND_COMMAND_LINE_NUMBER 16


struct nd_cmdline_root ;
typedef int (*nd_cmdline_entry)(struct nd_cmdline_root *root, int argc, const char *argv[]) ;


struct nd_cmdline_node 
{
	char name[ND_COMMAND_LINE_SIZE] ;
	nd_cmdline_entry entry ;	
};


struct nd_cmdline_root
{
	int exit_stat ;	
	int last_retval;
	char tips[ND_COMMAND_LINE_SIZE] ;	
	void *userdata ;
	nd_cmdline_entry update_func ; //return 0 nothing todo , -1 error ,else need renew screen
	nd_cmdline_entry init_func ;
	int number ;
	struct  nd_cmdline_node *entries ;
	char *next_cmd ;
};

ND_COMMON_API int nd_run_cmdline(struct nd_cmdline_root *root, int argc, const char *argv[] ) ;
ND_COMMON_API int _check_is_run_helper(int argc, const char *argv[], const char *help_tips);
ND_COMMON_API int nd_cmdline_help(struct nd_cmdline_root *root, int argc, const char *argv[]);

ND_COMMON_API int nd_cmd_push_next(struct nd_cmdline_root *root, const char *next_cmd_text);
ND_COMMON_API int nd_cmdline_check_and_eixt(struct nd_cmdline_root *root, int argc, const char *argv[]) ;

static __INLINE__ void nd_cmdline_quit(struct nd_cmdline_root *root) 
{
	root->exit_stat = 1 ;
}

#define ND_CMDLINE_FUNC_INSTANCE(_name) int _name(struct nd_cmdline_root *root, int argc, const char *argv[] )

#define ND_CMDLINE_CHECK_SHOW_HELP(_argc, _argv, _help_text) do {\
	if(_check_is_run_helper(_argc, _argv, _help_text) ==0){ \
		return 0 ; \
	} }while(0)

#endif
