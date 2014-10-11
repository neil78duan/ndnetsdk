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
	virtual int Update(ndtime_t tminterval) ;	//update per frame
	virtual int UpdateSecond() ;				//update per second
	virtual int UpdateMinute() ;				//update minute
	virtual int UpdateHour() ;				//update hour

	virtual void UpdateDaily() ;	//daily function
	virtual void UpdateWeek() ;		// weekly function
	virtual void Destroy() ;
	virtual int Create() ;

	void SetDailyTime(int hour_index,int minute_index=0) ;
    void SetWeekTime(int day_index,int hour_index, int minute_index = 0 ) ;
    nd_handle addTimer(NDObject *target, NDTimerFunc func, ndtime_t delay) ;
    int removeTimer(nd_handle hTimer) ;
    
	NDAlarm(NDObject *hostObj=NULL) ;
	virtual ~NDAlarm() ;
private:
	void update_alarm() ;
    void update_timer(ndtime_t tminterval) ;
	//int m_alarm_serial ;
	
	NDUINT8 m_daily_hour;		//�����ڼ���ִ��
	NDUINT8 m_daily_minute;		//�����ڼ��㼸��ִ��
	NDUINT8 m_daily_last_runday; //�ϴ�ִ�����������
	NDUINT8 m_week_hour;
	NDUINT8 m_week_day;
	NDUINT8 m_week_minute;
	NDUINT32 m_week_last_runweek;
protected:
	ndtime_t m_tminterval ;
	ndtime_t m_tick_minute ,m_tick_sec;
	NDUINT32 m_tick_index, m_second_index , m_minute_index ;
    
    NDObject *m_hostObject ;
    struct list_head sub_list ;

};
#endif
