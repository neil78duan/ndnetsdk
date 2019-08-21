/* file nd_vm.h
 * define virtual machine
 *
 * 2008-6-5 23:22
 */

#include "nd_vm/nd_vm.h"
#include "nd_common/ndstdstring.h"
#include <math.h>
#include <string.h>

#ifndef __cplusplus

#ifndef max 
#define max(a,b) 			(((a)>(b))? (a) : (b))
#endif 

#ifndef min
#define min(a,b) 			(((a)>(b))? (b) : (a))
#endif

#endif 

/*
	指令结构 :
	| 1bytes |  (1 bytes) |   4bytes  |  (1 bytes) |   4bytes  |  下一个条指令
		|			|			|
	  指令       源操作数源	 源操作数值,	目的操作数				或者地址
	 
	每条指令包括3个部分,指令,操作数源和操作数值
	指令是CPU动作,执行+ - * / 等 功能
	操作数来源: 立即数或者是内存地址(不是所有的指令都有)
	操作数值,或者地址 : 指明操的是立即数还是内存地址.	
*/

//初始化一个虚拟机器
void vm_machine_init(struct vm_cpu *vm, vm_value *mm_addr,size_t mm_size)
{
	memset(vm, 0, sizeof(*vm)) ;
	vm->memory = mm_addr ;
	vm->mem_size = mm_size ;
	
	vm->reg1 = 0 ;
	vm->sp = vm->stack ;
	
	//vm->ip = ins_addr ;
	//vm->ip_end = (void*)(((char*) vm->ip) + ins_size );
}


void vm_set_mmfunc(struct vm_cpu *vm,vm_mm_port mm_func,size_t mm_size) 
{
	if(mm_func) {
		vm->mm_func = mm_func ;
		vm->mem_size = mm_size;
	}
}
#define _READ_STREAM(_val, addr) \
	do 	{\
		int i  ; \
		char *_p1 = (char*)&_val; \
		for (i = 0; i<sizeof(_val); i++) {	\
			*_p1++ = *((*(char**)&(addr))++);	\
		}							\
	} while (0)

//#define _FETCH_AND_INC_ADDR(_type, addr) \
//	*((*(_type**)&(addr))++) 
// 

#define _WRITE_AND_INC_ADDR(_type, addr, _val)  \
	do 	{\
		int i  ; \
		char *_p1 = (char*)&(_val); \
		for (i = 0; i<sizeof(_type); i++) {	\
			*((*(char**)&(addr))++) =*_p1++ ;	\
		}							\
	} while (0)

static __INLINE__ vm_ins _read_ins(struct vm_cpu *vm)
{
	//return _FETCH_AND_INC_ADDR(vm_ins,vm->ip) ;
	vm_ins a;
	_READ_STREAM(a, vm->ip);
	return a;
}

static __INLINE__ vm_data_src _read_data_src(struct vm_cpu *vm)
{
	//return _FETCH_AND_INC_ADDR(vm_data_src,vm->ip) ;

	vm_data_src a;
	_READ_STREAM(a, vm->ip);
	return a;

}

static __INLINE__ vm_value _read_data(struct vm_cpu *vm)
{
	//return _FETCH_AND_INC_ADDR(vm_value,vm->ip) ;

	vm_value a;
	_READ_STREAM(a, vm->ip);
	return a;


}

/*得到内存地址,主要是为了打印调试信息*/
vm_value* _get_memory(struct vm_cpu *vm, vm_adddress index)
{
	if(index>=0 && index<=(vm_adddress)(vm->mem_size) ){
		if(vm->mm_func) 
			return vm->mm_func(index, vm) ;
		else 
			return &(vm->memory[index]) ;
	}
	//else if(index + vm->sp >= vm->stack) {
	//	return  vm->sp + index ;
	//}
	return 0 ;
}

//得到操作数数目
int get_operand_num(vm_ins instruction)
{
	if(EOP_NOP==instruction) {
		return 0 ;
	}
	else if(EOP_POP==instruction || EOP_PUSH==instruction || 
		EOP_LTZERO==instruction|| EOP_PROB==instruction || EOP_SQRT==instruction|| EOP_ROUND==instruction ||
		EOP_CEIL == instruction||EOP_FLOOR == instruction) {
		return 1 ;
	}
	else {
		return 2 ;
	}
}

int check_no_operand(vm_ins instruction)
{	
	if(EOP_NOP==instruction || EOP_POP==instruction) {
		return 1;
	}
	return 0 ;
}

int check_instruction_valid(vm_ins instruction)
{	
	if(instruction <EOP_NUMBERS) {
		return 1;
	}
	return 0 ;
}

//把指令流变成node结构
int vm_instruction_2node(struct vm_instruction_node *node , void **ins_start , void *ins_end)
{
	int num ;
	struct vm_cpu vm = {0};
	
	vm.ip = *ins_start ;
	vm.ip_end = ins_end ;
	
	node->ins = _read_ins(&vm) ;

	num = get_operand_num(node->ins) ;
	if(num--) {
		node->ds1 = _read_data_src(&vm) ;
		if(EDS_IMMEDIATE==node->ds1 || EDS_ADDRESS==node->ds1 ||EDS_STACK==node->ds1) {
			node->val1 = _read_data(&vm) ;
		}
	}	
	if(num) {
		node->ds2 = _read_data_src(&vm) ;
		if(EDS_IMMEDIATE==node->ds2 || EDS_ADDRESS==node->ds2||EDS_STACK==node->ds2) {
			node->val2 = _read_data(&vm) ;
		}
	}
	*ins_start = vm.ip ;
	return 0 ;

}
//单步读取并执行一个指令
int vm_step(struct vm_cpu *vm, void *ins_start , void *ins_end)
{
	struct vm_instruction_node node = {0};
	
	vm->ip = ins_start ;
	vm->ip_end = ins_end ;
	vm_instruction_2node(&node , &(vm->ip) , vm->ip_end) ;

	if(-1==vm_run_insnode(&node, vm)) {
		return -1 ;
	}
	return 0 ;

}

//执行一个指令流
int vm_run_cmd(struct vm_cpu *vm,void *ins_addr, size_t ins_size) 
{
	
	void *ins_end;
	vm->ip = ins_addr ;
	ins_end = (void*)(((char*) ins_addr) + ins_size );

	while (vm->ip < ins_end) {
		if(-1==vm_step(vm, vm->ip, ins_end) ) {
			return -1 ;
		}		
	}			//end while 
	
	return 0 ;
}

//把指令节点转变成流式指令,便于保存到文件或者缓冲中
size_t vm_instruct_2buf(struct vm_instruction_node *node, void *buf) 
{
	size_t size = 0 ;
	int num ;
	
	//*((vm_ins *)buf)++ = node->ins ;
	_WRITE_AND_INC_ADDR(vm_ins, buf, node->ins);

	num = get_operand_num(node->ins) ;
	size += sizeof(vm_ins) ;

	if(num--) {
		//*((vm_data_src  *)buf)++ = node->ds1 ;
		_WRITE_AND_INC_ADDR(vm_data_src, buf, node->ds1);
		size += sizeof(vm_data_src) ;
		if(EDS_IMMEDIATE==node->ds1 || EDS_ADDRESS==node->ds1||EDS_STACK==node->ds1) {
			//*((vm_value*)buf)++ = node->val1 ;
			_WRITE_AND_INC_ADDR(vm_value, buf, node->val1);
			size += sizeof(vm_value) ;
		}
	}	
	if(num--) {
		//*((vm_data_src  *)buf)++ = node->ds2 ;
		_WRITE_AND_INC_ADDR(vm_data_src, buf, node->ds2);
		size += sizeof(vm_data_src) ;
		if(EDS_IMMEDIATE==node->ds2 || EDS_ADDRESS==node->ds2||EDS_STACK==node->ds2) {
			//*((vm_value*)buf)++ = node->val2 ;
			_WRITE_AND_INC_ADDR(vm_value, buf, node->val2);
			size += sizeof(vm_value) ;
		}
	}	

	return size ;
}

//得到操作数的值
vm_value vm_getvalue(vm_data_src ds,vm_value val, struct vm_cpu *vm)
{
	if(EDS_IMMEDIATE==ds) {
		return val ;
	}
	else if(EDS_REG1==ds) {
		return vm->reg1;
	}
	else if(EDS_REG2==ds) {
		return vm->reg2;
	}
	else if(EDS_STACK==ds) {
		//读取堆栈
		vm_adddress index = *((vm_adddress*) &val) ;
		if( index < 0  ) {
			if(vm->stack <= (vm->sp + index))
				return *(vm->sp + index);
		}
		else if((vm->stack+index) <  vm->sp )
			return vm->stack[index] ;
		return (vm_value)0; 
	}
	else if(EDS_ADDRESS==ds) {
		vm_adddress index = *((vm_adddress*) &val) ;
		if(index >=0 && index < (vm_adddress)vm->mem_size ) {
			if(vm->mm_func) {
				vm_value *v = vm->mm_func(index,vm) ;
				if(v){
					return *v ;
				}
				else 
					return (vm_value)0;
			}
			else 
				return vm->memory[index] ;
		}
		return (vm_value)0;
	}
	return (vm_value)0;
}

//得到操作数的对应的地址
vm_value* vm_ref_operand(vm_data_src ds,vm_value val, struct vm_cpu *vm)
{
	if(EDS_REG1==ds) {
		return &(vm->reg1);
	}
	else if(EDS_REG2==ds) {
		return &(vm->reg2);
	}
	else if(EDS_STACK==ds) {
		//读取堆栈
		vm_adddress index = *((vm_adddress*) &val) ;
		if( index < 0  ) {
			if(vm->stack <= (vm->sp + index))
				return (vm_value*)(vm->sp + index);
		}
		else if((vm->stack+index) <  vm->sp )
			return &(vm->stack[index]) ;
		return 0; 
	}
	else if(EDS_ADDRESS==ds) {
		vm_adddress index = *((vm_adddress*) &val) ;
		if(index >=0 && index < (vm_adddress)vm->mem_size ) {
			if(vm->mm_func) 
				return vm->mm_func(index,vm) ;
			else 
				return &(vm->memory[index]) ;
		}
		//else if( index < 0 && vm->stack <= (vm->sp + index) ) {
		//	return (vm->sp + index);
		//}
		return 0;
	}
	return 0;
}


vm_value vm_rand(vm_value val1, vm_value val2) ;
//执行一个节点指令
int vm_run_insnode(struct vm_instruction_node *node,struct vm_cpu *vm)
{
	vm_value *val1, val2;
	switch (node->ins){
	case EOP_MOV:
		val1 = vm_ref_operand(node->ds1,node->val1, vm) ;
		val2 = vm_getvalue(node->ds2,node->val2, vm) ;
		if(val1) {
			*val1 = val2 ;
		}
		break ;
	case EOP_ADD:
		val1 = vm_ref_operand(node->ds1,node->val1, vm) ;
		val2 = vm_getvalue(node->ds2,node->val2, vm) ;
		if(val1) {
			*val1 += val2 ;
		}
		break ;
	case EOP_SUB:
		val1 = vm_ref_operand(node->ds1,node->val1, vm) ;
		val2 = vm_getvalue(node->ds2,node->val2, vm) ;

		if(val1) {
			*val1 -= val2 ;
		}
		break ;
	case EOP_MUL:
		val1 = vm_ref_operand(node->ds1,node->val1, vm) ;
		val2 = vm_getvalue(node->ds2,node->val2, vm) ;

		if(val1) {
			*val1 *= val2 ;
		}
		break ;
	case EOP_DIV:
		val1 = vm_ref_operand(node->ds1,node->val1, vm) ;
		val2 = vm_getvalue(node->ds2,node->val2, vm) ;
		if(val1 && val2!=(vm_value)0) {
			*val1 /= val2 ;
		}
		break ;

	case EOP_PUSH:
		val2 = vm_getvalue(node->ds1,node->val1, vm) ;
		//vm->stack[vm->sp++] = val2 ;
		*(vm->sp++) = val2 ;
		break ;
	case EOP_POP:
		val1 = vm_ref_operand(node->ds1,node->val1, vm) ;
		if(val1) {
			*val1 = *(--vm->sp) ;
		}
		else {
			vm->sp-- ;
		}
		break ;
	case EOP_MAX:
		{
			vm_value val_tmp = vm_getvalue(node->ds1,node->val1, vm) ;
			val2 = vm_getvalue(node->ds2,node->val2, vm) ;
			vm->reg1 = max(val_tmp, val2) ;
			break ;

		}
	case EOP_MIN:
		{
			vm_value val_tmp = vm_getvalue(node->ds1,node->val1, vm) ;
			val2 = vm_getvalue(node->ds2,node->val2, vm) ;
			vm->reg1 = min(val_tmp, val2) ;
			break ;

		}
	case EOP_RAND :
		{
			vm_value val_tmp = vm_getvalue(node->ds1,node->val1, vm) ;
			val2 = vm_getvalue(node->ds2,node->val2, vm) ;
			//*val1 = min(*val1, val2) ;
			vm->reg1 = vm_rand(val_tmp, val2) ;
			break ;

		}
	case EOP_LTZERO:
		{	
			//测试操作数是否小于0 ,小于0返回1 大于0 返回0 
			val2 = vm_getvalue(node->ds1,node->val1, vm) ;
			if(val2 < 0)
				vm->reg1 = (vm_value)1.0 ;
			else 
				vm->reg1 = (vm_value)0.0 ;
			break ;
		}
	case EOP_PROB:
		{
			//模拟一个概率
			vm_value vt ;
			val2 = vm_getvalue(node->ds1,node->val1, vm) ;
			vt = vm_rand(1, 100) ;
			if( vt <= (val2*(vm_value)100)  )
				vm->reg1 = (vm_value)1.0 ;
			else 
				vm->reg1 = (vm_value)0.0 ;
			break ;
		}
		break ;
	case EOP_ROUND :
	{
		val2 = vm_getvalue(node->ds1, node->val1, vm);
		if (val2 > 0) {
			vm->reg1 = (vm_value)((int)(val2 + 0.5f));
		}
		else if (val2 == 0) {
			vm->reg1 = 0;
		}
		else {
			vm->reg1 = (vm_value)((int)(val2 - 0.5f));
		}
		break;
	}
	case EOP_CEIL:
	{
		val2 = vm_getvalue(node->ds1, node->val1, vm);
		vm->reg1 = (vm_value)(ceil(val2));

		break;
	}
	case EOP_FLOOR:
	{
		val2 = vm_getvalue(node->ds1, node->val1, vm);
		vm->reg1 = (vm_value)(floor(val2));

		break;
	}

	case EOP_SQRT:
		{
			//开方
			val2 = vm_getvalue(node->ds1,node->val1, vm) ;
			if (val2 > 0){
				vm->reg1 = (vm_value)sqrt(val2) ;
			}
			else {
				vm->reg1 = 0 ;
			}
			break ;
		}
		break ;
		
	case EOP_NOP:
		//break;
		return 0;
	default:
		return -1 ;
	}
	if(vm->echo_cmd) {
		char buf[256] ;
		if(vm_output_asm(node, buf, MEDIUM_BUF) > 0 ) {
			vm_print( "%s \n", buf ) ; 
		}
	}

	//显示运行结果
	if(vm->echo_res){
		vm_echo_res(node,vm);
	}

	return 0 ;
}


static vm_value* _vm_mm_get (vm_adddress addr, struct vm_cpu *vm) 
{
	static vm_value test_mm ;
	return &test_mm ;
}

int vm_check_insnode(struct vm_instruction_node *node)
{
	struct vm_cpu vm ;
	vm_machine_init(&vm, 0, 0) ;
	
	vm_set_mmfunc(&vm,_vm_mm_get,1024*1024*1024)  ;


	return (0==vm_run_insnode(node, &vm) ) ;
}

//计算随机数
vm_value vm_rand(vm_value val1, vm_value val2)
{
	int val ;
	int range_min = (int) val1 ;
	int range_max = (int) val2 ;

	if(range_max < range_min) {
		int tmp = range_min;
		range_min = range_max ;
		range_max = tmp ;
	}
	else if(range_max == range_min)
		return (vm_value) range_max ;

	val =(int)  ((vm_value)rand() / RAND_MAX  * (range_max - range_min)	+ range_min);

	return (vm_value) val ;
}


#pragma warning(disable: 4996) 
//得到操作数的名字
char* vm_operand_name(vm_data_src ds,vm_value val, struct vm_cpu *vm, char *buf)
{
	if(EDS_REG1==ds) {
		ndstrcpy(buf, "REG1") ;
		//return &(vm->reg1);
	}
	else if(EDS_REG2==ds) {
		ndstrcpy(buf, "REG1") ;
		//return &(vm->reg2);
	}
	else if(EDS_STACK==ds) {
		//读取堆栈
		vm_adddress index = *((vm_adddress*) &val) ;
		if( index < 0  ) {
			if(vm->stack <= (vm->sp + index)) {
				//return (vm->sp + index);
				ndsprintf(buf, "STACK[%d]", index) ;
			}
		}
		else if((vm->stack+index) <  vm->sp ){
			//return &(vm->stack[index]) ;
			ndsprintf(buf, "STACK[%d]", index) ;
		}
		else
			return NULL;
		//return (vm_value)0; 
	}
	else if(EDS_ADDRESS==ds) {
		vm_adddress index = *((vm_adddress*) &val) ;
		if(index >=0 && index < (vm_adddress)vm->mem_size ) {
			ndsprintf(buf, "MEMORY[%d]", index) ;
		}
		else 
			return 0;
	}
	else {
		return 0;
	}
	return buf;
}

//显示运行结果
int vm_echo_res(struct vm_instruction_node *node,struct vm_cpu *vm)
{
	switch (node->ins){
	case EOP_MOV:		
	case EOP_ADD:		
	case EOP_SUB:		
	case EOP_MUL:		
	case EOP_DIV:
		{
			char op_name[128] ;
			vm_value val_tmp = vm_getvalue(node->ds1,node->val1, vm) ;

			if(vm_operand_name(node->ds1,node->val1, vm, op_name) ) {
				vm_print(" RESULT : %s = %f \n", op_name, val_tmp) ;		
			}

			break ;

		}
	case EOP_PUSH:
		vm_print(" RESULT : STACK[-1] = %f \n", *(vm->sp -1) );	
		break ;
	case EOP_POP:
		{
			char op_name[128] ;

			vm_value *val1 = vm_ref_operand(node->ds1,node->val1, vm) ;
			//vm_value val_tmp = vm_getvalue(node->ds1,node->val1, vm) ;
			if(val1 && vm_operand_name(node->ds1,node->val1, vm, op_name) ) {
				
				vm_print(" RESULT : %s = %f \n", op_name, *val1) ;		
			}

			break ;

		}
	case EOP_MAX:		
	case EOP_MIN:		
	case EOP_RAND :	
	case EOP_LTZERO:
	case EOP_PROB:
	case EOP_SQRT:
	case EOP_ROUND:
	case EOP_CEIL:
	case EOP_FLOOR:
		vm_print("\t REG1 =%f\n", vm->reg1) ;
		break ;
	case EOP_NOP:
		break;
	default:
		return -1 ;
	}
	return 0;

}
