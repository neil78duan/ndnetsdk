/* file pg_alarms.cpp
 *
 *
 * create by duan
 *
 * 2014-9-29
 *
 */

#include "ndapplib/nd_alarms.h"

#include "ndapplib/applib.h"

#define ALARM_HANDLE_ID 32
struct alarm_node
{
    ND_OBJ_BASE ;
    struct list_head list ;
    NDINT32 timeval ;
    NDObject *target ;
    NDObjectFunc timer_func ;
};


NDAlarm::NDAlarm()
{
	ND_TRACE_FUNC() ;
    m_lastTickTm = nd_time() ;
    m_tminterval =0 ;
	m_tick_minute = 0;
	m_tick_sec = 0;
	m_second_index = 0;
	m_minute_index = 0;
	m_tick_index = 0;

// 	m_daily_hour = 0 ;
// 	m_week_hour = 0 ;
// 	m_week_day = 0 ;
// 	m_daily_last_runday = 0xff;
// 	m_week_last_runweek = 0 ;
// 
// 	//m_alarm_serial = 0 ;
// 
// 	m_daily_minute = 0 ;
// 	m_week_minute = 0;
    

    INIT_LIST_HEAD(&sub_list) ;
	Create() ;
}
NDAlarm::~NDAlarm()
{

}

int NDAlarm::Create()
{
	ND_TRACE_FUNC() ;
	time_t now ;
	struct tm *gtm ;

	app_inst_time(&now) ;
	gtm = localtime( &now );
// 	
// 	if ((gtm->tm_hour > m_daily_hour) || (gtm->tm_hour == m_daily_hour&& gtm->tm_min >= m_daily_minute)) {		
// 		m_daily_last_runday = gtm->tm_mday ;
// 	}
// 	if (m_week_day == gtm->tm_wday ){
// 		NDUINT32 wkindex = (NDUINT32)(now / (60*60 * 24 *7)) ;
// 		
//         if ((gtm->tm_hour > m_week_hour) || (gtm->tm_hour == m_week_hour&& gtm->tm_min >= m_week_minute)) {
// 			m_week_last_runweek = wkindex;
// 		}		
// 	}	
	return 0;
}

void NDAlarm::Destroy(int flag)
{
	ND_TRACE_FUNC() ;

    
    struct list_head *pos, *next ;
    list_for_each_safe(pos,next, &sub_list) {
        alarm_node *node = list_entry(pos,alarm_node, list) ;
        removeAlarm((nd_handle)node) ;
    }
    INIT_LIST_HEAD(&sub_list) ;
}

nd_handle NDAlarm::addAlarm(NDObject *target, NDObjectFunc func, ndtime_t delay)
{
    //nd_handle pool = GetMmpool()  ;
    //alarm_node *node = (alarm_node *)nd_pool_alloc(pool, sizeof(alarm_node) );
	alarm_node *node = (alarm_node *)malloc(sizeof(alarm_node)) ;
    if (!node){
        return NULL ;
    }
    memset(node, 0, sizeof(alarm_node));
    node->size = sizeof(alarm_node) ;
    node->type = ALARM_HANDLE_ID ;
    node->__created = 1 ;
    
    node->target = target ;
    node->timeval = delay ;
    node->timer_func = func ;
    
    INIT_LIST_HEAD(&(node->list)) ;
    list_add(&node->list, &sub_list);
    
    return (nd_handle) node ;
    
}
int NDAlarm::removeAlarm(nd_handle hTimer)
{
    //nd_handle pool = GetMmpool() ;
    alarm_node *node = (alarm_node *) hTimer ;
    nd_assert(node);
    nd_assert(node->size == sizeof(*node)) ;
    nd_assert(node->type == ALARM_HANDLE_ID) ;
    
    list_del(&node->list) ;
    //nd_pool_free(pool, node) ;
	free(node);
    return 0;
}

int NDAlarm::Update()
{
    ND_TRACE_FUNC() ;
    ndtime_t now = nd_time() ;
    ndtime_t tminterval = now - m_lastTickTm ;
    
    m_lastTickTm = now ;
    
    return tick(tminterval);
    
}

int NDAlarm::tick(ndtime_t tminterval)
{
    ND_TRACE_FUNC() ;
    m_tminterval = tminterval ;
    
    m_tick_minute += tminterval ;
    m_tick_sec += tminterval ;
    ++m_tick_index ;
    UpdateFrame(tminterval) ;
    if (m_tick_sec >= 1000){
        if(-1==UpdateSecond())
            return -1 ;
        
        if (m_tick_sec > 2000)	{
            m_tick_sec = 0 ;
        }
        else {
            m_tick_sec -= 1000 ;
        }
        ++m_second_index ;
        if (m_tick_minute >= 1000 * 60) {
            if(-1==UpdateMinute() )
                return -1;
            
			++m_minute_index;
			if (m_minute_index % 60 == 0) {
				UpdateHour();
			}

            m_tick_minute -= 1000*60 ;
            //update_alarm() ;
        }        
    }
    
    if (list_empty(&sub_list)) {
        update_timer(tminterval);
    }
    return 0;
}


void NDAlarm::update_timer(ndtime_t tminterval)
{
    struct list_head *pos, *next ;
    list_for_each_safe(pos,next, &sub_list) {
        alarm_node *node = list_entry(pos,alarm_node, list) ;
        nd_assert(node) ;
        
        node->timeval -= tminterval ;
        if (node->timeval <= 1) {
            NDObject *obj = node->target ;
            (obj->*(node->timer_func)) () ;
            removeAlarm((nd_handle) node);
        }
        
    }
}
// 
// void NDAlarm::update_alarm()
// {
// 	ND_TRACE_FUNC() ;
// 	time_t now ;
// 	struct tm *gtm ,tmnow;
// 
// 	app_inst_time(&now) ;
// 	gtm = localtime( &now );
// 	tmnow = *gtm;
// 	gtm = &tmnow;
// 	
// 	if ( gtm->tm_mday!= m_daily_last_runday){
// 		if ((gtm->tm_hour > m_daily_hour) || (gtm->tm_hour == m_daily_hour&& gtm->tm_min >= m_daily_minute)) {
// 			UpdateDaily() ;
// 			m_daily_last_runday = gtm->tm_mday ;
// 		}
// 	}
// 	
// 	NDUINT32 wkindex = (NDUINT32)(now / (60*60 * 24 *7)) ;
// 	
//     if (m_week_day == gtm->tm_wday ){
// 		if (m_week_last_runweek != wkindex){
// 			if ((gtm->tm_hour > m_week_hour) || (gtm->tm_hour == m_week_hour&& gtm->tm_min >= m_week_minute)) {
// 				UpdateWeek() ;
// 				m_week_last_runweek = wkindex;
// 			}			
// 		}
// 	}
// }

int NDAlarm::UpdateFrame(ndtime_t tminterval)
{
    return 0;
}

int NDAlarm::UpdateSecond( )
{
	return 0;
}

int NDAlarm::UpdateMinute()
{
	return 0;
}

int NDAlarm::UpdateHour()
{
	return 0;
}


void NDAlarm::UpdateDaily(const char *eventName, void* p, int renewTimes)
{

}
void NDAlarm::UpdateWeek(const char *eventName, void* p)
{

}

void NDAlarm::UpdateMonth(const char *eventName, void* p)
{

}
// 
// void NDAlarm::SetDailyTime(int hour_index,int minute_index)
// {
// 	ND_TRACE_FUNC() ;
// 	m_daily_hour = (NDUINT8) hour_index ;
// 	if (m_daily_hour > 23){
// 		m_daily_hour = 23 ;
// 	}
// 
// 	m_daily_minute = (NDUINT8) minute_index ;
// 	if (m_daily_minute > 59){
// 		m_daily_minute = 59 ;
// 	}
// 
// 	time_t now ;
// 	struct tm *gtm ;
// 
// 	app_inst_time(&now) ;
// 	gtm = localtime( &now );
// 	
// 	if ((gtm->tm_hour > m_daily_hour) || (gtm->tm_hour == m_daily_hour&& gtm->tm_min > m_daily_minute)) {
// 		m_daily_last_runday = gtm->tm_mday ;
// 	}
// 	
// }
// 
// void NDAlarm::SetWeekTime(int day_index,int hour_index, int minute_index )
// {
// 	ND_TRACE_FUNC() ;
// 	m_week_day = day_index ;
// 	if (m_week_day>6){
// 		m_week_day = 6 ;
// 	}
// 	m_week_hour = hour_index ;
// 	if (m_week_hour>23)	{
// 		m_week_hour = 23 ;
// 	}
// 	m_week_minute = minute_index ;
// 	if(m_week_minute > 59){
// 		m_week_minute = 59 ;
// 	}
// 
// 	time_t now ;
// 	struct tm *gtm ;
// 	app_inst_time(&now) ;
// 	gtm = localtime( &now );
// 
// 	//每周定时器
// 	if (m_week_day == gtm->tm_wday ){
// 		//闹铃没有执行
// 		if ((gtm->tm_hour > m_week_hour) || (gtm->tm_hour == m_week_hour&& gtm->tm_min > m_week_minute)) {
// 			NDUINT32 wkindex = (NDUINT32)(now / (60*60 * 24 *7)) ;
// 			m_week_last_runweek = wkindex;
// 		}	
// 	}	
// }

