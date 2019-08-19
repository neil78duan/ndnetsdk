/* file nd_time.c
 *
 * time function for nd-common
 *
 * create by duan 
 * 
 * 2016.8
 */

#include "nd_common/nd_common.h"

#include "nd_common/nd_time.h"

#ifdef ND_UNIX 
#if defined(_BSD_SOURCE)  || defined(_SVID_SOURCE )

#else 
#define timegm nd_sys_timegm
#endif

#endif 

static time_t app_inst_delta = 0;

void nd_gmtime_set_offset(int _h, int _m)
{
	app_inst_delta += _h * 3600 + _m * 60;
}

time_t nd_gmtime(time_t* _t)
{
	//porting to linux 
	time_t cur = time(NULL);
	cur += app_inst_delta;
	if (_t){
		*_t = cur;
	}
	return cur;
}


time_t nd_sys_timegm(struct tm* _res_tm)
{
	int mytimezone = nd_time_zone() ;
	time_t ret = mktime(_res_tm);
	return ret + mytimezone * 3600;
}

//get string format time
const char *nd_get_timestr(void)
{
	static __ndthread  char timebuf[64];
	time_t nowtm;
	struct tm *gtm, tm1;

	time(&nowtm);
	gtm = localtime_r(&nowtm, &tm1);

	ndsnprintf(timebuf, 64, "%d:%d:%d", gtm->tm_hour,
		gtm->tm_min, gtm->tm_sec);
	return (const char *)timebuf;
}
//get string format date
const char *nd_get_datestr(void)
{
	static __ndthread  char datebuf[64];
	time_t nowtm;
	struct tm *gtm, tm1;

	time(&nowtm);
	gtm = localtime_r(&nowtm, &tm1);

	ndsnprintf(datebuf, 64, "%d-%d-%d", gtm->tm_year + 1900, gtm->tm_mon + 1,
		gtm->tm_mday);
	return (const char *)datebuf;
}
//get string format datetime
const char *nd_get_datetimestr(void)
{
	static __ndthread  char timebuf[64];
	time_t nowtm;
	time(&nowtm);
	return nd_get_datetimestr_ex(nowtm, timebuf, 64);
}

const char *nd_get_datetimestr_ex(time_t in_tm, char *timebuf, int size)
{
	struct tm *gtm, tm1;
	gtm = localtime_r(&in_tm, &tm1);

	ndsnprintf(timebuf, size, "%d-%d-%d %d:%d:%d",
		gtm->tm_year + 1900, gtm->tm_mon + 1, gtm->tm_mday,
		gtm->tm_hour, gtm->tm_min, gtm->tm_sec);
	return (const char *)timebuf;
}

const char *nd_get_datetimestr_gm(time_t in_tm, char *timebuf, int size)
{
	struct tm *gtm, tm1;
	gtm = gmtime_r(&in_tm, &tm1);

	ndsnprintf(timebuf, size, "%d-%d-%d %d:%d:%d",
		gtm->tm_year + 1900, gtm->tm_mon + 1, gtm->tm_mday,
		gtm->tm_hour, gtm->tm_min, gtm->tm_sec);
	return (const char *)timebuf;
}


int nd_time_day_interval(time_t end_tm, time_t start_tm)
{
	return nd_time_day_interval_ex(end_tm, start_tm, nd_time_zone());
}

int nd_time_day_interval_ex(time_t end_tm, time_t start_tm, int tm_zone)
{

	NDINT64 start64 = (NDINT64)start_tm;
	NDINT64 end64 = (NDINT64)end_tm;

	start64 += tm_zone * 3600;
	end64 += tm_zone * 3600;

	NDINT64 start = (int)(start64 / (3600 * 24));
	NDINT64 end = (int)(end64 / (3600 * 24));
	return (int)(end - start);
}

//get week index from time_t =0 (1970.1.1 GMT)
int nd_time_week_index(time_t now, int tm_zone)
{
	NDINT64 tm64 = now + (time_t)tm_zone * 3600; //move to local time
	tm64 -= 3600 * 24 * 3;	// time_t(0) is thursday
	if (tm64 < 0 )	{
		tm64 = 0;
	}
	tm64 /= 3600 * 24 * 7;
	return (int)tm64 ;
}

int nd_time_zone(void)
{
	static int s_zoneid = -1;

	if (s_zoneid == -1) {
		time_t stamp = 12 * 3600; //this time all world is in the same day
		struct  tm loca_tm = { 0 };
		struct  tm gm_tm = { 0 };
		localtime_r(&stamp, &loca_tm);
		gmtime_r(&stamp, &gm_tm);
		s_zoneid = loca_tm.tm_hour - gm_tm.tm_hour;
	}
	return s_zoneid;
}

//convert the clock-time (9:30:00) to the index-of-second from 00:00:00
int nd_time_clock_to_seconds(const char *timetext)
{
	int hour = 0, minute = 0, second = 0;
	char *p = (char*)timetext;

	if (!timetext)	{
		return -1;
	}

	hour = (int)strtol(p, &p, 0);
	if (hour <0 || hour>23) {
		return -1;
	}

	if (p && *p) {
		while (*p && !IS_NUMERALS(*p)) {
			++p;
		}
		if (*p) {
			minute = (int)strtol(p, &p, 0);
			if (minute <0 || minute >= 60) {
				return -1;
			}
		}


	}
	if (p && *p) {
		while (*p && !IS_NUMERALS(*p)) {
			++p;
		}
		if (*p) {
			second = (int)strtol(p, &p, 0);
			if (second <0 || second >= 60) {
				return -1;
			}
		}
	}
	return hour * 3600 + minute * 60 + second;
}

time_t nd_time_from_offset(int secondIndexOfDay, time_t now, int tm_zone)
{
	if (tm_zone == 0xff){
		tm_zone = nd_time_zone();
	}
	now += tm_zone * 3600;

	now /= (3600 * 24);
	now *= (3600 * 24);

	now += secondIndexOfDay - tm_zone * 3600;
	
	return now;
}

time_t nd_time_from_month(int day_in_month, int secondIndexOfDay, time_t nowtm, int timezone)
{
	struct tm *gtm, tm1;

	nowtm += timezone * 3600;

	gtm = gmtime_r(&nowtm, &tm1);

	gtm->tm_hour = 0;
	gtm->tm_min = 0;
	gtm->tm_sec = 0;
	gtm->tm_mday = day_in_month;

	time_t ret = timegm(gtm);
	return ret - timezone * 3600 + secondIndexOfDay;
}

time_t nd_time_from_month_clock(int day_in_month, const char*timetext, time_t nowtm, int timezone)
{
	int secondIndex = nd_time_clock_to_seconds(timetext);
	if (secondIndex == -1) {
		return 0;
	}
	return nd_time_from_month(day_in_month, secondIndex, nowtm, timezone);
}

//get time_t from text-clock "9:30:10" GT
time_t nd_time_from_clock(const char *timetext, time_t cur_time, int tm_zone)
{
	int secondIndex = nd_time_clock_to_seconds(timetext);
	if (secondIndex == -1){
		return 0;
	}
	return nd_time_from_offset(secondIndex, cur_time, tm_zone);

}

time_t nd_time_from_week(int week_day, int secondIndexOfDay, time_t cur_time, int tm_zone)
{
	struct  tm gm_tm = { 0 };
	time_t stamp = cur_time;
	
	//get day offset 
	stamp += tm_zone * 3600;
	stamp -= stamp % (3600 * 24);	
	gmtime_r(&stamp, &gm_tm);

	cur_time += (week_day - gm_tm.tm_wday) * 24 * 3600;

	return nd_time_from_offset(secondIndexOfDay, cur_time, tm_zone);
}

time_t nd_time_from_week_clock(int week_day, const char *timetext, time_t cur_time, int tm_zone)
{
	int secondIndex = nd_time_clock_to_seconds(timetext);
	if (secondIndex == -1){
		return 0;
	}
	return nd_time_from_week(week_day, secondIndex, cur_time, tm_zone);
}

//get second index from 00:00:00 (local time)
int nd_time_second_offset_fromday(time_t timest, int tm_zone)
{
	//NDINT64 tm_zone = nd_time_zone();
	NDINT64 start64 = (NDINT64)timest;

	start64 += tm_zone * 3600;

	return (int)(start64 % (3600 * 24));
}
int nd_time_day_index(time_t timest, int tm_zone)
{
	//NDINT64 tm_zone = nd_time_zone();
	NDINT64 start64 = (NDINT64)timest;

	start64 += tm_zone * 3600;

	return (int)(start64 / (3600 * 24));
}

time_t  nd_time_from_str(const char *pInput, time_t* pOutTm)
{
	return  nd_time_from_str_ex(pInput, nd_time_zone(), pOutTm);
}

time_t  nd_time_from_str_ex(const char *pInput, int tm_zone, time_t* pOutTm)
{
	time_t ret = 0;
	struct tm mytm = { 0 };
	char *p = (char*)pInput;
	mytm.tm_year = (int)strtol(p, &p, 0);
	mytm.tm_year -= 1900;
	if (mytm.tm_year <0)	{
		return -1;
	}

#define GET_TIME_TYPE(_tm_type) \
	if (p && *p){				\
				while (*p && !IS_NUMERALS(*p))	{++p;}	\
		if (*p)	{								\
			int a = (int)strtol(p, &p, 0);			\
			if(errno==ERANGE || a < 0) {return -1 ;}	\
			mytm._tm_type = a ;				\
						}	\
			}
	GET_TIME_TYPE(tm_mon);
	--mytm.tm_mon;
	GET_TIME_TYPE(tm_mday);
	GET_TIME_TYPE(tm_hour);
	GET_TIME_TYPE(tm_min);
	GET_TIME_TYPE(tm_sec);

	ret = timegm(&mytm); //localtime time ;
	ret -= tm_zone * 3600;
	if (pOutTm){
		*pOutTm = ret;
	}
	return ret;
}
//get weekday for nowtime (1-7)
int nd_time_weekday(void)
{
	time_t nowtm;
	struct tm *gtm, tm1;

	time(&nowtm);
	gtm = localtime_r(&nowtm, &tm1);
	if (gtm->tm_wday == 0){
		return 7;
	}
	return gtm->tm_wday;
}
