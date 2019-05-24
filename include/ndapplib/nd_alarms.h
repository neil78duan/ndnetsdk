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

class ND_COMMON_CLASS  NDAlarm //: public NDObject
{
public:

	virtual int Update();
	virtual int UpdateFrame(ndtime_t tminterval) ;	//update per frame
	virtual int UpdateSecond() ;				//update per second
	virtual int UpdateMinute() ;				//update minute
	virtual int UpdateHour() ;				//update hour

	virtual void UpdateDaily(const char *eventName, void* p, int renewTimes) ;	//daily function
	virtual void UpdateWeek(const char *eventName, void* p) ;		// weekly function
	virtual void UpdateMonth(const char *eventName, void* p);		// monthly function
	
	virtual void Destroy(int flag=0) ;
	virtual int Create() ;

    nd_handle addAlarm(NDObject *target, NDObjectFunc func, ndtime_t delay) ; //add timer run one
    int removeAlarm(nd_handle hAlarm) ;
    
	NDAlarm() ;
	virtual ~NDAlarm() ;
private:
    int tick(ndtime_t tminterval) ;
	void update_timer(ndtime_t tminterval) ;
	
protected:
    ndtime_t m_lastTickTm ;
	ndtime_t m_tminterval ;
	ndtime_t m_tick_minute ,m_tick_sec;
	NDUINT32 m_tick_index, m_second_index , m_minute_index ;
    
    struct list_head sub_list ;

};
#endif
