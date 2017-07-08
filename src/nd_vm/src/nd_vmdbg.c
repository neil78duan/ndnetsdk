/* file nd_vmdbg.c
 * define nd virtual machine's debuger tool
 *
 * 2008-6-7
 * all right reserved by neil 
 */

#include "nd_vm/nd_vm.h"
#include "nd_common/nd_str.h"


//size_t vm_parse_expression(char *textbuf, char *code_buf) ;

void run_debuger(struct vm_cpu *vm) ;


int replace_func(const char *input, char *buf, int size,void *userData) 
{
	if(ndstricmp(input, "HP")==0) {
		strncpy(buf, "[1]", size) ;
		return 0 ;
	}
	return -1;
}


enum dbg_instruction {
	DBG_NOP = 0 ,
	DBG_PRINT,
	DBG_HELP,
	DBG_EXP,
	DBG_QUIT,
	DBG_CMD_NUM 
};

char *__dbg_cmd[]  = {
	"NOP" ,
	"PRINT" ,
	"HELP" ,
	"EXP",
	"QUIT"
} ;
extern char *__op[];

void show_help() 
{
	int i ; 
	fprintf(stdout, "USAGE: debug command \n" ) ;
	for (i=0; i<DBG_CMD_NUM; i++) {
		fprintf(stdout,"\t%s\n", __dbg_cmd[i]) ;
	}
	
	fprintf(stdout, "\nUSAGE: vm command \n" ) ;
	for (i=0; i<EOP_NUMBERS; i++) {
		fprintf(stdout,"\t%s\n", __op[i]) ;
	}
}

void run_print(const char *src, struct vm_cpu *vm)
{
	char buf[128] ;
	const char *addr = ndstr_first_valid(src) ; 
	if (!addr || !addr[0]){
		return ;
	}

	addr = ndstr_parse_word_n(addr, buf, 128);
	if(!addr) 
		return ;
	if(0==ndstricmp(buf, "reg")) {
		fprintf(stdout, " reg1 = %f\n  reg2 = %f \n", vm->reg1, vm->reg2) ;
	}
	else if(0==ndstricmp(buf, "mem")) {
		int mm_addr = ndstr_atoi_hex(addr);
		vm_value * pval = _get_memory(vm ,  (vm_adddress )mm_addr) ;
		if(pval) {
			fprintf(stdout, " memory[%d] = %f\n", mm_addr, *pval) ;
		}
		else {
			fprintf(stdout, " bad memory address\n") ;
		}
	}
	else if(0==ndstricmp(buf, "stack")) {
		int i = 0 ;
		vm_value * pval = vm->stack ;
		while(pval < vm->sp) {
			fprintf(stdout, " stack[%d] = %f\n", i, *pval) ;
			++pval ;
			++i ;
		}
		if(0==i) {
			fprintf(stdout, " empty stack \n", i, *pval) ;
		}
		else 
			fprintf(stdout, " sp =%d \n", i ) ;
	}
	else {
		fprintf(stdout, " usage: print reg/mem/stack n \n") ;
	}
	
}

int get_dbg_inst(char *op)
{
	int i ;
	for(i=0; i<DBG_CMD_NUM; i++) {
		if(0==ndstricmp(op, __dbg_cmd[i] )) {
			return (vm_ins) i ;
		}
	}
	return -1 ;
}

int try_run_dbgcmd(char *linebuf, struct vm_cpu *vm)
{
	int cmd ;
	char buf[128] ;
	const char *addr = ndstr_first_valid(linebuf) ;
		
	buf[0] = 0 ;
	addr = ndstr_parse_word_n(addr, buf, 128) ;
	if(!addr) {
		return -1 ;
	}

	cmd =  get_dbg_inst(buf) ;
	if(-1==cmd) {
		return -1 ;
	}
		
	switch(cmd) {
	case DBG_NOP:
		fprintf(stdout, " NOP\t\n", vm->reg1) ;
		break ;
	case DBG_PRINT:
		//fprintf(stdout, " reg = %f\n", vm->reg1) ;
		run_print(addr, vm) ;
		break ;
	case DBG_HELP:
		//fprintf(stdout, " USAGE: \n NOP \n PRINT \n HELP \n QUIT \n") ;
		show_help() ;
		break ;
	case DBG_EXP:
		{
			char buf[4096] ;
			size_t bin_size = vm_parse_expression(addr, buf, sizeof(buf), (vm_param_replace_func)replace_func, NULL);
			if(bin_size>0) {
				//run code
				if(-1==vm_run_cmd(vm,buf, bin_size)  ) {
					fprintf(stdout , " bad exp!\n") ;
				}
				else {
					fprintf(stdout, "REG1= %f\n" , vm->reg1) ; 
				}
			}
			break ;
		}
	case DBG_QUIT:
		exit(0) ;
		break ;

	}//	end switch
	return 0 ;
}

static vm_value __memory[VM_DFT_MMSIZE] ;
static struct vm_cpu __dbg_vm ;

vm_value * get_mm(vm_adddress index,struct vm_cpu*vcpu) 
{
	//static 
	return &(__memory[index] );
}

void vm_start_debug() 
{
	vm_machine_init(&__dbg_vm, 0, 0) ;
	vm_set_mmfunc(&__dbg_vm,get_mm, VM_DFT_MMSIZE);
	vm_set_echo_ins(&__dbg_vm,1) ;
	vm_set_echo_result(&__dbg_vm,0) ;

	run_debuger(&__dbg_vm);

}

void run_debuger(struct vm_cpu *vm)
{
	const char *p ;
	int ret ;
	struct vm_instruction_node node ;
	char buf[128] ;

	for (;;)
	{
		fprintf(stdout, "nd vm>" ) ;

		memset(buf, 0, sizeof(buf)) ;

		if(fgets( buf, 128, stdin ) ) {
			p = ndstr_first_valid(buf) ;
			if(!p) 
				continue ;
			if(0==try_run_dbgcmd(buf, vm)) {
				continue ;
			}
			ret = vm_compiler_line(buf , &node) ;
			if(ret > 0 ) {
				if(-1!=vm_run_insnode(&node, vm) ) {
					//vm_output_asm(&node, stdout, MEDIUM_FILE);
				}
			}
			else if(ret == -1) {
				fprintf(stderr, "bad command: %s\n", buf ) ;
			}
			
		}
		else {
			break ;
		}
	}
	
}

void vm_end_debug()
{
	exit(0) ;
}

