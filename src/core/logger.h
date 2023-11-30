#pragma once

#define LOG_WARN_ENABLED 1
#define LOG_INFO_ENABLED 1
#define LOG_DEBUG_ENABLED 1
#define LOG_TRACE_ENABLED 1

#if DRELEASE == 1
    #define LOG_DEBUG_ENABLED 0
    #define LOG_TRACE_ENABLED 0
#endif

enum LogLevel {
    LOG_LEVEL_FATAL = 0,
    LOG_LEVEL_ERROR = 1,
    LOG_LEVEL_WARN = 2,
    LOG_LEVEL_INFO = 3,
    LOG_LEVEL_DEBUG = 4,
    LOG_LEVEL_TRACE = 5
};

//Call twice: first with state = 0 to get required mem size and second passing alloced mem to state
b8 InitializeLogging(u64* memoryRequirement, void* state);

void ShutdownLogging(void* state);

 void LogOutput(LogLevel level, char* message, b8 insert_newline, ...);

#define DFATAL(message, ...) LogOutput(LOG_LEVEL_FATAL, true, message, ##__VA_ARGS__);
#define DFATALN(message, ...) LogOutput(LOG_LEVEL_FATAL, false, message, ##__VA_ARGS__);

#ifndef DERROR
    #define DERROR(message, ...) LogOutput(LOG_LEVEL_ERROR, true, message, ##__VA_ARGS__);
    #define DERRORN(message, ...) LogOutput(LOG_LEVEL_ERROR, false, message, ##__VA_ARGS__);
#endif

#if LOG_WARN_ENABLED == 1
    #define DWARN(message, ...) LogOutput(LOG_LEVEL_WARN, true, message, ##__VA_ARGS__);
    #define DWARNN(message, ...) LogOutput(LOG_LEVEL_WARN, false, message, ##__VA_ARGS__);
#else
    #define DWARN(message, ...)
    #define DWARNN(message, ...)
#endif

#if LOG_INFO_ENABLED == 1
    #define DINFO(message, ...) LogOutput(LOG_LEVEL_INFO, true, message, ##__VA_ARGS__);
    #define DINFON(message, ...) LogOutput(LOG_LEVEL_INFO, false, message, ##__VA_ARGS__);
#else
    #define DINFO(message, ...)
    #define DINFON(message, ...)
#endif

#if LOG_DEBUG_ENABLED == 1
    #define DDEBUG(message, ...) LogOutput(LOG_LEVEL_DEBUG, true, message, ##__VA_ARGS__);
    #define DDEBUGN(message, ...) LogOutput(LOG_LEVEL_DEBUG, false, message, ##__VA_ARGS__);
#else
    #define DDEBUG(message, ...)
    #define DDEBUGN(message, ...)
#endif

#if LOG_TRACE_ENABLED == 1
    #define DTRACE(message, ...) LogOutput(LOG_LEVEL_TRACE, true, message, ##__VA_ARGS__);
    #define DTRACEN(message, ...) LogOutput(LOG_LEVEL_TRACE, false, message, ##__VA_ARGS__);
#else
    #define DTRACE(message, ...)
    #define DTRACEN(message, ...)
#endif