/* file nd_vmmain.c
 * main function fo nd virtual machine
 *
 * neil duan
 * 2008-6
 * all right reserved !
 */

#if !defined(_LIB) || !defined(_USRDLL)
#include "nd_vm/nd_vm.h"
#include "nd_common/nd_str.h"
#include <stdio.h>

//#pragma comment(lib, "nd_vmlib.lib")

extern void vm_start_debug(void) ;
extern void vm_end_debug(void) ;

#define NDVM_BIN_HDR 0x4e44564d 
//文件头
struct nd_binfile_header {
	unsigned int flag ;
	short	header_size ;
	size_t  file_size ;
};

//虚拟机器头
struct nd_binvm_header
{
	short  vm_hdr_size;
	size_t mm_size ;
	size_t stack_size ;
};

int bin_check_header(FILE *fp) 
{
	size_t curpos;
	size_t size ;
	struct nd_binfile_header hdr ;
	
	curpos = ftell(fp);

	size = _filesize(fp) ;

	if(fread(&hdr, sizeof(hdr), 1 , fp) <= 0) {
		return -1 ;
	}

	if(hdr.flag == NDVM_BIN_HDR ) {
		if(hdr.file_size!=size) {
			return -1;
		}
		if(hdr.file_size<= (sizeof(struct nd_binfile_header)+sizeof(struct nd_binvm_header)))
			return -1 ;

		return 1 ;
	}
	else {
		fseek(fp, (long)curpos, SEEK_SET);
	}
	return 0 ;
}

void bin_write_hdr(FILE *fp)
{
	unsigned int h = NDVM_BIN_HDR ;
	fwrite(&h, sizeof(h), 1, fp) ;
}

//执行控制台程序
int run_control(void) 
{
	return 0;
}

//编译文件
int run_compile(char *src_file, char *dest_file) 
{
	return 0 ;
}

//反汇编文件
int run_rcompile(char *src_file, char*dest_file) 
{
	return 0;
}

//执行文件(二进制或者汇编)
int run_file(char *file)
{
	int ret ;
	
	struct vm_cpu vm ;
	vm_value memory[VM_DFT_MMSIZE] = {0};
	
	FILE *fp = fopen(file, "r") ;
	if(!fp) {
		ndfprintf(stderr, "open %s file error\n", file); ;
		return 1 ;
	}
	
	vm_machine_init(&vm, memory, VM_DFT_MMSIZE) ;
	
	ret = bin_check_header(fp) ;
	if(-1==ret) {
		ndfprintf(stderr, "bad file %s\n", file) ;
		return 1 ;
	}
	else if(1==ret) {
		//run bin file
		size_t fsize ;
		void *run_buf ;
		struct nd_binfile_header hdr ;
		struct nd_binvm_header vm_hdr ;

		//fsize = _filesize(fp) ;

		fread(&hdr, sizeof(hdr), 1 , fp) ;
		fread(&vm_hdr, sizeof(vm_hdr), 1 , fp) ;
		//
		run_buf = malloc(hdr.file_size) ;
		if(!run_buf) {
			fclose(fp);
			return -1 ;
		}

		fsize = fread(run_buf,hdr.file_size, 1, fp) ;
		if(fsize!=hdr.file_size - sizeof(struct nd_binfile_header)+sizeof(struct nd_binvm_header)) {			
			fclose(fp);
			return -1 ;
		}
		//run command
		vm_run_cmd(&vm,run_buf, fsize) ;

	}
	else {
		// run asm file 

		char *p ;
		
		struct vm_instruction_node node ;
		char line[256] ;

		while ( fgets( line, 256, fp ) ) {
			p = (char*) ndstr_first_valid(line) ;
			if(!p) 
				continue ;
			
			ret = vm_compiler_line(line , &node) ;
			if(ret > 0 ) {
				if(-1!=vm_run_insnode(&node, &vm) ) {
					vm_output_asm(&node, stdout, MEDIUM_FILE);
				}
			}
			else if(ret == -1) {
				ndfprintf(stderr, "bad command: %s\n", line ) ;
			}
//#error not complete
		}
	}
	return 0 ;
}

int run_help()
{
	return 0;
}

int main(int argc, char *argv[]) 
{
	vm_start_debug() ;

	vm_end_debug() ;
	return 0 ;
}

#endif
