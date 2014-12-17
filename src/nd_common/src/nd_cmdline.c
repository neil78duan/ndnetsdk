//
//  nd_cmdline.c
//  ndMacStatic
//
//  Created by duanxiuyun on 14-12-16.
//  Copyright (c) 2014年 duanxiuyun. All rights reserved.
//


#include "nd_common/nd_common.h"
#include "nd_common/nd_cmdline.h"



int _command_line(struct nd_cmdline_root *root, int argc,  const char *argv[])
{
	int i = 0 ;
	int ret = -1;
	
	for (i=0; i< root->number; ++i) {
		if (ndstricmp(root->entries[i].name, (char*)argv[0]) == 0) {
			ret = root->entries[i].entry(root, argc, argv) ;
			break ;
		}
	}
		
	return  ret;
}


int _parse_input_and_run(struct nd_cmdline_root *root,char *input_text) 
{
	//int i ;
	int argc = 0 ;
	char *p = ndstr_first_valid(input_text) ;
	char commands_buf[ND_COMMAND_LINE_NUMBER][ND_COMMAND_LINE_SIZE] ; 
	char *argv[ND_COMMAND_LINE_NUMBER] ;
	
	if (!p) {		
		//fprintf(stderr, "bad command: %s\n", input_text ) ;
		return -1;
	}
	for (int i=0; i<ND_COMMAND_LINE_NUMBER; i++) {
		argv[i] = commands_buf[i] ;
		commands_buf[i][0] = 0 ;
	}
	argc = ndstr_parse_command(p, argv, ND_COMMAND_LINE_SIZE, ND_COMMAND_LINE_NUMBER) ;
	if (argc == 0) {
		return -1;
	}
	
//	for (i=0; i<argc; i++) {
//		fprintf(stdout, "\t %s\n", argv[i]) ;
//	}
	
	return _command_line(root, argc, argv) ;
	
}


int nd_run_cmdline(struct nd_cmdline_root *root, int argc, const char *argv[] ) 
{
	int ret=0 ;
	char buf[1024] ;
	
	if (root->init_func) {
		if(-1==root->init_func(root, argc, argv) ) {
			nd_logerror("init command lient error ") ;
			return -1 ;
		}
	}
	
	
	fprintf(stdout, "%s>", root->tips ) ;
	fflush(stdout) ;
	while (root->exit_stat==0) {
		if (root->update_func) {
			ret = root->update_func(root, 0, NULL) ;
			if (ret > 0) {				
				fprintf(stdout, "\n%s>", root->tips ) ;
				fflush(stdout) ;	
			}
			
			if(!kbhit()) {
				nd_sleep(50) ;
				continue ;
			}
		}
		
		memset(buf, 0, sizeof(buf)) ;		
		if(fgets( buf, sizeof(buf), stdin ) ) {
			ret = _parse_input_and_run(root, buf) ;
			if(ret == -1) {
				fprintf(stderr, "bad command: %s\n", buf ) ;
			}
			if (root->exit_stat) {
				break ;
			}			
		}
		
		fprintf(stdout, "%s>", root->tips ) ;
		fflush(stdout) ;	
			
	}
	return 0;
}


int _check_is_run_helper(int argc, const char *argv[], const char *help_tips)
{
	for (int i=1; i<argc; i++) {
		if ( ndstricmp((char*)argv[i], (char*)"-h")==0 || ndstricmp((char*)argv[i], (char*) "--help")==0) {
			fprintf(stdout, "\t%s\n", help_tips) ;
			return 0;
		}
	}
	return -1;
}

int nd_cmdline_help(struct nd_cmdline_root *root, int argc, const char *argv[])
{
	int i ;
	const char *myargv[2] ;
	myargv[1] = "-h" ;
	
	ND_CMDLINE_CHECK_SHOW_HELP(argc, argv, "help") ;
	
	for (i=0; i< root->number; ++i) {
		myargv[0] = root->entries[i].name ;
		root->entries[i].entry(root, 2, myargv) ;
	}
	return 0;
}
