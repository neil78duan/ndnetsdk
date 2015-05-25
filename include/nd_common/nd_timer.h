/* file nd_timer.h
 * header file timer of nd engine 
 *
 * all right reserved 2009
 * create time 2009-2-24 14:38
 */

#ifndef _ND_TIMER_H_
#define _ND_TIMER_H_

typedef unsigned int ndtimer_t ;
typedef int (*nd_timer_entry)( void *param) ;

enum eTimeType
{
	ETT_LOOP = 0 ,			//循环执行的定时器
	ETT_ONCE ,				//执行一次的定时器
	ETT_DESTROY				//定时器被销毁时执行
};

/* 增加一个计时执行函数*/
ND_COMMON_API ndtimer_t nd_timer_add(nd_handle handle,nd_timer_entry func,void *param,ndtime_t interval, int run_type );
ND_COMMON_API int nd_timer_del(nd_handle handle, ndtimer_t id) ;

ND_COMMON_API int  nd_timer_destroy(nd_handle timer_handle, int force) ;
/* create timer root */
ND_COMMON_API nd_handle nd_timer_create(nd_handle pallocator) ;
ND_COMMON_API int  nd_timer_update(nd_handle handle)  ;
#endif
