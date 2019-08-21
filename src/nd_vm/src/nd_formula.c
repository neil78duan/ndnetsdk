/* file nd_formula.c
 * 公式翻译
 * 2008-8-29  all right reserved by duan 
 * version 1.0
 */

/* 
 * 算法描述: 首先输入一个公式字符串,然后对字符串进行分解
 * 把公式分解为一个个的节点保存在列表中,每个节点包含操作数,操作符和操作符优先级别.
 * 分解完以后对每两个节点进行运算.
 * 运算方法 取出第一个节点作为当前运算节点,和下一个节点进行优先级比较,如果下一个节点优先级高,
 * 把下一个节点作为当前节点进行重复以上操作.如果优先级一样,或者已经到了尾部,则直接把者两个节点进行运算.
 * 如果还有下一个节点,同样依照以上方法运算.直到算完为止.
 *
 */


#include "nd_vm/nd_vm.h"
#include "nd_common/list.h"
#include "nd_common/nd_str.h"

#define OPERAND_SIZE  64 
typedef vm_value OPDATA_T ;

#define deta  0.00000001

#define FUNCTION_SIZE 4096
//子函数的指令
struct function_cmd
{
	int size ;
	char buf[FUNCTION_SIZE] ;
};

//操作函数入口
struct sOpterateEntry
{
	int level ;				//优先级
	char name ;				//
};

#define FUNC_PARAM_NUM 2 
//定义操作单元
struct sOperateUnit{
	char op ;
	char is_function ;
	int level ;				//优先级
	char operand[OPERAND_SIZE];
	struct list_head lst ;
	//struct function_cmd * func_cmd;		//函数指令
	struct list_head param_list[FUNC_PARAM_NUM]	;	//参数列表
};

//分解函数所包含的代码
int parse_function(struct sOperateUnit *pUnit,  char *textbuf, char **next)  ;

//把函数生成指令
int run_function(struct sOperateUnit *pOpUnit) ;

//检测指令是否是函数
int is_function(char *src) ;

/*把变量替换成操作数或者内存地址*/

int repace_param(char *param, char *buf, int size) ;

//static __INLINE__ void init_funcdata(struct function_cmd *f_cmd)
//{
//	f_cmd->size = 0;
//}

static __INLINE__ void  initOperateUnit(struct sOperateUnit *pOpUnit)
{
	pOpUnit->op = ' ';
	pOpUnit->is_function  = 0 ;
	pOpUnit->operand[0] = 0 ;
	INIT_LIST_HEAD(&pOpUnit->lst) ;
	INIT_LIST_HEAD(&pOpUnit->param_list[0]) ;
	INIT_LIST_HEAD(&pOpUnit->param_list[1]) ;
	//INIT_LIST_HEAD(&pOpUnit->children) ;
}

//优先级数值
enum {
	LEVEL_EVALUEATE = 0,
	LEVEL_ADD ,
	LEVEL_MUL ,
	LEVEL_BRACKET
};
struct sOpterateEntry __opentry[] = 
{
	{0, '='} ,
	{1, '+'} ,
	{1, '-' } ,
	{2, '*' } ,
	{2, '/' } ,
	{3, '(' } ,
	{3, ')' }
};


int asm_printf(const char *stm, ...) ;

void out_op_asm(char op , char *operand1, char *operand2)
{
	
	switch(op) {
	
	case '+' :
		asm_printf("\t ADD %s , %s \n" , operand1, operand2) ;
		break ;
	case '-' :
		asm_printf( "\t SUB %s , %s \n", operand1, operand2) ;
		break ;
	case '*' :
		asm_printf( "\t MUL %s , %s \n" , operand1, operand2) ;
		break ;
	case '/' :
		asm_printf( "\t DIV %s , %s \n", operand1, operand2) ;
		break ;
	case '=':
		asm_printf( "\t MOV %s , %s \n", operand1, operand2) ;
		break ;
	}
}
int GetOpLevel(char op)
{
	int i;
	for (i=0; i<sizeof(__opentry)/sizeof(struct sOpterateEntry); i++)
	{
		if(op==__opentry[i].name) {
			return __opentry[i].level ;
		}
	}
	return -1;
}


#define OPUNIT_NUM 1024
struct sOperateUnit __opbuf[OPUNIT_NUM] ;
ND_LIST_HEAD(__free_operate) ;

void init_operator_buf()
{
	int  i ;
	INIT_LIST_HEAD(&__free_operate) ;
	for (i=0; i<OPUNIT_NUM; i++){
		initOperateUnit(&__opbuf[i]) ;
		list_add_tail(&(__opbuf[i].lst), &__free_operate) ;
	}
}

struct sOperateUnit *alloc_operator()
{
	struct sOperateUnit *punit ;
	struct list_head *pos = __free_operate.next ;
	if(pos == &__free_operate) {
		return NULL ;
	}
	list_del_init(pos);

	punit = list_entry(pos,struct sOperateUnit, lst) ;
	punit->level = 0 ;

	punit->op = 0 ;

	return punit;
}

void free_operator(struct sOperateUnit *op)
{
	list_del(&op->lst) ;
	initOperateUnit(op);
	list_add(&op->lst, &__free_operate) ;
}

//读取一个操作数
//return 1 读取数字成功,2读取了一个'(' , 返回0 没有读取,返回-1 出错了 
//return 3 读取了一个函数
//int readData(char *textbuf, OPDATA_T *outData, char **next) 
int read_operand(const char *textbuf,char* operand_buf, char **next) 
{
	int readOk ;
	const char *error_addr = textbuf;
		
	textbuf = ndstr_first_valid(textbuf) ;
	if(!textbuf){
		*next = 0 ;
		return 0 ;
	}
	if('('==*textbuf) {
		*next =(char*)( ++textbuf );
		return 2 ;
	}
	else if('['==*textbuf) {
		//读取[]中的所有数据
		int len = 0;
		operand_buf[0] = 0 ;
		textbuf = ndstr_str_end(textbuf, operand_buf, ']') ;
		if(!textbuf || !*textbuf || *textbuf != ']') {
			*next = (char*) textbuf ;

			vm_error("syntax error:  miss ']' in %s  \n", error_addr) ;
			return -1 ;
		}
		if(!ndstr_is_naturalnumber(operand_buf+1)) {
			*next = (char*)textbuf;

			vm_error("syntax error: int [] must be numbers, %s  \n", error_addr) ;
			return -1 ;
		}

		len = (int) ndstrlen(operand_buf) ;
		operand_buf[len] = *textbuf++ ;
		operand_buf[len+1]  = 0 ;


	}
	/*else if(IS_LITTLE_LATIN(*textbuf) || IS_BIG_LATIN(*textbuf)) {
		//可能是函数
		textbuf = ndstr_parse_word(textbuf, operand_buf) ;
		if(is_function(operand_buf) ) {
			*next = textbuf ;
			return 3 ;
		}
		vm_error("bad operand [%s]  \n", operand_buf) ;
		return -1 ;
	}*/

	else if(IS_NUMERALS(*textbuf) || _DOT==*textbuf || _MINUS==*textbuf || *textbuf=='+' ) {
		textbuf = ndstr_read_numerals(textbuf, operand_buf, &readOk) ; 
		if(! readOk){
			*next = (char*)textbuf;
			vm_error("bad operand [%s] is not numeral  \n", error_addr) ;			
			return -1 ;
		}
	}
	else {
		//可能是函数
		char optmp[OPERAND_SIZE] ;
		textbuf = ndstr_parse_word(textbuf, operand_buf) ;
		if(is_function(operand_buf) ) {
			*next = (char*)textbuf;
			return 3 ;
		}
		else if(0==repace_param(operand_buf,optmp, sizeof(optmp) ) ){
			ndstrncpy(operand_buf, optmp ,OPERAND_SIZE) ;
		}
		else {
			vm_error("bad operand [%s]  \n", operand_buf) ;
			return -1 ;

		}
	}

	
	//*(++operand_buf) = 0 ;
	//if(*textbuf)
	*next = (char*)textbuf;
	
	return 1 ;
}

//读取一个操作符号,return 1 读取成功, 返回2 读取了一个')' ,操作符号保存在ch中,
//返回0 没有读取内容,返回-1出错
int readOperator(char *textbuf, char *ch, char**next) 
{
	textbuf = (char *) ndstr_first_valid(textbuf) ;
	if(!textbuf){
		*next = NULL;
		return 0 ;
	}

	if(')'==*textbuf) {
		*next= ++textbuf ;
		return 2 ;
	}
	if('+'==*textbuf || '-'==*textbuf || '*'==*textbuf || '/'==*textbuf || '='==*textbuf) {
		*ch = *textbuf++ ;
		*next = textbuf ;
		return 1 ;
	}

	vm_error("syntax error :Unknown operator '%c' \n", *textbuf) ;
	return -1 ;
}

//把表达式分解成操作节点
//包操作数放到header队列中
int parse_express_2node(char *textBuf, struct list_head *header) 
{
	char chop =0;
	int ret ;
	int bReadData = 1 ;		//读取类型,首先读取数值
	int current_level = 0 ;
	char *nextaddr = textBuf ;
	struct sOperateUnit *opUnit =0;

	while (*nextaddr){
		if(bReadData) {
			//OPDATA_T opdata ;
			char op_data[sizeof(opUnit->operand)] ;
			opUnit = NULL ;
			ret = read_operand(nextaddr,op_data, &nextaddr) ;
			
			if(-1==ret) {
				return -1 ;		//input error
			}
			else if(0==ret) {
				break ;			//read end 
			}
			else if(1==ret ) {
				
				opUnit = alloc_operator() ;
				if(!opUnit) {
					return-1 ;		//input too much
				}

				ndstrncpy(opUnit->operand, op_data,sizeof(opUnit->operand)) ;
				list_add_tail(&opUnit->lst, header) ;
				bReadData = 0 ;		//下次读操作符号
			}
			else if(2==ret) {
				current_level += LEVEL_BRACKET ;
			} 
			else if(3==ret) {
				//读取了一个函数, 解析函数

				opUnit = alloc_operator() ;
				if(!opUnit) {
					return-1 ;		//input too much
				}

				ndstrncpy(opUnit->operand, op_data,sizeof(opUnit->operand)) ;
				list_add_tail(&opUnit->lst, header) ;
				bReadData = 0 ;		//下次读操作符号

				opUnit->is_function = 1 ;
				//分解函数所包含的代码
				if(0!=parse_function(opUnit, nextaddr, &nextaddr) ) {
					return -1 ;
				}

			}

			if(0==nextaddr) {
				break ;
			}
		}
		else{
			ret = readOperator(nextaddr, &chop, &nextaddr) ;
			if(-1==ret) {
				return -1 ;		//input error
			}
			else if(0==ret) {
				break ;			//read end 
			}
			else if(1==ret) {
				opUnit->op = chop ;
				opUnit->level = current_level + GetOpLevel(chop) ;
				
				bReadData = 1 ;	//下次读数据
			}
			else if(2==ret) {
				current_level -= LEVEL_BRACKET ;
			} 
		}

	}			//end for
	
	if(current_level) {
		//INPUT error short right bracket
		return -1 ;
	}
	if(bReadData==1 &&opUnit && opUnit->op) {
		//INPUT error 
		return -1 ;
	}
	return 0 ;
}

//执行两个节点的运行(opUnit)当前节点与它的下一个节点运算)
OPDATA_T calcNodeResult(struct list_head *curr_pos, struct list_head *lst_end,int depth)
{
	int cur_level ;			//当前运算优先级
	int is_recursion = 0 ;
	struct list_head *next ;
	struct sOperateUnit *pOpUnit, *pOpNext ;
	OPDATA_T data ;
	if(curr_pos==lst_end){
		return (OPDATA_T)0 ;
	}
	pOpUnit = list_entry(curr_pos,struct sOperateUnit, lst) ;
	next = curr_pos->next ;
	cur_level = pOpUnit->level ;
	
	if(pOpUnit->is_function) {
		//如果只有一个节点,将在这里执行
		if(0!=run_function(pOpUnit)  ){
			return -1 ;
		}
		pOpUnit->is_function = 0 ;
	}
	else{
		asm_printf( "\t MOV REG1, %s\n", pOpUnit->operand) ;

	}	

	while (next!= lst_end){
		pOpNext = list_entry(next,struct sOperateUnit, lst) ;

		if(pOpNext->is_function) {

			asm_printf( "\t PUSH REG1 \n") ;
			if(0!=run_function(pOpNext)  ){
				return -1 ;
			}
			asm_printf( "\t MOV REG2, REG1 \n") ;
			ndstrncpy(pOpNext->operand, "REG2", sizeof(pOpNext->operand));
			asm_printf( "\t POP REG1 \n" ) ;
			pOpNext->is_function = 0 ;
		}

		if(pOpUnit->level == pOpNext->level || !pOpNext->op ) {		//级别相等可以运算,运算后直接把这个节点丢弃或者后面没有操作符了	


			if(is_recursion) {
				out_op_asm(pOpUnit->op, "REG1", "REG2") ;
				//out_op_asm(pOpUnit->op, "[SP -1]", "REG1") ;
				//asm_printf( "\t POP REG1 \n" ) ;
			}
			else {
				out_op_asm(pOpUnit->op, "REG1", pOpNext->operand) ;
			}
			
			free_operator(pOpUnit) ;

			is_recursion = 0 ;
			if(!pOpNext->op)
				return 0;
		}
		else if(pOpUnit->level > pOpNext->level){
			if(is_recursion) {
				
				out_op_asm(pOpUnit->op, "REG1", "REG2") ;
				//asm_printf( "\t POP REG1 \n" ) ;
				
			}
			else {
				out_op_asm(pOpUnit->op, "REG1", pOpNext->operand) ;
			}
			is_recursion = 0 ;
			free_operator(pOpUnit) ;
			
			if(depth) {
				struct list_head *prev = pOpNext->lst.prev ;
				struct sOperateUnit *prevOp = list_entry(prev,struct sOperateUnit, lst) ;
				if(prevOp->level > pOpNext->level)
					return 0 ;
			}
		}
		else {
			//遇到优先级,			
			asm_printf( "\t PUSH REG1 \n") ;
			data = calcNodeResult(next, lst_end,depth+1) ;
			//把结果放到REG2中
			asm_printf( "\t MOV REG2 , REG1 \n") ;
			asm_printf( "\t POP REG1 \n") ;
			

			next = pOpUnit->lst.next;
			is_recursion = 1 ;
			continue ;
		}
		curr_pos = next ;
		next = curr_pos->next ;
		
		pOpUnit = list_entry(curr_pos,struct sOperateUnit, lst) ;

	}
	return 0;//pOpUnit->data ;
}

//执行表达式,把节点转换成asm
int run_experssion(struct list_head * start_pos,struct list_head *header)
{
	struct sOperateUnit *pOpUnit, *pOpNext;

	if(start_pos==header){
		return -1;
	}

	pOpUnit = list_entry(start_pos,struct sOperateUnit, lst) ;
	
	if(pOpUnit->op == '=') {
		struct list_head *pnext = start_pos->next ;
		pOpNext = list_entry(pnext, struct sOperateUnit, lst) ;
		
		if(pOpNext->op== '=') {
			run_experssion(pnext, header) ;
		}
		else {
			calcNodeResult(pnext, header,0) ;			
		}
		
		if(-1==asm_printf( "\t MOV  %s, REG1\n", pOpUnit->operand) ) {
			return -1;
		}
		
	}
	else {
		calcNodeResult(start_pos, header,0) ;
	}
	return 0 ;
}

enum e_asm_type{
	TYPE_ASM = 0 ,
	TYPE_OP_BIN =1
};

struct formula_out_info
{
	int medium ;		//ref e_medium
	int type ;			//ref e_asm_type
	size_t buf_size ;
	vm_param_replace_func param_parse_func ;
	void *parse_user_data ;
	void *out_handle ;
};

__ndthread struct formula_out_info  __asm_out ;

//把list输出为二进制指令,保存到缓冲中
size_t oplist_2buf(struct list_head *h, char *outbuf)
{

	__asm_out.medium = MEDIUM_BUF ;
	__asm_out.type = TYPE_OP_BIN ;
	__asm_out.out_handle = outbuf ;

	if(-1==run_experssion(h->next, h) ) {
		return 0;
	}

	return ( (char*)(__asm_out.out_handle) - outbuf) ;
}

int repace_param(char *param, char *buf, int size)
{
	if(__asm_out.param_parse_func) {
		return __asm_out.param_parse_func(param, buf, size,__asm_out.parse_user_data) ;
	}
	return -1 ;
}
//计算表达式结果
size_t vm_parse_expression(const char *textbuf, char *code_buf,size_t buf_size,vm_param_replace_func func, void*user_data)
{
	int ret ;

	ND_LIST_HEAD(op_list) ;

	__asm_out.param_parse_func = func ;
	__asm_out.buf_size = buf_size ;
	__asm_out.parse_user_data = user_data;
	init_operator_buf() ;

	ret = parse_express_2node((char*)textbuf, &op_list)  ;
	if(-1==ret) {
		return 0 ;
	}
#if 0
	list_for_each(pos, &op_list) {
		struct sOperateUnit *pOpUnit = list_entry(pos,struct sOperateUnit, lst) ;
		ndprintf("operand=%s, op = %c ,level=%d\n" AND pOpUnit->operand AND pOpUnit->op AND pOpUnit->level) ;	
	}
#endif 


	return oplist_2buf(&op_list,code_buf) ;
}

int asm_printf(const char *stm, ...)
{
	size_t done = 0;
	va_list arg;

	char buf[128] ;
	
	va_start (arg, stm);
	done = ndvsnprintf (buf,sizeof(buf), stm, arg);
	va_end (arg);
	

	if(TYPE_OP_BIN == __asm_out.type) {
		struct vm_instruction_node  ins_node = {0};
		int ret = vm_compiler_line(buf ,  &ins_node) ;
//		ndfprintf(stdout, "%s", buf) ;
		if(ret > 0 ) {
			done = vm_instruct_2buf(&ins_node, buf) ;
			if(done > 0 && done< sizeof(buf)) {
				buf[done] = 0 ;
			}
			else 
				return -1 ;			
		}
		else if(-1==ret) {
			return -1;
		}
	}

	if(MEDIUM_BUF==__asm_out.medium ) {
		if(__asm_out.buf_size >= done) {
			memcpy(__asm_out.out_handle, buf, done) ;
			__asm_out.out_handle = ((char*)__asm_out.out_handle) + done ;
			__asm_out.buf_size -= done ;

		}
		else {
			return -1;
		}
	}
	else {
		ndfprintf((FILE*)__asm_out.out_handle, "%s", buf) ;
	}
	//ndfprintf(stdout, "%s", buf) ;
	//int vm_compiler_line(char *text , struct vm_instruction_node *out_node)
	return 0 ;
}

/* parse function 
 * 解析表达是内部函数
 */

char *__inter_func[] = 
{
	"MAX", "MIN", "RAND", "LTZERO", "PROB", "SET", "SQRT", "ROUND","CEIL","FLOOR"
} ;

//检测是否是内部函数
int is_function(char *src)
{
	int i ;
	for(i=0; i<sizeof(__inter_func)/sizeof(__inter_func[0]); i++) {
		if(0==ndstricmp(__inter_func[i], src)) {
			return 1 ;
		}
	}

	return 0 ;
}

//得到参数的个数
int get_fun_params(char *func_name)
{
	if(0==ndstricmp(func_name, "LTZERO") || 
		0==ndstricmp(func_name, "PROB") ||
		0==ndstricmp(func_name, "SQRT") ||
		0 == ndstricmp(func_name, "ROUND") ||
		0 == ndstricmp(func_name, "CEIL") ||
		0 == ndstricmp(func_name, "FLOOR") 		)  {
		return 1 ;
	}
	else 
		return 2 ;
}

//读取函数的参数,知道遇到 ',' 或者 )
char *read_func_param(char *textbuf, char* param_buf, int *is_ok)
{
	*is_ok = 0 ;
	while(*textbuf) {
		if( ')'== *textbuf) {
			if(*is_ok > 0) {
				--(*is_ok)  ;
			}
			else {
				//return textbuf ;
				break ;
			}
		}
		else if(','==*textbuf && *is_ok==0) {
			//return textbuf ;
			break ;
		}
		else if('('==*textbuf) {
			++(*is_ok) ;
		}
		
		*param_buf++ = *textbuf++ ;		
	}
	*param_buf = 0 ;
	return textbuf ;
}

//分解函数所包含的代码
int parse_function(struct sOperateUnit *pUnit,  char *textbuf, char **next) 
{
	int num = 0 ;
	char * error_addr = textbuf ;
	char f_param[FUNCTION_SIZE] ;

	textbuf =(char*) ndstr_first_valid(textbuf) ;
	if(!textbuf || *textbuf != '(') {
		vm_error("syntax error : miss '(' \n") ;
		return -1 ;
	}
	++textbuf ;
	
	do {
		int is_ok = 0 ;
		textbuf = read_func_param(textbuf, f_param, &is_ok) ;
		if(0!=is_ok) {	//on error
			*next = textbuf ;
			return -1 ;
		}
		if(0!=parse_express_2node(f_param, &pUnit->param_list[num])  ) {
			*next = textbuf ;

			return -1 ;
		}
		++num  ;
		if(','==*textbuf) {
			textbuf++ ;
		}
		else if(')'==*textbuf) {
			++textbuf;
			break ;
		}
	}while(textbuf && num < FUNC_PARAM_NUM) ;

	if(num !=  get_fun_params(pUnit->operand)) {

		vm_error("syntax error : miss param IN %s", error_addr) ;
		return -1 ;
	}
	*next = textbuf ;

	return 0 ;

}

//执行内部函数
int run_function(struct sOperateUnit *pOpUnit)
{
	int i ;
	int num = 0 ;
	struct list_head *start ;
	//asm_printf( "\t PUSH   REG1\n" ) ;
	int param_num = get_fun_params(pOpUnit->operand) ;

	for ( i=0; i<param_num ; i++){
		start = pOpUnit->param_list[i].next ; 
		if(-1==run_experssion(start,&(pOpUnit->param_list[i])) ) {
			return -1 ;
		}
		if(param_num> 1)
			asm_printf( "\t PUSH   REG1\n" ) ;
		++num ;
	}

	if(0==ndstricmp(pOpUnit->operand, "SET")) {
		struct sOperateUnit *param_node ;
		start = pOpUnit->param_list[0].next ;
		param_node = list_entry(start, struct sOperateUnit, lst) ;

		asm_printf( "\t MOV %s, REG1\n", param_node->operand ) ;
	}
	else if(1==num) {
		asm_printf( "\t %s  REG1\n", pOpUnit->operand ) ;
	}
	else {
		asm_printf( "\t %s   [SP -2], [SP -1]\n", pOpUnit->operand ) ;
	}
	//if()

	if(num> 1) {
		for(i=0; i<num; i++) {
			asm_printf( "\t POP \n" ) ;
		}
	}
	//ndstrncpy(pOpUnit->operand, "REG2", sizeof(pOpUnit->operand));
	//asm_printf( "\t POP REG1 \n" ) ;
	//ndsnprintf(pOpUnit->operand, sizeof(pOpUnit->operand),)
	return 0 ;
	
}
