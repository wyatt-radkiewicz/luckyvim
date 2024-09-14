#ifndef _log_h_
#define _log_h_

#include <stdarg.h>
#include <string.h>

enum log_level {
	LOG_DBG,
	LOG_INFO,
	LOG_WARN,
	LOG_ERR,
};

typedef void (log_cb_t)(enum log_level l, const char *msg, va_list args);

void logcb(log_cb_t *cb);
#define logdbg(msg) \
	_log(LOG_DBG, "%s in %s: " msg, \
		__func__, \
		strrchr(__FILE__, '/') + 1 ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define loginfo(msg) \
	_log(LOG_INFO, "%s in %s: " msg, \
		__func__, \
		strrchr(__FILE__, '/') + 1 ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define logwarn(msg) \
	_log(LOG_WARN, "%s in %s: " msg, \
		__func__, \
		strrchr(__FILE__, '/') + 1 ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define logerr(msg) \
	_log(LOG_ERR, "%s in %s: " msg, \
		__func__, \
		strrchr(__FILE__, '/') + 1 ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define logdbgf(msg, ...) \
	_log(LOG_DBG, "%s in %s: " msg, \
		__func__, \
		strrchr(__FILE__, '/') + 1 ? strrchr(__FILE__, '/') + 1 : __FILE__, \
		__VA_ARGS__)
#define loginfof(msg, ...) \
	_log(LOG_INFO, "%s in %s: " msg, \
		__func__, \
		strrchr(__FILE__, '/') + 1 ? strrchr(__FILE__, '/') + 1 : __FILE__, \
		__VA_ARGS__)
#define logwarnf(msg, ...) \
	_log(LOG_WARN, "%s in %s: " msg, \
		__func__, \
		strrchr(__FILE__, '/') + 1 ? strrchr(__FILE__, '/') + 1 : __FILE__, \
		__VA_ARGS__)
#define logerrf(msg, ...) \
	_log(LOG_ERR, "%s in %s: " msg, \
		__func__, \
		strrchr(__FILE__, '/') + 1 ? strrchr(__FILE__, '/') + 1 : __FILE__, \
		__VA_ARGS__)
void _log(enum log_level lvl, const char *msg, ...);

#endif

