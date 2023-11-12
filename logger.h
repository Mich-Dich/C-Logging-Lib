#pragma once
/*
 * Copyright (c) 2020 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See `logger.c` for details.
 */

// This enables the verious levels of the logging function (FATAL & ERROR are always on)
//  0    =>   FATAL + ERROR
//  1    =>   FATAL + ERROR + WARN
//  2    =>   FATAL + ERROR + WARN + INFO
//  3    =>   FATAL + ERROR + WARN + INFO + DEBUG
//  4    =>   FATAL + ERROR + WARN + INFO + DEBUG + TRACE
#define LOG_LEVEL_ENABLED 4

#define FUNCTION_NAME_STRING __func__
#define FILE_NAME_STRING __FILE__
#define MAX_MEASSGE_SIZE 2048
#define MAX_BUFFERED_MESSAGES 10

enum log_level {

    LL_FATAL = 0,
    LL_ERROR = 1,
    LL_WARN = 2,
    LL_INFO = 3,
    LL_DEBUG = 4,
    LL_TRACE = 5

};

//  Save FileName and LogFormat && Reset LogFile
int log_init(char* LogFileName, char* LogFormat);
void log_shutdown();

void log_output(enum log_level level, const char* message, const char* funcName, const char* fileName, ...);

/*  Formating the LogMessages can be customised with the following tags
    to format all following Log Messages use: set_Formating(char* format);
    e.g. set_Formating("$B[$T] $L [$F]  $C$E")  or set_Formating("$BTime:[$M $S] $L $E ==> $C")
    
$T		Time		hh:mm:ss
$H		Time Hour	hh
$M		Time Min.	mm
$S		Time Sec.	ss

$N		Date		yyyy:mm:dd:
$Y		Date Year	yyyy
$O		Date Month	mm
$D		Date Day	dd

$L		LogLevel	[TRACE], [DEBUG] … [FATAL]
$F		Func. Name	main, foo
$A		File Name	main.c foo.c
$B		Color Begin	from here the color starts
$E		Color End	from here the color ends
$C		Text		Formated Message with variables*/
void set_Formating(char* format);
void use_Formating_Backup();

// Define witch log levels should be written to log file directly and witch should be buffered
//  0    =>   write all logs directly to log file
//  1    =>   buffer: TRACE
//  2    =>   buffer: TRACE + DEBUG
//  3    =>   buffer: TRACE + DEBUG + INFO
//  4    =>   buffer: TRACE + DEBUG + INFO + WARN
void set_buffer_Level();
const char* append_prefix(const char* prefix, const char* message);



// ------------------------------------------------------------ LOGGING ------------------------------------------------------------
// log macro for FATAL
#ifndef CL_FATAL
    #define CL_FATAL(message, ...)                  log_output(LL_FATAL, message, FUNCTION_NAME_STRING, FILE_NAME_STRING, ##__VA_ARGS__);
#endif

// log macro for ERRORS
#ifndef CL_ERROR
    #define CL_ERROR(message, ...)                  log_output(LL_ERROR, message, FUNCTION_NAME_STRING, FILE_NAME_STRING, ##__VA_ARGS__);
#endif

// define conditional log macro for WARNINGS
#if LOG_LEVEL_ENABLED >= 1
    #define CL_WARNING(message, ...)                log_output(LL_WARN, message, FUNCTION_NAME_STRING, FILE_NAME_STRING, ##__VA_ARGS__);
#else
    // Disabled by LogLevel
    #define CL_WARN(message, ...)
#endif

// define conditional log macro for INFO
#if LOG_LEVEL_ENABLED >= 2
    #define CL_INFO(message, ...)                   log_output(LL_INFO, message, FUNCTION_NAME_STRING, FILE_NAME_STRING, ##__VA_ARGS__);

    // Logs the end of a function, it would be helpfull to has the '$F' in your format
    #define CL_LOG_FUNC_END(message, ...)           CL_INFO(append_prefix("%s ", message), "END ", ##__VA_ARGS__)
#else
    // Disabled by LogLevel
    #define CL_INFO(message, ...) ;
    // Disabled by LogLevel
    #define CL_LOG_FUNC_END(message, ...)
#endif

// define conditional log macro for DEBUG
#if LOG_LEVEL_ENABLED >= 3
    #define CL_DEBUG(message, ...)                  log_output(LL_DEBUG, message, FUNCTION_NAME_STRING, FILE_NAME_STRING, ##__VA_ARGS__);
#else
    // Disabled by LogLevel
    #define CL_DEBUG(message, ...) ;
#endif

// define conditional log macro for REACE
#if LOG_LEVEL_ENABLED >= 4
    #define CL_TRACE(message, ...)                  log_output(LL_TRACE, message, FUNCTION_NAME_STRING, FILE_NAME_STRING, ##__VA_ARGS__);

    // Logs the start of a function, it would be helpfull to has the '$F' in your format
    #define CL_LOG_FUNC_START(message, ...)         CL_TRACE(append_prefix("%s ", message), "START ", ##__VA_ARGS__)

    // Insert a seperatioon line in Logoutput (-------)
    #define CL_SEPERATOR()                                                                                                  \
        set_Formating("$C");                                                                                                \
        CL_TRACE("-------------------------------------------------------------------------------------------------------") \
        use_Formating_Backup();

    // Insert a seperatioon line in Logoutput (=======)
    #define CL_SEPERATOR_BIG()                                                                                              \
        set_Formating("$C");                                                                                                \
        CL_TRACE("=======================================================================================================") \
        use_Formating_Backup();
#else
    // Disabled by LogLevel
    #define CL_TRACE(message, ...) ;
    // Disabled by LogLevel
    #define CL_LOG_FUNC_START(message, ...)
    // Disabled by LogLevel
    #define CL_SEPERATOR()
    #define CL_SEPERATOR_BIG()
#endif

// ------------------------------------------------------------ VALIDATION / ASSERTION ------------------------------------------------------------
#define CL_VALIDATE(expr, messageSuccess, messageFailure)                               \
        if (expr) {                                                                     \
            CL_DEBUG(messageSuccess);                                                   \
        } else {                                                                        \
            CL_WARNING(messageFailure);                                                 \
        }   

#define CL_ASSERT(expr, messageSuccess, messageFailure, RetVal, ...)                    \
        if (expr) {                                                                     \
            CL_TRACE(append_prefix("   ", messageSuccess), ##__VA_ARGS__)               \
        } else {                                                                        \
            CL_ERROR(append_prefix("ABORT ", messageFailure), RetVal, ##__VA_ARGS__)    \
            return RetVal;                                                              \
        }
