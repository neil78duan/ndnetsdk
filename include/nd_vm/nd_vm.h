/* file nd_vm.h
 * define virtual machine
 *
 * 2009-6-5 23:22
 */

/*
 * 这是一个执行数值运算的虚拟CPU
 * 可以用来执行一些简单的四则运算,
 * 并对内存数组做相关的操作
 * 而且可以返回结果
 */
#ifndef _ND_VM_H_
#define _ND_VM_H_

#include <stdio.h>
#include <stdlib.h>
//#include "nd_comdef.h"
//#include "nd_common/nd_common.h"

#include "nd_common/nd_export_def.h"

#pragma warning(disable: 4996)


typedef float vm_value ;				//操作数
typedef unsigned char vm_ins ;			//指令
typedef unsigned char vm_data_src ;		//数据来源
typedef signed int	   vm_adddress ;	//地址
#define BAD_INSTRUCTION (vm_data_type)-1 

#define VM_STACK_SIZE 32 
#define VM_DFT_MMSIZE  1024 

typedef void (*vm_error_func)(const char *err_desc)   ;


struct vm_cpu ;
//通过端口访问内存函数
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
	EOP_ROUND,
	EOP_CEIL,
	EOP_FLOOR,
	EOP_NUMBERS 
};

//操作数来源
enum e_data_src
{
	EDS_NONE			//没有操作数
	,EDS_IMMEDIATE		//立即数
	,EDS_ADDRESS		//内存地址
	,EDS_STACK			//堆栈寻址
	,EDS_REG1			//寄存器1
	,EDS_REG2			//寄存器2
};

//输出到文件还是缓冲标志
enum e_medium{
	MEDIUM_FILE = 0,
	MEDIUM_BUF =1 
};
struct vm_cpu
{
	vm_value reg1 ;							//寄存器1 通用寄存器
	vm_value reg2 ;							//寄存器2 临时寄存器
	void *ip ;								//指令地址
	void *ip_end ;
	vm_value *sp ;							//stack point
	vm_mm_port	mm_func ;					//访问内存的函数入口(如果mm_func==NULL将从memory中访问内存)
	vm_value *memory ;						//操作内存
	size_t	 mem_size ;						//内存大小 
	unsigned char	 echo_cmd:1 ;			//显示指令
	unsigned char	 echo_res:1 ;			//显示运行结果
	unsigned char	 flag ;					//标志位
	void *extern_param;						//外部使用的参数
	vm_value stack[VM_STACK_SIZE] ;			//stack
};

struct vm_instruction_node
{
	vm_ins  ins;
	
	vm_data_src ds1, ds2 ;		//数据来源
	
	vm_value val1, val2 ; //操作数
} ;

//设置输出函数
ND_VM_API vm_error_func vm_set_outfunc(vm_error_func func) ;
//设置错误输出函数
ND_VM_API vm_error_func vm_set_errfunc(vm_error_func errfunc);

//打印信息
ND_VM_API int vm_print( const char *stm,...) ;

//单步执行一个指令
ND_VM_API int vm_step(struct vm_cpu *vm, void *ins_addr, void *ins_end) ;

//执行节点对应的指令
ND_VM_API int vm_run_insnode(struct vm_instruction_node *node,struct vm_cpu *vm) ;

//把命令节点转化成bin流
ND_VM_API size_t vm_instruct_2buf(struct vm_instruction_node *instruction_node, void *buf) ;

//把指令流变成node结构
ND_VM_API int vm_instruction_2node(struct vm_instruction_node *node , void **ins_start , void *ins_end);

//初始化虚拟机
ND_VM_API void vm_machine_init(struct vm_cpu *vm, vm_value *mm_addr,size_t mm_size) ;

//设置内存存取函数
ND_VM_API void vm_set_mmfunc(struct vm_cpu *vm,vm_mm_port mm_func,size_t mm_size) ;

//执行命令流
ND_VM_API int vm_run_cmd(struct vm_cpu *vm, void *ins_addr, size_t ins_size) ;

//设置外部参数
static __INLINE__ void vm_set_param(struct vm_cpu *vm, void *param)
{	
	vm->extern_param = param ;
}

//得到外部参数
static __INLINE__ void* vm_get_param(struct vm_cpu *vm)
{	
	return vm->extern_param;
}

//得到返回值
static __INLINE__ vm_value vm_return_val(struct vm_cpu *vm)
{
	return vm->reg1 ;
}

//设置是否显示指令
static __INLINE__ int vm_set_echo_ins(struct vm_cpu *vm, int flag)
{
	int ret = vm->echo_cmd ;
	vm->echo_cmd = flag ? 1 : 0 ;
	return ret ;
}

//设置是否显示运行结构
static __INLINE__ int vm_set_echo_result(struct vm_cpu *vm, int flag)
{
	int ret = vm->echo_res ;
	vm->echo_res = flag ? 1 : 0 ;
	return ret ;
}

//错误输出函数
ND_VM_API int vm_error( const char *stm,...) ;

//编译一个ASM行
ND_VM_API int vm_compiler_line(char *text , struct vm_instruction_node *out_node);

//得到操作数的值
ND_VM_API vm_value vm_getvalue(vm_data_src ds,vm_value val, struct vm_cpu *vm);
//得到操作数的对应的地址
ND_VM_API vm_value* vm_ref_operand(vm_data_src ds,vm_value val, struct vm_cpu *vm);
//执行一个节点指令
ND_VM_API int vm_run_insnode(struct vm_instruction_node *node,struct vm_cpu *vm);

//得到指令对应的操作数
ND_VM_API int get_operand_num(vm_ins instruction);

//检测是否需要操作数
ND_VM_API int check_no_operand(vm_ins instruction) ;

//检测指令是否有效
ND_VM_API int check_instruction_valid(vm_ins instruction) ;

//得到地址为index的内存地址
ND_VM_API vm_value* _get_memory(struct vm_cpu *vm, vm_adddress index);

//得到指令名字
ND_VM_API char *get_ins_name(vm_ins ins);

//输出反汇报信息
ND_VM_API int vm_output_asm(struct vm_instruction_node *node, void *outstream, int type) ;

//检测指令是否合法但不会检测内存是否越界
ND_VM_API int vm_check_insnode(struct vm_instruction_node *node) ;

//把asm文件编译程bin文件
ND_VM_API int vm_file_compile(/*struct vm_instruction_node *out_node*/ FILE *infp, FILE *outfile);

//反汇编
ND_VM_API int vm_file_rcompile(/*struct vm_instruction_node *out_node*/ FILE *infp, FILE *outfile);

//显示运行node指令后的结果
ND_VM_API int vm_echo_res(struct vm_instruction_node *node,struct vm_cpu *vm) ;

/* 变量替换函数
 * 在解析表达式的时候,可能会用的变量名字,或者宏定义,需要替换成相应的内存地址或者常数
 * 需要外部程序提供替换规则
 * @input 从表达式中读取的变量名
 * @buf替换后输出变量名
 * @size size of buf
 *
 * return value : return 0 success ,on error return -1
 */
typedef int (*vm_param_replace_func)(const char *input,char *buf, int size,void *user_data) ;

/* 把四则运算公式是解析成bin
 * 公式 范例 : [1] = 2*3+[4] * min([5],7*8) + rand(1,100)  + max([6],[7])  //把表达式的值付给内存1
 *			[n]  地址为n的内存单元
			min(m,n) 在[m,n]取最小值
			max(m,n) 在[m,n]取最大值
			rand(m,n) 在[m,n]取随机值

			
 */
ND_VM_API size_t vm_parse_expression(const char *textbuf, char *code_buf,size_t buf_size, vm_param_replace_func func,void*user_data) ;

//得到文件长度
ND_VM_API size_t _filesize(FILE *stream);

//调试
ND_VM_API void vm_start_debug(void) ;
ND_VM_API void vm_end_debug(void) ;

#endif

