/* file nd_time.h
 *
 * define time function
 * 
 * create by duan 
 *
 * 2016.8
 */

#ifndef _ND_TIME_H_
#define _ND_TIME_H_

#include <time.h>

ND_COMMON_API ndtime_t		nd_time(void);
ND_COMMON_API ndbigtime_t	nd_bigtime(void);

ND_COMMON_API void nd_gmtime_set_offset(int hour_offset, int minute_offset);
ND_COMMON_API time_t nd_gmtime(time_t* pt);

ND_COMMON_API  time_t nd_sys_timegm(struct tm* _res_tm);

static __INLINE__  time_t app_inst_time(time_t* _t) {	return nd_gmtime(_t);}
static __INLINE__  void app_inst_set_hm(int _h, int _m) {	nd_gmtime_set_offset(_h, _m);}

ND_COMMON_API int nd_time_day_interval(time_t end_tm, time_t start_tm);	//get interval of days (local time)
ND_COMMON_API int nd_time_day_interval_ex(time_t end_tm, time_t start_tm, int tm_zone);

ND_COMMON_API int nd_time_zone(void);
ND_COMMON_API time_t  nd_time_from_str(const char *pInput, time_t* pOutTm); //convert localtime (2016-10-1 10:00:08) to time_t
ND_COMMON_API time_t  nd_time_from_str_ex(const char *pInput,int tm_zone, time_t* pOutTm); //convert localtime (2016-10-1 10:00:08) to time_t

//convert the clock-time (9:30:00) to the index-of-second from 00:00:00
ND_COMMON_API int nd_time_clock_to_seconds(const char *timetext);
//get time_t from text-clock "9:30:10" GT, cur_time means the current day
// @tm_zone = 0xff ,use the system time-zone, else time zone is set as param
ND_COMMON_API time_t nd_time_from_offset(int secondIndexOfDay, time_t now, int tm_zone);
ND_COMMON_API time_t nd_time_from_clock(const char *timetext, time_t cur_time, int tm_zone);
ND_COMMON_API time_t nd_time_from_week_clock(int week_day, const char *timetext, time_t cur_time, int tm_zone);
ND_COMMON_API time_t nd_time_from_week(int week_day, int secondIndexOfDay, time_t cur_time, int tm_zone);
ND_COMMON_API time_t nd_time_from_month(int day_in_month, int secondIndexOfDay, time_t nowtm, int timezone);
ND_COMMON_API time_t nd_time_from_month_clock(int day_in_month, const char*timetext, time_t nowtm, int timezone);

//get second index from 00:00:00 (local time)
ND_COMMON_API int nd_time_second_offset_fromday(time_t timest, int tm_zone);
ND_COMMON_API int nd_time_day_index(time_t timest, int tm_zone);
ND_COMMON_API int nd_time_week_index(time_t now, int tm_zone); //get local week index from time_t =0 (1970.1.1 GMT)


ND_COMMON_API const char *nd_get_timestr(void);			//get time in string/text
ND_COMMON_API const char *nd_get_datestr(void);			//get date in string
ND_COMMON_API const char *nd_get_datetimestr(void);		//get time and date in string

ND_COMMON_API const char *nd_get_datetimestr_ex(time_t in_tm, char *buf, int size);
ND_COMMON_API const char *nd_get_datetimestr_gm(time_t in_tm, char *timebuf, int size);
ND_COMMON_API int nd_time_weekday(void);

#endif
