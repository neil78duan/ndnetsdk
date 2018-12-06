/* file nd_formula.c
 * ��ʽ����
 * 2008-8-29  all right reserved by duan 
 * version 1.0
 */

/* 
 * �㷨����: ��������һ����ʽ�ַ���,Ȼ����ַ������зֽ�
 * �ѹ�ʽ�ֽ�Ϊһ�����Ľڵ㱣�����б���,ÿ���ڵ����������,�������Ͳ��������ȼ���.
 * �ֽ����Ժ��ÿ�����ڵ��������.
 * ���㷽�� ȡ����һ���ڵ���Ϊ��ǰ����ڵ�,����һ���ڵ�������ȼ��Ƚ�,�����һ���ڵ����ȼ���,
 * ����һ���ڵ���Ϊ��ǰ�ڵ�����ظ����ϲ���.������ȼ�һ��,�����Ѿ�����β��,��ֱ�Ӱ��������ڵ��������.
 * ���������һ���ڵ�,ͬ���������Ϸ�������.ֱ������Ϊֹ.
 *
 */


#include "nd_vm/nd_vm.h"
#include "nd_common/list.h"
#include "nd_common/nd_str.h"

#define OPERAND_SIZE  64 
typedef vm_value OPDATA_T ;

#define deta  0.00000001

#define FUNCTION_SIZE 4096
//�Ӻ�����ָ��
struct function_cmd
{
	int size ;
	char buf[FUNCTION_SIZE] ;
};

//�����������
struct sOpterateEntry
{
	int level ;				//���ȼ�
	char name ;				//
};

#define FUNC_PARAM_NUM 2 
//���������Ԫ
struct sOperateUnit{
	char op ;
	char is_function ;
	int level ;				//���ȼ�
	char operand[OPERAND_SIZE];
	struct list_head lst ;
	//struct function_cmd * func_cmd;		//����ָ��
	struct list_head param_list[FUNC_PARAM_NUM]	;	//�����б�
};

//�ֽ⺯���������Ĵ���
int parse_function(struct sOperateUnit *pUnit,  char *textbuf, char **next)  ;

//�Ѻ�������ָ��
int run_function(struct sOperateUnit *pOpUnit) ;

//���ָ���Ƿ��Ǻ���
int is_function(char *src) ;

/*�ѱ����滻�ɲ����������ڴ��ַ*/

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

//���ȼ���ֵ
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

//��ȡһ��������
//return 1 ��ȡ���ֳɹ�,2��ȡ��һ��'(' , ����0 û�ж�ȡ,����-1 ������ 
//return 3 ��ȡ��һ������
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
		//��ȡ[]�е���������
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
		//�����Ǻ���
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
		//�����Ǻ���
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

//��ȡһ����������,return 1 ��ȡ�ɹ�, ����2 ��ȡ��һ��')' ,�������ű�����ch��,
//����0 û�ж�ȡ����,����-1����
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

//�ѱ��ʽ�ֽ�ɲ����ڵ�
//���������ŵ�header������
int parse_express_2node(char *textBuf, struct list_head *header) 
{
	char chop =0;
	int ret ;
	int bReadData = 1 ;		//��ȡ����,���ȶ�ȡ��ֵ
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
				bReadData = 0 ;		//�´ζ���������
			}
			else if(2==ret) {
				current_level += LEVEL_BRACKET ;
			} 
			else if(3==ret) {
				//��ȡ��һ������, ��������

				opUnit = alloc_operator() ;
				if(!opUnit) {
					return-1 ;		//input too much
				}

				ndstrncpy(opUnit->operand, op_data,sizeof(opUnit->operand)) ;
				list_add_tail(&opUnit->lst, header) ;
				bReadData = 0 ;		//�´ζ���������

				opUnit->is_function = 1 ;
				//�ֽ⺯���������Ĵ���
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
				
				bReadData = 1 ;	//�´ζ�����
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

//ִ�������ڵ������(opUnit)��ǰ�ڵ���������һ���ڵ�����)
OPDATA_T calcNodeResult(struct list_head *curr_pos, struct list_head *lst_end,int depth)
{
	int cur_level ;			//��ǰ�������ȼ�
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
		//���ֻ��һ���ڵ�,��������ִ��
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

		if(pOpUnit->level == pOpNext->level || !pOpNext->op ) {		//������ȿ�������,�����ֱ�Ӱ�����ڵ㶪�����ߺ���û�в�������	


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
			//�������ȼ�,			
			asm_printf( "\t PUSH REG1 \n") ;
			data = calcNodeResult(next, lst_end,depth+1) ;
			//�ѽ���ŵ�REG2��
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

//ִ�б��ʽ,�ѽڵ�ת����asm
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

//��list���Ϊ������ָ��,���浽������
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
//������ʽ���
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
 * ����������ڲ�����
 */

char *__inter_func[] = 
{
	"MAX", "MIN", "RAND", "LTZERO", "PROB", "SET", "SQRT", "ROUND"
} ;

//����Ƿ����ڲ�����
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

//�õ������ĸ���
int get_fun_params(char *func_name)
{
	if(0==ndstricmp(func_name, "LTZERO") || 
		0==ndstricmp(func_name, "PROB") ||
		0==ndstricmp(func_name, "SQRT") ||
		0 == ndstricmp(func_name, "ROUND") 
		)  {
		return 1 ;
	}
	else 
		return 2 ;
}

//��ȡ�����Ĳ���,֪������ ',' ���� )
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

//�ֽ⺯���������Ĵ���
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

//ִ���ڲ�����
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
