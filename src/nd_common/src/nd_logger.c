/* nd_logger.c
 *
 * merge the logger function to this file
 *
 * create by duan
 *
 * 2018.8.30
 */


#include "nd_common/nd_comcfg.h"
#include "nd_common/nd_os.h"
#include "nd_common/nd_logger.h"
#include "nd_common/nddir.h"
#include "nd_common/nd_str.h"
#include "nd_common/ndstdstring.h"


#include <time.h>
#include <stdlib.h>
#include <stdio.h>

#include "nd_common/nd_time.h"


static char __log_filename[256];

static nd_log_entry __log_func = NULL;
static ndatomic_t __log_write_len = 0;
static NDUINT32 __log_file_length = -1;
static NDUINT8 __close_screen_log = 0;

static NDUINT8 __without_file_info = 0;
static NDUINT8 __without_file_time = 0;
static NDUINT8 __witout_date = 0;
static NDUINT8 __log_path_without_date = 0;
#ifdef ND_DEBUG
static NDUINT8 _logoff_switch_bits = 0; //close debug and warning
#else 
static NDUINT8 _logoff_switch_bits = 2; //close debug and warning
#endif 

int nd_set_logpath_without_date(int without_date)
{
	int ret = __log_path_without_date;
	__log_path_without_date = without_date;
	return ret;
}

int nd_log_no_file(int without_file)
{
	int ret = __without_file_info;
	__without_file_info = without_file;
	return ret;
}
int nd_log_no_time(int without_time)
{
	int ret = __without_file_time;
	__without_file_time = without_time;
	return ret;

}
int nd_log_no_date(int without_date)
{
	int ret = __witout_date;
	__witout_date = without_date;
	return ret;


}
nd_log_entry nd_setlog_func(nd_log_entry f)
{
	nd_log_entry ret = __log_func;
	__log_func = f;
	return ret;
}
void nd_log_close_screen(int flag)
{
	__close_screen_log = flag ? 1 : 0;
}


void nd_log_open(edg_ID logType)
{
	_logoff_switch_bits &= ~(1 << (int)logType);
}
void nd_log_close(edg_ID logType)
{
	_logoff_switch_bits |= 1 << (int)logType;
}

int nd_log_check(edg_ID logType)
{
	return !(_logoff_switch_bits & (1 << (int)logType));
}

NDUINT32 nd_setlog_maxsize(NDUINT32 perfile_size)
{
	NDUINT32 oldval = __log_file_length;
	__log_file_length = perfile_size;
	return  oldval;
}

int nd_output(const char *text)
{
	return ndfprintf(stdout, "%s", text);
}

#define ND_LOG_FILE get_log_file()


#if defined(ND_LOG_PATH_WITH_DATE)
static char  __log_ext[10];
#endif

void set_log_file(const char *pathfile)
{
	//nd_fi
#if defined(ND_LOG_PATH_WITH_DATE)

	if (__log_path_without_date == 0) {
		const char *fileExt = nd_file_ext_name(pathfile);
		if (fileExt) {
			char ch = 0;
			char *tmp = fileExt - 1;
			ndstrncpy(__log_ext, fileExt, sizeof(__log_ext));

			ch = *tmp;
			*tmp = 0;
			ndstrncpy(__log_filename, pathfile, sizeof(__log_filename));
			*tmp = ch;
		}
		else {
			ndstrncpy(__log_filename, pathfile, sizeof(__log_filename));
		}
	}
	else
#endif 
	{
		ndstrncpy(__log_filename, pathfile, sizeof(__log_filename));
	}
}

char *get_log_file()
{

#if defined(ND_LOG_PATH_WITH_DATE)
	if (__log_path_without_date) {
		time_t now = time(NULL);
		static time_t s_lastRePathTime = 0;
		static ndatomic_t s_atomic_using;
		static char s_cur_path[1024];


		int  days = nd_time_day_interval(now, s_lastRePathTime);

		if (days > 0) {
			if (nd_compare_swap(&s_atomic_using, 0, 1)) {
				struct tm tm1;
				//char myPath[1024];
				//localtime_r(&now, &tm1);

				if (__log_ext[0]) {
					ndsnprintf(s_cur_path, sizeof(s_cur_path), "%s.%s.%s", __log_filename, nd_get_datestr(), __log_ext);
				}
				else {
					ndsnprintf(s_cur_path, sizeof(s_cur_path), "%s.%s", __log_filename, nd_get_datestr());
				}
				s_lastRePathTime = now;
				nd_atomic_set(&s_atomic_using, 0);

			}
		}

		if (s_cur_path[0])
			return s_cur_path;
		else
			return "ndlog.log";
	}
	else
#endif 
	{
		if (__log_filename[0])
			return __log_filename;
		else
			return "ndlog.log";
	}


}


int nd_default_filelog(const char* text)
{
	int size = 0;
	const char *logfile_name = ND_LOG_FILE;
	FILE *log_fp = fopen(logfile_name, "a");
	if (!log_fp) {
		return 0;
	}
	size = ndfprintf(log_fp, "%s", text);
	fclose(log_fp);

	if (size <= 0) {
		return 0;
	}


	nd_atomic_add(&__log_write_len, size);
	if ((NDUINT32)__log_write_len >= __log_file_length && __log_file_length != 0) {
		char aimFile[1024];
		int i = 1;
		char *p = aimFile;

		size = ndsnprintf(p, sizeof(aimFile), "%s.", logfile_name);
		p += size;
		do {
			ndsnprintf(p, sizeof(aimFile) - size, "%d", i);
			++i;
		} while (nd_existfile(aimFile));
		nd_renfile(logfile_name, aimFile);

		nd_atomic_set(&__log_write_len, 0);
	}
	return size;
}

//ndchar_t *strtowcs(char *src, ndchar_t *desc,int len) ;

static const char *_dg_msg_str[] =
{ "COMMON" , ("DEBUG"),("WARNING"),
("ERROR"),("FATAL") };

static __INLINE__ const char *log_level_str(int level)
{
	if ((level) >= ND_MSG && (level) <= ND_FATAL_ERR) {
		return _dg_msg_str[level];
	}
	else {
		return "COMMON";
	}
}

/* log_msg mutex */
//static nd_mutex 		log_mutex  ;


const char * _getfilename(const char *filenamePath)
{
	const char * ret = filenamePath;
	const char *p = filenamePath;
	while (*p) {
		if (*p == '/' || *p == '\\') {
			ret = p + 1;
		}
		++p;
	}

	return ret;
}

int _logmsg_screen(const char *filePath, int line, const char *stm, ...)
{
	char buf[1024 * 4];
	char *p = buf;
	va_list arg;
	int done;
	const char *file;

	if (__close_screen_log) {
		return 0;
	}

	file = _getfilename(filePath);

#ifdef	ND_LOG_WITH_DATE
	p += ndsnprintf(p, 4096, "%s ", nd_get_datestr());
#endif

#ifdef	ND_LOG_WITH_TIME
	p += ndsnprintf(p, 4096 - (p - buf), "%s ", nd_get_timestr());
#endif

#ifdef	ND_LOG_WITH_SOURCE
	p += ndsnprintf(p, 4096 - (p - buf), "[%d:%s] ", line, file);
#endif

	va_start(arg, stm);
	done = ndvsnprintf(p, sizeof(buf) - (p - buf), stm, arg);
	va_end(arg);

	ndfprintf(stdout, "%s", buf);

	return done;
}

int nd_logtext(const char *buf)
{
	int ret = 0;


#if defined(ND_OUT_LOG_2CTRL)
	if (__close_screen_log == 0) {
		ret = ndfprintf(stderr, "%s", buf);
	}
#endif

	if (__log_func) {
		ret = __log_func(buf);
	}
	else {
#ifdef ND_OUT_LOG_2FILE
		ret = nd_default_filelog(buf);
#endif
	}

	return ret;
}

int _logmsg(const char *func, const char *filePath, int line, int level, const char *stm, ...)
{
	size_t size;
	char buf[1024 * 4];
	char *p = buf;
	va_list arg;
	int done;
	const char *file = _getfilename(filePath);

	if (!nd_log_check(level)) {
		return 0;
	}

	size = sizeof(buf);

	p += ndsnprintf(p, size, "[%s] ", log_level_str(level));
	size = sizeof(buf) - (p - buf);

#ifdef ND_LOG_WITH_ND_MARK

	ndstrncpy(p, "[nd-log]  ", sizeof(buf));
	p += 9;
	size = sizeof(buf) - (p - buf);

#endif

#ifdef ND_LOG_WITH_DATE
	if (__witout_date == 0) {
		p += ndsnprintf(p, size, "%s ", nd_get_datestr());
		size = sizeof(buf) - (p - buf);
	}
#endif

#ifdef	ND_LOG_WITH_TIME
	if (__without_file_time == 0) {
		p += ndsnprintf(p, size, " %s ", nd_get_timestr());
		size = sizeof(buf) - (p - buf);
	}
#endif

#ifdef	ND_LOG_WITH_SOURCE
	if (__without_file_info == 0) {
		p += ndsnprintf(p, size, " %s() (%d:%s)", func, line, file);
		size = sizeof(buf) - (p - buf);
	}
#endif

	va_start(arg, stm);
	done = ndvsnprintf(p, size, stm, arg);
	size -= done;
	va_end(arg);

	nd_logtext(buf);
	return (int)(sizeof(buf) - size);
}
