#include <stdio.h>
#include <stdarg.h>
#include <time.h>

#include "loggy.h"

int _loggy_level = 1;

void loggy_datetime(char* output) {
    time_t timeraw;
    struct tm * timeinfo;
    time(&timeraw); // current
    timeinfo = localtime(&timeraw); // local
    strftime(output,50,"%d.%m.%Y %X", timeinfo);
}

void loggy_prepare(char* output, const char* level, const char* name, const char* format) {
  char datetime[50];
  loggy_datetime(datetime);
  sprintf(output, "%s %s - %s - %s\n", datetime, level, name, format);
}

void loggy_level(int level) {
  _loggy_level = level;
}

void loggy_trace(const char* name, const char* format, ... ) {
  if(_loggy_level < 3) return;
  char fullformat[1024];
  loggy_prepare(fullformat, "TRACE", name, format);
  va_list argp;
  va_start(argp, format);
  vprintf(fullformat, argp);
  va_end(argp);
}

void loggy_debug(const char* name, const char* format, ... ) {
  if(_loggy_level < 2) return;
  char fullformat[1024];
  loggy_prepare(fullformat, "DEBUG", name, format);
  va_list argp;
  va_start(argp, format);
  vprintf(fullformat, argp);
  va_end(argp);
}

void loggy_info(const char* name, const char* format, ... ) {
  if(_loggy_level < 1) return;
  char fullformat[1024];
  loggy_prepare(fullformat, "INFO ", name, format);
  va_list argp;
  va_start(argp, format);
  vprintf(fullformat, argp);
  va_end(argp);
}

void loggy_error(const char* name, const char* format, ... ) {
  if(_loggy_level < 0) return;
  char fullformat[1024];
  loggy_prepare(fullformat, "ERROR", name, format);
  va_list argp;
  va_start(argp, format);
  vprintf(fullformat, argp);
  va_end(argp);
}
