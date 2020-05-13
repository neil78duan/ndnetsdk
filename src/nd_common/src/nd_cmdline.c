//
//  nd_cmdline.c
//  ndMacStatic
//
//  Created by duanxiuyun on 14-12-16.
//  Copyright (c) 2014 duanxiuyun. All rights reserved.
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
	int i ;
	int argc = 0 ;
	const char *p = ndstr_first_valid(input_text) ;
	char commands_buf[ND_COMMAND_LINE_NUMBER][ND_COMMAND_LINE_SIZE] ; 
	char *argv[ND_COMMAND_LINE_NUMBER] ;
	
	if (!p) {		
		//ndfprintf(stderr, "bad command: %s\n", input_text ) ;
		return -1;
	}
	for (i=0; i<ND_COMMAND_LINE_NUMBER; i++) {
		argv[i] = commands_buf[i] ;
		commands_buf[i][0] = 0 ;
	}
	argc = ndstr_parse_command(p, argv, ND_COMMAND_LINE_SIZE, ND_COMMAND_LINE_NUMBER) ;
	if (argc == 0) {
		return -1;
	}
	
//	for (i=0; i<argc; i++) {
//		ndfprintf(stdout, "\t %s\n", argv[i]) ;
//	}
	
	return _command_line(root, argc, (const char**)argv) ;
	
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
	
	
	ndfprintf(stdout, "%s>", root->tips ) ;
	fflush(stdout) ;
	while (root->exit_stat==0) {
		int read_flag = 0;
		if (root->update_func) {
			ret = root->update_func(root, 0, NULL) ;
			if (ret > 0) {				
				//ndfprintf(stdout, "update ret =%d\n%s>",ret, root->tips ) ;
				fflush(stdout) ;	
			}
			
			if(!kbhit()) {
				nd_sleep(50) ;
				continue ;
			}
		}
		
		memset(buf, 0, sizeof(buf)) ;	
		if (root->next_cmd) {
			ndstrncpy(buf, root->next_cmd, sizeof(buf)) ;
			read_flag = 1 ;
			free(root->next_cmd) ;
			root->next_cmd = 0;
		}
		else if(fgets( buf, sizeof(buf), stdin ) ) {
			read_flag = 1 ;
		}
		
		if (read_flag) {
			ret = _parse_input_and_run(root, buf) ;
			if(ret == -1) {
				ndfprintf(stderr, "bad command: %s\n", buf ) ;
			}
			root->last_retval =ret ;
			if (root->exit_stat) {
				break ;
			}
			
			ndfprintf(stdout, ">%s>", root->tips ) ;
			fflush(stdout) ;	
		}
		read_flag = 0 ;
			
	}
	return 0;
}


int nd_cmd_push_next(struct nd_cmdline_root *root, const char *next_cmd_text)
{
	int size = 0;
	if (!next_cmd_text || next_cmd_text[0]) {
		return -1;
	}
	
	size =(int)ndstrlen(next_cmd_text) ;
	if (root->next_cmd) {
		free(root->next_cmd) ;
		root->next_cmd = 0 ;
	}
	root->next_cmd = malloc(size + 1) ;
	if (root->next_cmd) {
		ndstrncpy(root->next_cmd, next_cmd_text, size) ;
	}
	return 0;
}

int _check_is_run_helper(int argc, const char *argv[], const char *help_tips)
{
	int i ;
	for (i=1; i<argc; i++) {
		if ( ndstricmp((char*)argv[i], (char*)"-h")==0 || ndstricmp((char*)argv[i], (char*) "--help")==0) {
			ndfprintf(stdout, "\t%s\n", help_tips) ;
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

int nd_cmdline_check_and_eixt(struct nd_cmdline_root *root, int argc, const char *argv[])
{
	ND_CMDLINE_CHECK_SHOW_HELP(argc, argv, "checkexit") ;
	if (root->last_retval) {
		exit(root->last_retval) ;
	}
	return 0;
}
