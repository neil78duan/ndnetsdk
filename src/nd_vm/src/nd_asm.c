/* file nd_asm.c
 * define nd virtual machine's asm compile
 *
 * 2008-6-6 1:24
 * all right reserved by neil 
 */

#include "nd_vm/nd_vm.h"
#include "nd_common/nd_str.h"

//#define _CRT_SECURE_NO_WARNINGS 1

char *__op[] = {
	"NOP",
	"MOV" ,
	"ADD",
	"SUB",
	"MUL",
	"DIV" ,
	"PUSH",
	"POP" ,
	"MAX",
	"MIN",
	"RAND",
	"LTZERO",
	"PROB" ,
	"SQRT",
	"ROUND",
	"CEIL",
	"FLOOR"
};

#define IS_COMMENT(a) (a)=='#' 

vm_error_func __err_out =NULL ;
vm_error_func __print_out =NULL ;

vm_error_func vm_set_outfunc(vm_error_func func)
{
	vm_error_func f = __print_out ;
	__print_out = func ;
	return f ;
}


vm_error_func vm_set_errfunc(vm_error_func errfunc)
{
	vm_error_func f = __err_out ;
	__err_out = errfunc ;
	return f ;
}


int vm_error( const char *stm,...) 
{
	int done ;
	char buf[1024*4] ;
	char *p = buf;
	va_list arg;
	
	buf[0] = 0 ;
	va_start (arg, stm);
	done = ndvsnprintf (p, sizeof(buf) , stm , arg);
	va_end (arg);
	
	if(__err_out) {
		__err_out((const char *)buf) ;
		return done ;
	}
	else {
		return ndfprintf(stderr, "%s",  buf) ;
	}
	
}

int vm_print( const char *stm,...) 
{
	int done ;
	char buf[1024*4] ;
	char *p = buf;
	va_list arg;
	
	buf[0] = 0 ;
	va_start (arg, stm);
	done = ndvsnprintf (p, sizeof(buf), stm, arg);
	va_end (arg);
	
	if(__print_out) {
		__print_out((const char *)buf) ;
		return done ;
	}
	else {
		return ndfprintf(stderr, "%s", buf) ;
	}
	
}

//parse instruct
vm_ins get_ins(char * ins) 
{
	int i ;
	for(i=0; i<EOP_NUMBERS; i++) {
		if(0==ndstricmp(ins, __op[i] )) {
			return (vm_ins) i ;
		}
	}
	return -1 ;
}

char *get_ins_name(vm_ins ins)
{
	if(ins >= 0 && ins<EOP_NUMBERS ) {
		return __op[ins] ;
	}
	return NULL;

}

//
char *read_instruction(char *src,  vm_ins *instruction)
{
	char buf[128]; 
	char *ret_addr = (char *) ndstr_parse_word_n(src, buf, 128) ;
	if(!ret_addr) {
		vm_error("syntax error: [%s]\n",src) ;
		return NULL ;
	}
	//find instruct
	*instruction =  get_ins(buf) ;
	if((vm_ins)-1==*instruction ) {
		vm_error("bad instruction : [%s]\n",buf) ;
		return NULL ;
	}
	return ret_addr ;
}

//
int asm_read_operand(char *addr, vm_data_src *data_desc, vm_value *val, char **ret_addr)
{
	char *_start_addr ;
	char buf[128] ;
	addr = (char *)ndstr_first_valid(addr);
	if(!addr) {
		return -1 ;
	}
	_start_addr = addr;
	if('['== *addr) {
		vm_adddress mm_addr ;

		if((addr[1]=='s'|| addr[1]=='S') && (addr[2]=='p' || addr[2]=='P')) {
			//read stack address
			*data_desc = EDS_STACK ;
			addr += 3 ;

			addr = (char *)ndstr_first_valid(addr);
			if(!addr) {

				//vm_error("syntax error: [%s]\n",_start_addr) ;
				return -1 ;
			}

		}
		else {
			//read memory address
			*data_desc = EDS_ADDRESS ;
			++addr ;
		}
		addr = (char *)ndstr_nstr_end(addr, buf, ']', 20);
		if(!addr || !*addr || *addr != ']' ) {
			//vm_error("syntax error: [%s]\n",_start_addr) ;
			return -1 ;
		}
		if(! ndstr_is_numerals(buf)) {
			return -1 ;
		}
		mm_addr = (vm_adddress)ndstr_atoi_hex(buf);
		*val =*((vm_value*) (&mm_addr) ); 
	}
	else if(IS_BIG_LATIN(*addr) || IS_LITTLE_LATIN(*addr)){
		addr = (char *)ndstr_parse_word_n(addr, buf, 20);
		if(0==ndstricmp("reg1", buf)) {
			*data_desc = EDS_REG1 ;
		}
		else if(0==ndstricmp("reg2", buf)) {
			*data_desc = EDS_REG2 ;
		}
		else {
			//vm_error("bad operand: [%s]\n",buf) ;
			return -1 ;
		}
	}
	else {
		int isok = 0 ;
		
		addr = (char *)ndstr_read_numerals(addr, buf, &isok);
		if(!isok) {
			//vm_error("bad operand: [%s]\n",_start_addr) ;
			return -1 ;
		}
		*val =(vm_value) atof(buf) ;
		*data_desc = EDS_IMMEDIATE ;
	}
	*ret_addr = addr ;
	return 0 ;
}
/*
 * parse a line of asm
 * return -1 error 
 *		0 nothing to be done 
		else numbers of instruction 
 */
int vm_compiler_line(char *text , struct vm_instruction_node *out_node)
{
	int num ;

	char *addr = (char *)ndstr_first_valid(text);
	if(!addr) {

		vm_error("syntax error: [%s]\n",text) ;
		//vm_error("read data error in %s \n", text) ;
		return -1 ;
	}
	if(IS_COMMENT(*addr)) {
		return 0 ;
	}
	memset(out_node, 0, sizeof(*out_node)) ;
	out_node->ds1 = EDS_NONE ;
	addr = read_instruction(addr, &(out_node->ins) ) ;
	
	if(!check_instruction_valid(out_node->ins)) {

		vm_error("syntax error: [%s]\n",text) ;
		return -1 ;
	}
	if(!addr) {
		return check_no_operand(out_node->ins) ? 1 : -1 ;
	}

	addr = (char *)ndstr_first_valid(addr);
	if(!addr) {
		if( check_no_operand(out_node->ins) ) {
			return 1 ;
		}
		else {
			vm_error("syntax error: [%s]\n",text) ;
			return -1;
		}
	}

	num =  get_operand_num(out_node->ins) ;
	if(num--) {
		if(-1==asm_read_operand(addr, &out_node->ds1, &out_node->val1, &addr) ) {

			vm_error("operand error: [%s]\n",text) ;
			return -1 ;
		}
	}

	if(num) {
		addr = ndstrchr(addr, ',') ;
		if(!addr) {
			vm_error("syntax error: [%s] miss , \n",text) ;
			return -1 ;
		}
		++addr ;
		if(-1==asm_read_operand(addr, &out_node->ds2, &out_node->val2, &addr) ) {
			vm_error("read second operand error: [%s] \n",text) ;
			return -1 ;
		}
	}

	//read data success! check success
	//
	
	return 1 ;	
}

//int vm_run_line()

int vm_file_compile(/*struct vm_instruction_node *out_node,*/ FILE *infp, FILE *outfile)
{
	char *p ;
	int ret, n ;
	struct vm_instruction_node node ;
	char buf[1024] ;

	while ( fgets( buf, 1024, infp ) ) {
		
		p = (char *)ndstr_first_valid(buf);
		if(!p) 
			continue ;

		ret = vm_compiler_line(buf , &node) ;
	
		if(ret > 0  && vm_check_insnode( &node)) {			
			n = (int)vm_instruct_2buf( &node,  buf) ;
			if(n > 0 && n< sizeof(buf)) {
				buf[n] = 0 ;
				fwrite(buf,n, 1,outfile);
			}
		}
		else  {
			vm_error( "bad instruction: %s\n", buf ) ;
			return -1 ;
		}

	}
	return 0 ;

}

size_t _filesize(FILE *stream)
{
	long curpos, length;

	curpos = ftell(stream);
	fseek(stream, 0L, SEEK_END);
	length = ftell(stream);
	fseek(stream, curpos, SEEK_SET);
	return length;
} 

//disasm
int vm_file_rcompile(/*struct vm_instruction_node *out_node,*/ FILE *infp, FILE *outfile)
{
	char *p, *buf ;
	int  n ;
	size_t fsize ;

	if(infp == stdin) {
		fsize = 4096 ;
	}
	else {
		fsize = _filesize(infp) ;
	}
	if(fsize ==0 ) {
		vm_error("bad file!\n") ;
		return -1;
	}
	buf = malloc(fsize) ;
	if(!buf) {
		vm_error("bad memory alloc!\n") ;
		return -1 ;
	}
	n = (int) fread( buf, sizeof( char ), fsize, infp );
	if(n<=0) {
		free(buf) ;
		return -1 ;
	}
	p = buf ;
	
	while (p <= (buf+n)) {
		struct vm_instruction_node node = {0};
		if(0==vm_instruction_2node(&node ,(void**) &p , buf+n) ) {
			vm_output_asm(&node, outfile, MEDIUM_FILE) ;
		}
	}

	free(buf) ;
	return 0 ;

}

//output dis asm 
int vm_output_asm(struct vm_instruction_node *node, void *outstream, int type)
{
	int num , len;
	char *p ;

	size_t buf_size ;

	char buf[256] ;
	
	buf_size = sizeof(buf) ;
	p = get_ins_name(node->ins);
	if(!p)
		return -1 ;
	len = ndsnprintf(buf, buf_size, "\t%s\t", p) ;

	p = buf + len ;
	buf_size -= len ;

	num =  get_operand_num(node->ins) ;
	
	len = 0 ;
	if(num--){
		switch(node->ds1) {
		case EDS_NONE:
			break ;
		case EDS_IMMEDIATE:
			len = ndsnprintf(p, buf_size, "%f", node->val1) ;
			break ;
		case EDS_ADDRESS:
			len = ndsnprintf(p, buf_size, "[%x]", *((vm_adddress*) &node->val1)  ) ;
			break ;
		case EDS_STACK:
			len = ndsnprintf(p, buf_size, "[SP  %d]", *((vm_adddress*) &node->val1)  ) ;
			break ;
		case EDS_REG1:
			len = ndsnprintf(p, buf_size, "REG1" ) ;
			break ;
		case EDS_REG2:
			len = ndsnprintf(p, buf_size, "REG2" ) ;
			break ;
		default :
			break ;
		}
	}
	if(num) {
		p += len ;
		buf_size -= len ;
		len = ndsnprintf(p, buf_size, " ,\t") ;

		p += len ;
		buf_size -= len ;

		switch(node->ds2) {
		case EDS_NONE:
			break ;
		case EDS_IMMEDIATE:
			len = ndsnprintf(p, buf_size, "%f", node->val2) ;
			break ;
		case EDS_ADDRESS:
			len = ndsnprintf(p, buf_size, "[%x]", *((vm_adddress*) &node->val2)  ) ;
			break ;
		case EDS_STACK:
			len = ndsnprintf(p, buf_size, "[SP %d]", *((vm_adddress*) &node->val2)  ) ;
			break ;
		case EDS_REG1:
			len = ndsnprintf(p, buf_size, "REG1" ) ;
			break ;
		case EDS_REG2:
			len = ndsnprintf(p, buf_size, "REG2" ) ;
			break ;
		default :
			break ;
		}
	}


	if(MEDIUM_FILE==type) {
		return ndfprintf((FILE*)outstream, "%s\n", buf) ;
	}
	else if(MEDIUM_BUF==type) {
		return ndsnprintf(outstream, sizeof(buf),"%s", buf) ;
	}
	return 0 ;
}
