/* file nd_alarms.h
 *
 *
 * create by duan
 *
 * 2014-9-30
 *
 */

#ifndef _PG_ALARMS_H_
#define _PG_ALARMS_H_

#include "ndapplib/nd_object.h"

class NDAlarm : public NDObject
{
public:
    
    virtual int Update() ;
    
	virtual int Update(ndtime_t tminterval) ;	//update per frame
	virtual int UpdateSecond() ;				//update per second
	virtual int UpdateMinute() ;				//update minute
	virtual int UpdateHour() ;				//update hour

	virtual void UpdateDaily() ;	//daily function
	virtual void UpdateWeek() ;		// weekly function
	virtual void Destroy(int flag=0) ;
	virtual int Create() ;

	void SetDailyTime(int hour_index,int minute_index=0) ;
    void SetWeekTime(int day_index,int hour_index, int minute_index = 0 ) ;
    nd_handle addAlarm(NDObject *target, NDObjectFunc func, ndtime_t delay) ; //add timer run one
    int removeAlarm(nd_handle hAlarm) ;
    
	NDAlarm() ;
	virtual ~NDAlarm() ;
private:
    int tick(ndtime_t tminterval) ;
	void update_alarm() ;
    void update_timer(ndtime_t tminterval) ;
	//int m_alarm_serial ;
	
	NDUINT8 m_daily_hour;		//闹铃在几点执行
	NDUINT8 m_daily_minute;		//闹铃在几点几分执行
	NDUINT8 m_daily_last_runday; //上次执行闹铃的日期
	NDUINT8 m_week_hour;
	NDUINT8 m_week_day;
	NDUINT8 m_week_minute;
	NDUINT32 m_week_last_runweek;
protected:
    ndtime_t m_lastTickTm ;
	ndtime_t m_tminterval ;
	ndtime_t m_tick_minute ,m_tick_sec;
	NDUINT32 m_tick_index, m_second_index , m_minute_index ;
    
    //NDObject *m_hostObject ;
    struct list_head sub_list ;

};
#endif
