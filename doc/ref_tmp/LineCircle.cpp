/* file LineCircle.cpp
 *
 * line inter circle
 *
 * 计算直线和球是否相交
 * 来源: http://blog.csdn.net/rabbit729/archive/2009/06/20/4285119.aspx
 */
  
#include <math.h>
 
struct point2f   
{
	float x;  
	float y;    
};  

float InvSqrt (float x)
{
	float xhalf = 0.5f*x;
	int i = *(int*)&x;
	i = 0x5f3759df - (i >> 1); // 计算第一个近似根
	x = *(float*)&i;
	x = x*(1.5f - xhalf*x*x); // 牛顿迭代法
	return x;
}

double InvSqrt(double number)
{
	double x2, y;
	const double threehalfs = 1.5F;
	union
	{
		double d;
		__int64 i;
	}d;
	x2 = number * 0.5F;
	y = number;
	d.d =  y;
	d.i = 0x5fe6ec85e7de30da - (d.i >> 1);
	y = d.d;
	y = y * (threehalfs - (x2 * y * y)); //1st iteration
	y = y * (threehalfs - (x2 * y * y)); //2nd iteration, this can be removed
	return y;
}

/** 
* @brief 求线段与圆的交点 
* @return 如果有交点返回true,否则返回false 
* @note 与圆可能存在两个交点，如果存在两个交点在ptInter1和ptInter2都为有效值，如果有一个交点，则ptInter2的值为 
*       无效值，此处为65536.0 
*/  
int LineInterCircle(  
               point2f *ptStart, // 线段起点  
               point2f *ptEnd,	// 线段终点  
               point2f *ptCenter, // 圆心坐标  
               float Radius/*,  
               point2f& ptInter1,  
               point2f& ptInter2*/)  
{  
	//ptInter1.x = ptInter2.x = 65536.0f;  
	//ptInter2.y = ptInter2.y = 65536.0f;  
	float a, a2, e2, r2;
	point2f E,d;
	float fDis = InvSqrt((ptEnd->x - ptStart->x) * (ptEnd->x - ptStart->x) + (ptEnd->y - ptStart->y) * (ptEnd->y - ptStart->y));  
	
	d.x = (ptEnd->x - ptStart->x) * fDis;  
	d.y = (ptEnd->y - ptStart->y) * fDis;  
	
	E.x = ptCenter->x - ptStart->x;  
	E.y = ptCenter->y - ptStart->y;  
	
	a = E.x * d.x + E.y * d.y;  
	a2 = a * a;  
	
	e2 = E.x * E.x + E.y * E.y;  
	
	r2 = Radius * Radius;  
	
	if ((r2 - e2 + a2) < 0) {  
		return 0;  
	}  
	else  
	{  
		/*float f = sqrt(r2 - e2 + a2);  
		
		float t = a - f;  
		
		if( ((t - 0.0) > - EPS) && (t - fDis) < EPS)  	{  
			ptInter1.x = ptStart.x + t * d.x;  
			ptInter1.y = ptStart.y + t * d.y;  
		}         
		
		t = a + f;  
		
		if( ((t - 0.0) > - EPS) && (t - fDis) < EPS)  {  
			ptInter2.x = ptStart.x + t * d.x;  
			ptInter2.y = ptStart.y + t * d.y;  
		}  
		*/
		return 1;  
	}  
}  

int LineInterCircle2( float ptStart[2],float ptEnd[2],float ptCenter[2], float Radius)
{
	return LineInterCircle( (point2f *)ptStart, (point2f *)ptEnd,(point2f *)ptCenter,  Radius) ;
}

/*

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
			strcpy_s(pOpNext->operand, sizeof(pOpNext->operand), "REG2");
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
*/