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


static time_t app_inst_delta = 0;

void app_inst_set_hm(int _h, int _m)
{
	app_inst_delta += _h * 3600 + _m * 60;
}

time_t app_inst_time(time_t* _t)
{
	//porting to linux 
	time_t cur = time(NULL);
	cur += app_inst_delta;
	if (_t){
		*_t = cur;
	}
	return cur;
}


//得到字符串形式的时间
const char *nd_get_timestr(void)
{
	static __ndthread  char timebuf[64];
	time_t nowtm;
	struct tm *gtm, tm1;

	time(&nowtm);
	gtm = localtime_r(&nowtm, &tm1);

	snprintf(timebuf, 64, "%d:%d:%d", gtm->tm_hour,
		gtm->tm_min, gtm->tm_sec);
	return (const char *)timebuf;
}
//得到字符串形式的日期
const char *nd_get_datestr(void)
{
	static __ndthread  char datebuf[64];
	time_t nowtm;
	struct tm *gtm, tm1;

	time(&nowtm);
	gtm = localtime_r(&nowtm, &tm1);

	snprintf(datebuf, 64, "%d-%d-%d", gtm->tm_year + 1900, gtm->tm_mon + 1,
		gtm->tm_mday);
	return (const char *)datebuf;
}
//得到字符串形式的时间和日期
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

	snprintf(timebuf, size, "%d-%d-%d %d:%d:%d",
		gtm->tm_year + 1900, gtm->tm_mon + 1, gtm->tm_mday,
		gtm->tm_hour, gtm->tm_min, gtm->tm_sec);
	return (const char *)timebuf;
}

int nd_time_day_interval(time_t end_tm, time_t start_tm)
{
	NDINT64 timezone = nd_time_zone();
	return nd_time_day_interval_ex(end_tm, start_tm, (int)timezone);
}

int nd_time_day_interval_ex(time_t end_tm, time_t start_tm, int timezone)
{

	NDINT64 start64 = (NDINT64)start_tm;
	NDINT64 end64 = (NDINT64)end_tm;

	start64 += timezone * 3600;
	end64 += timezone * 3600;

	NDINT64 start = (int)(start64 / (3600 * 24));
	NDINT64 end = (int)(end64 / (3600 * 24));
	return (int)(end - start);
}

int nd_time_zone()
{
	time_t stamp = 12 * 3600;
	struct  tm loca_tm = { 0 };
	struct  tm gm_tm = { 0 };
	localtime_r(&stamp, &loca_tm);
	gmtime_r(&stamp, &gm_tm);
	return loca_tm.tm_hour - gm_tm.tm_hour;
}

//convert the clock-time (9:30:00) to the index-of-second from 00:00:00
int nd_time_clock_to_seconds(const char *timetext)
{
	int hour = 0, minute = 0, second = 0;

	char *p = (char*)timetext;
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

//get time_t from text-clock "9:30:10" GT
time_t nd_time_from_clock(const char *timetext, time_t cur_time, int timezone)
{
	int hour = 0, minute = 0, second = 0;

	char *p = (char*)timetext;
	if (!p || !*p) {
		return 0;
	}
	hour = (int)strtol(p, &p, 0);
	if (hour <0 || hour>23) {
		return 0;
	}

	if (p && *p) {
		while (*p && !IS_NUMERALS(*p)) {
			++p;
		}
		if (*p) {
			minute = (int)strtol(p, &p, 0);
			if (minute <0 || minute >= 60) {
				return 0;
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
				return 0;
			}
		}
	}


	time_t now = cur_time; //get the current day
	struct  tm loca_tm;
	if (timezone == 0xff) {
		localtime_r(&now, &loca_tm);
	}
	else {
		now += timezone * 3600;
		gmtime_r(&now, &loca_tm);
	}

	loca_tm.tm_sec = second;
	loca_tm.tm_hour = hour;
	loca_tm.tm_min = minute;
	return mktime(&loca_tm);

}

//get second index from 00:00:00 (local time)
int nd_time_second_index_day(time_t timest)
{
	NDINT64 timezone = nd_time_zone();
	NDINT64 start64 = (NDINT64)timest;

	start64 += timezone * 3600;

	return (int)(start64 % (3600 * 24));
}
int nd_time_day_index_second(time_t timest)
{
	NDINT64 timezone = nd_time_zone();
	NDINT64 start64 = (NDINT64)timest;

	start64 += timezone * 3600;

	return (int)(start64 / (3600 * 24));
}
time_t  nd_time_from_str(const char *pInput, time_t* tim)
{
	time_t ret = 0;
	struct tm mytm = { 0 };
	char *p = (char*)pInput;
	mytm.tm_year = strtol(p, &p, 0);
	mytm.tm_year -= 1900;
	if (mytm.tm_year <0)	{
		return -1;
	}

#define GET_TIME_TYPE(_tm_type) \
	if (p && *p){				\
				while (*p && !IS_NUMERALS(*p))	{++p;}	\
		if (*p)	{								\
			int a = strtol(p, &p, 0);			\
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

	ret = mktime(&mytm); //localtime time ;
	if (tim){
		*tim = ret;
	}
	return ret;
}
//get weekday for nowtime (1-7)
int nd_time_weekday()
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
