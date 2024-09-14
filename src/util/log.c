#include "log.h"

log_cb_t *_logcb;

void logcb(log_cb_t *cb) {
	_logcb = cb;
}
void _log(enum log_level lvl, const char *msg, ...) {
	va_list args;
	va_start(args, msg);
	if (_logcb) _logcb(lvl, msg, args);
	va_end(args);
}

