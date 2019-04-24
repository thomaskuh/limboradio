#ifndef LOGGY_H
#define LOGGY_H

void loggy_level(int level);
void loggy_trace(const char* name, const char* format, ... );
void loggy_debug(const char* name, const char* format, ... );
void loggy_info(const char* name, const char* format, ... );
void loggy_error(const char* name, const char* format, ... );

#endif
