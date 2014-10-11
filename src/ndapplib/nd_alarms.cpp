/* file pg_alarms.cpp
 *
 *
 * create by duan
 *
 * 2014-9-29
 *
 */

#include "nd_alarms.h"

#include "ndapplib/applib.h"

#define ALARM_HANDLE_ID 32
struct alarm_node
{
    ND_OBJ_BASE ;
    struct list_head list ;
    NDINT32 timeval ;
    NDObject *target ;
    NDTimerFunc timer_func ;
};


NDAlarm::NDAlarm(NDObject *host) : m_hostObject(host)
{
	ND_TRACE_FUNC() ;
	m_tminterval =0 ;
	m_tick_minute = 0;
	m_tick_sec = 0;
	m_second_index = 0;
	m_minute_index = 0;
	m_tick_index = 0;

	m_daily_hour = 0 ;
	m_week_hour = 0 ;
	m_week_day = 0 ;
	m_daily_last_runday = 0xff;
	m_week_last_runweek = 0 ;

	//m_alarm_serial = 0 ;

	m_daily_minute = 0 ;
	m_week_minute = 0;

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

	time(&now) ;
	gtm = localtime( &now );
	
	if ((gtm->tm_hour > m_daily_hour) || (gtm->tm_hour == m_daily_hour&& gtm->tm_min >= m_daily_minute)) {		
		m_daily_last_runday = gtm->tm_mday ;
	}
	if (m_week_day == gtm->tm_wday ){
		NDUINT32 wkindex = (NDUINT32)(now / (60*60 * 24 *7)) ;
		
        if ((gtm->tm_hour > m_week_hour) || (gtm->tm_hour == m_week_hour&& gtm->tm_min >= m_week_minute)) {
			m_week_last_runweek = wkindex;
		}		
	}	
	return 0;
}

void NDAlarm::Destroy()
{
	ND_TRACE_FUNC() ;

    
    struct list_head *pos, *next ;
    list_for_each_safe(pos,next, &sub_list) {
        alarm_node *node = list_entry(pos,alarm_node, list) ;
        removeTimer((nd_handle)node) ;
    }
    INIT_LIST_HEAD(&sub_list) ;
}

nd_handle NDAlarm::addTimer(NDObject *target, NDTimerFunc func, ndtime_t delay)
{
    nd_handle pool = GetMmpool()  ;
    alarm_node *node = (alarm_node *)nd_pool_alloc(pool, sizeof(alarm_node) );
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
int NDAlarm::removeTimer(nd_handle hTimer)
{
    nd_handle pool = GetMmpool() ;
    alarm_node *node = (alarm_node *) hTimer ;
    nd_assert(node);
    nd_assert(node->size == sizeof(*node)) ;
    nd_assert(node->type == ALARM_HANDLE_ID) ;
    
    list_del(&node->list) ;
    nd_pool_free(pool, node) ;
    return 0;
}

int NDAlarm::Update(ndtime_t tminterval)
{
	ND_TRACE_FUNC() ;
	m_tminterval = tminterval ;

	m_tick_minute += tminterval ;
	m_tick_sec += tminterval ;
	++m_tick_index ;
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

			++m_minute_index ;
			m_tick_minute -= 1000*60 ;
			update_alarm() ;

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
            removeTimer((nd_handle) node);
        }
        
    }
}

void NDAlarm::update_alarm()
{
	ND_TRACE_FUNC() ;
	time_t now ;
	struct tm *gtm ,tmnow;

	time(&now) ;
	gtm = localtime( &now );
	tmnow = *gtm;
	gtm = &tmnow;

	UpdateHour();

	if ( gtm->tm_mday!= m_daily_last_runday){
		if ((gtm->tm_hour > m_daily_hour) || (gtm->tm_hour == m_daily_hour&& gtm->tm_min >= m_daily_minute)) {
			UpdateDaily() ;
			m_daily_last_runday = gtm->tm_mday ;
		}
	}
	
	NDUINT32 wkindex = (NDUINT32)(now / (60*60 * 24 *7)) ;
	
    if (m_week_day == gtm->tm_wday ){
		if (m_week_last_runweek != wkindex){
			if ((gtm->tm_hour > m_week_hour) || (gtm->tm_hour == m_week_hour&& gtm->tm_min >= m_week_minute)) {
				UpdateWeek() ;
				m_week_last_runweek = wkindex;
			}			
		}
	}
}

int NDAlarm::UpdateSecond( )
{
    if (m_hostObject) {
        return m_hostObject->UpdateSecond() ;
    }
	return 0;
}

int NDAlarm::UpdateMinute()
{
    if (m_hostObject) {
        return m_hostObject->UpdateMinute() ;
    }
    
	return 0;
}

int NDAlarm::UpdateHour()
{
    if (m_hostObject) {
        return m_hostObject->UpdateHour() ;
    }
	return 0;
}

void NDAlarm::UpdateDaily()
{

}
void NDAlarm::UpdateWeek()
{

}

void NDAlarm::SetDailyTime(int hour_index,int minute_index)
{
	ND_TRACE_FUNC() ;
	m_daily_hour = (NDUINT8) hour_index ;
	if (m_daily_hour > 23){
		m_daily_hour = 23 ;
	}

	m_daily_minute = (NDUINT8) minute_index ;
	if (m_daily_minute > 59){
		m_daily_minute = 59 ;
	}

	time_t now ;
	struct tm *gtm ;

	time(&now) ;
	gtm = localtime( &now );
	
	if ((gtm->tm_hour > m_daily_hour) || (gtm->tm_hour == m_daily_hour&& gtm->tm_min > m_daily_minute)) {
		m_daily_last_runday = gtm->tm_mday ;
	}
	
}

void NDAlarm::SetWeekTime(int day_index,int hour_index, int minute_index )
{
	ND_TRACE_FUNC() ;
	m_week_day = day_index ;
	if (m_week_day>6){
		m_week_day = 6 ;
	}
	m_week_hour = hour_index ;
	if (m_week_hour>23)	{
		m_week_hour = 23 ;
	}
	m_week_minute = minute_index ;
	if(m_week_minute > 59){
		m_week_minute = 59 ;
	}

	time_t now ;
	struct tm *gtm ;
	time(&now) ;
	gtm = localtime( &now );

	//每周定时器
	if (m_week_day == gtm->tm_wday ){
		//闹铃没有执行
		if ((gtm->tm_hour > m_week_hour) || (gtm->tm_hour == m_week_hour&& gtm->tm_min > m_week_minute)) {
			NDUINT32 wkindex = (NDUINT32)(now / (60*60 * 24 *7)) ;
			m_week_last_runweek = wkindex;
		}	
	}	
}

