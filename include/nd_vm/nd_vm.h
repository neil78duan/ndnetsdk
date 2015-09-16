/* file nd_vm.h
 * define virtual machine
 *
 * 2009-6-5 23:22
 */

/*
 * ����һ��ִ����ֵ���������CPU
 * ��������ִ��һЩ�򵥵���������,
 * �����ڴ���������صĲ���
 * ���ҿ��Է��ؽ��
 */
#ifndef _ND_VM_H_
#define _ND_VM_H_

//#include <stdio.h>
//#include <stdlib.h>
//#include "nd_comdef.h"
#include "nd_common/nd_common.h"

#pragma warning(disable: 4996)

#ifndef CPPAPI
#ifdef __cplusplus
#define CPPAPI extern "C" 
#else 
#define CPPAPI 
#endif 
#endif

#ifndef __INLINE__
#ifdef __cplusplus
#define __INLINE__			inline	
#else 
#define __INLINE__			__inline	
#endif 
#endif

#ifndef ND_VM_API
#define ND_VM_API		CPPAPI
#endif

typedef float vm_value ;				//������
typedef unsigned char vm_ins ;			//ָ��
typedef unsigned char vm_data_src ;		//������Դ
typedef signed int	   vm_adddress ;	//��ַ
#define BAD_INSTRUCTION (vm_data_type)-1 

#define VM_STACK_SIZE 32 
#define VM_DFT_MMSIZE  1024 

typedef void (*vm_error_func)(const char *err_desc)   ;


struct vm_cpu ;
//ͨ���˿ڷ����ڴ溯��
typedef vm_value* (*vm_mm_port) (vm_adddress addr, struct vm_cpu *vm) ;

enum e_vm_op
{
	EOP_NOP = 0 ,
	EOP_MOV,
	EOP_ADD,
	EOP_SUB,
	EOP_MUL,
	EOP_DIV,
	
	EOP_PUSH,
	EOP_POP	,
	EOP_MAX	,
	EOP_MIN	,
	EOP_RAND,
	EOP_LTZERO,
	EOP_PROB ,
	EOP_SQRT ,
	EOP_NUMBERS 
};

//��������Դ
enum e_data_src
{
	EDS_NONE			//û�в�����
	,EDS_IMMEDIATE		//������
	,EDS_ADDRESS		//�ڴ��ַ
	,EDS_STACK			//��ջѰַ
	,EDS_REG1			//�Ĵ���1
	,EDS_REG2			//�Ĵ���2
};

//������ļ����ǻ����־
enum e_medium{
	MEDIUM_FILE = 0,
	MEDIUM_BUF =1 
};
struct vm_cpu
{
	vm_value reg1 ;							//�Ĵ���1 ͨ�üĴ���
	vm_value reg2 ;							//�Ĵ���2 ��ʱ�Ĵ���
	void *ip ;								//ָ���ַ
	void *ip_end ;
	vm_value *sp ;							//stack point
	vm_mm_port	mm_func ;					//�����ڴ�ĺ������(���mm_func==NULL����memory�з����ڴ�)
	vm_value *memory ;						//�����ڴ�
	size_t	 mem_size ;						//�ڴ��С 
	unsigned char	 echo_cmd:1 ;			//��ʾָ��
	unsigned char	 echo_res:1 ;			//��ʾ���н��
	unsigned char	 flag ;					//��־λ
	void *extern_param;						//�ⲿʹ�õĲ���
	vm_value stack[VM_STACK_SIZE] ;			//stack
};

struct vm_instruction_node
{
	vm_ins  ins;
	
	vm_data_src ds1, ds2 ;		//������Դ
	
	vm_value val1, val2 ; //������
} ;

//�����������
ND_VM_API vm_error_func vm_set_outfunc(vm_error_func func) ;
//���ô����������
ND_VM_API vm_error_func vm_set_errfunc(vm_error_func errfunc);

//��ӡ��Ϣ
ND_VM_API int vm_print( const char *stm,...) ;

//����ִ��һ��ָ��
ND_VM_API int vm_step(struct vm_cpu *vm, void *ins_addr, void *ins_end) ;

//ִ�нڵ��Ӧ��ָ��
ND_VM_API int vm_run_insnode(struct vm_instruction_node *node,struct vm_cpu *vm) ;

//������ڵ�ת����bin��
ND_VM_API size_t vm_instruct_2buf(struct vm_instruction_node *instruction_node, void *buf) ;

//��ָ�������node�ṹ
ND_VM_API int vm_instruction_2node(struct vm_instruction_node *node , void **ins_start , void *ins_end);

//��ʼ�������
ND_VM_API void vm_machine_init(struct vm_cpu *vm, vm_value *mm_addr,size_t mm_size) ;

//�����ڴ��ȡ����
ND_VM_API void vm_set_mmfunc(struct vm_cpu *vm,vm_mm_port mm_func,size_t mm_size) ;

//ִ��������
ND_VM_API int vm_run_cmd(struct vm_cpu *vm, void *ins_addr, size_t ins_size) ;

//�����ⲿ����
static __INLINE__ void vm_set_param(struct vm_cpu *vm, void *param)
{	
	vm->extern_param = param ;
}

//�õ��ⲿ����
static __INLINE__ void* vm_get_param(struct vm_cpu *vm)
{	
	return vm->extern_param;
}

//�õ�����ֵ
static __INLINE__ vm_value vm_return_val(struct vm_cpu *vm)
{
	return vm->reg1 ;
}

//�����Ƿ���ʾָ��
static __INLINE__ int vm_set_echo_ins(struct vm_cpu *vm, int flag)
{
	int ret = vm->echo_cmd ;
	vm->echo_cmd = flag ? 1 : 0 ;
	return ret ;
}

//�����Ƿ���ʾ���нṹ
static __INLINE__ int vm_set_echo_result(struct vm_cpu *vm, int flag)
{
	int ret = vm->echo_res ;
	vm->echo_res = flag ? 1 : 0 ;
	return ret ;
}

//�����������
ND_VM_API int vm_error( const char *stm,...) ;

//����һ��ASM��
ND_VM_API int vm_compiler_line(char *text , struct vm_instruction_node *out_node);

//�õ���������ֵ
ND_VM_API vm_value vm_getvalue(vm_data_src ds,vm_value val, struct vm_cpu *vm);
//�õ��������Ķ�Ӧ�ĵ�ַ
ND_VM_API vm_value* vm_ref_operand(vm_data_src ds,vm_value val, struct vm_cpu *vm);
//ִ��һ���ڵ�ָ��
ND_VM_API int vm_run_insnode(struct vm_instruction_node *node,struct vm_cpu *vm);

//�õ�ָ���Ӧ�Ĳ�����
ND_VM_API int get_operand_num(vm_ins instruction);

//����Ƿ���Ҫ������
ND_VM_API int check_no_operand(vm_ins instruction) ;

//���ָ���Ƿ���Ч
ND_VM_API int check_instruction_valid(vm_ins instruction) ;

//�õ���ַΪindex���ڴ��ַ
ND_VM_API vm_value* _get_memory(struct vm_cpu *vm, vm_adddress index);

//�õ�ָ������
ND_VM_API char *get_ins_name(vm_ins ins);

//������㱨��Ϣ
ND_VM_API int vm_output_asm(struct vm_instruction_node *node, void *outstream, int type) ;

//���ָ���Ƿ�Ϸ����������ڴ��Ƿ�Խ��
ND_VM_API int vm_check_insnode(struct vm_instruction_node *node) ;

//��asm�ļ������bin�ļ�
ND_VM_API int vm_file_compile(/*struct vm_instruction_node *out_node*/ FILE *infp, FILE *outfile);

//�����
ND_VM_API int vm_file_rcompile(/*struct vm_instruction_node *out_node*/ FILE *infp, FILE *outfile);

//��ʾ����nodeָ���Ľ��
ND_VM_API int vm_echo_res(struct vm_instruction_node *node,struct vm_cpu *vm) ;

/* �����滻����
 * �ڽ������ʽ��ʱ��,���ܻ��õı�������,���ߺ궨��,��Ҫ�滻����Ӧ���ڴ��ַ���߳���
 * ��Ҫ�ⲿ�����ṩ�滻����
 * @input �ӱ��ʽ�ж�ȡ�ı�����
 * @buf�滻�����������
 * @size size of buf
 *
 * return value : return 0 success ,on error return -1
 */
typedef int (*vm_param_replace_func)(const char *input,char *buf, int size,void *user_data) ;

/* ���������㹫ʽ�ǽ�����bin
 * ��ʽ ���� : [1] = 2*3+[4] * min([5],7*8) + rand(1,100)  + max([6],[7])  //�ѱ��ʽ��ֵ�����ڴ�1
 *			[n]  ��ַΪn���ڴ浥Ԫ
			min(m,n) ��[m,n]ȡ��Сֵ
			max(m,n) ��[m,n]ȡ���ֵ
			rand(m,n) ��[m,n]ȡ���ֵ

			
 */
ND_VM_API size_t vm_parse_expression(const char *textbuf, const char *code_buf,size_t buf_size, vm_param_replace_func func,void*user_data) ;

//�õ��ļ�����
ND_VM_API size_t _filesize(FILE *stream);

//����
ND_VM_API void vm_start_debug() ;
ND_VM_API void vm_end_debug() ;

#endif

