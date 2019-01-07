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
	ETT_LOOP = 0 ,			//loop timer
	ETT_ONCE ,				//only run once timer
	ETT_DESTROY				//run on destroied 
};

/* add / delete timer*/
ND_COMMON_API ndtimer_t nd_timer_add(nd_handle handle,nd_timer_entry func,void *param,ndtime_t interval, int run_type );
ND_COMMON_API int nd_timer_del(nd_handle handle, ndtimer_t id) ;

ND_COMMON_API int  nd_timer_destroy(nd_handle timer_handle, int force) ;
/* create timer root */
ND_COMMON_API nd_handle nd_timer_create(nd_handle pallocator) ;
ND_COMMON_API int  nd_timer_update(nd_handle handle)  ;
#endif
