#pragma once

#include <time.h>
#include <pthread.h>
#include <errno.h>
#include <stdbool.h>

// This enables the verious levels of the logging function (FATAL & ERROR are always on)
//  0    =>   FATAL + ERROR
//  1    =>   FATAL + ERROR + WARN
//  2    =>   FATAL + ERROR + WARN + INFO
//  3    =>   FATAL + ERROR + WARN + INFO + DEBUG
//  4    =>   FATAL + ERROR + WARN + INFO + DEBUG + TRACE
#define LOG_LEVEL_ENABLED 4

#define MAX_MEASSGE_SIZE        2048
#define MAX_BUFFERED_MESSAGES   10
#define FUNCTION_NAME_STRING    __func__
#define FILE_NAME_STRING        __FILE__
#define FUNC_LINE               __LINE__
#define THREAD_ID               pthread_self()
#define ERROR_STR               strerror(errno)

// Debug breakpoint macro
#if defined(__GNUC__) || defined(__clang__)
    #define MY_DEBUG_BREAK      __asm__("int $3")
#elif defined(_MSC_VER)
    #define MY_DEBUG_BREAK      __debugbreak()
#else
    #define MY_DEBUG_BREAK      /* Unsupported platform, do nothing */
#endif

enum log_level {
    Fatal = 0,
    Error = 1,
    Warn = 2,
    Info = 3,
    Debug = 4,
    Trace = 5,
    LL_MAX_NUM = 6
};

struct log_time_exact{
    struct tm tm_generalTime;
    struct timespec ts_exact;
};

int log_init(char* LogFileName, char* LogFormat);
void log_shutdown();
void log_output(enum log_level level, const char* prefix, const char* funcName, char* fileName, int Line, pthread_t thread_id, const char* message, ...);

/*  Formating the LogMessages can be customised with the following tags
    to format all following Log Messages use: set_Formating(char* format);
    e.g. set_Formating("$B[$T] $L [$F]  $C$E")  or set_Formating("$BTime:[$M $S] $L $E ==> $C")
    
    $T		Time				hh:mm:ss
    $H		Hour				hh
    $M		Minute				mm
    $S		Secunde				ss
    $J		MilliSecunde		mm

    $N		Date				yyyy:mm:dd:
    $Y		Date Year			yyyy
    $O		Date Month			mm
    $D		Date Day			dd

    $F		Func. Name			main, foo
    $A		File Name			C:\Porject\main.c C:\Porject\foo.c
    $I		shortend File Name	main.c foo.c
    $G		Line				1, 42
    $P      thread id          b5bff640

    $L		LogLevel			[TRACE], [DEBUG] … [FATAL]
    $X		Alienment			add space for "INFO" & "WARN"
    $B		Color Begin			from here the color starts
    $E		Color End			from here the color ends
    $C		Text				Formated Message with variables
    $Z		New Line			Adds a new Line to the log*/
void set_Formating(char* format);
void use_Formating_Backup();

// Define witch log levels should be written to log file directly and witch should be buffered
//  0    =>   write all logs directly to log file
//  1    =>   buffer: TRACE
//  2    =>   buffer: TRACE + DEBUG
//  3    =>   buffer: TRACE + DEBUG + INFO
//  4    =>   buffer: TRACE + DEBUG + INFO + WARN
void set_buffer_Level();

void set_log_level(enum log_level new_level);
void print_Seperator(bool big);
void Calc_Func_Duration_Start(struct log_time_exact* StartTime);
void Calc_Func_Duration(struct log_time_exact* StartTime);

// Converts a bool value to a string
static inline const char* bool_To_String(bool boolValue) { return boolValue ? " true" : "false"; }

// checks pointers and returns " NULL" or "valid"
static inline const char* ptr_To_String(void* pointer) { return (pointer == NULL) ? " NULL" : "valid"; }

// ------------------------------------------------------------ LOGGING ------------------------------------------------------------

    #define CL_LOG_Fatal(message, ...)              log_output(Fatal, "", FUNCTION_NAME_STRING, FILE_NAME_STRING, FUNC_LINE, THREAD_ID, message, ##__VA_ARGS__);
    #define CL_LOG_Error(message, ...)              log_output(Error, "", FUNCTION_NAME_STRING, FILE_NAME_STRING, FUNC_LINE, THREAD_ID, message, ##__VA_ARGS__);


// define conditional log macro for WARNINGS
#if LOG_LEVEL_ENABLED >= 1
    #define CL_LOG_Warn(message, ...)               log_output(Warn, "", FUNCTION_NAME_STRING, FILE_NAME_STRING, FUNC_LINE, THREAD_ID, message, ##__VA_ARGS__);
#else
    // Disabled by LogLevel
    #define CL_LOG_Warn(message, ...)               {;}
#endif

// define conditional log macro for INFO
#if LOG_LEVEL_ENABLED >= 2
    #define CL_LOG_Info(message, ...)               log_output(Info, "", FUNCTION_NAME_STRING, FILE_NAME_STRING, FUNC_LINE, THREAD_ID, message, ##__VA_ARGS__);
                 
#else
    // Disabled by LogLevel
    #define CL_LOG_Info(message, ...)               {;}
#endif

// define conditional log macro for DEBUG
#if LOG_LEVEL_ENABLED >= 3
    #define CL_LOG_Debug(message, ...)              log_output(Debug, "", FUNCTION_NAME_STRING, FILE_NAME_STRING, FUNC_LINE, THREAD_ID, message, ##__VA_ARGS__);

    // Logs the end of a function, it would be helpfull to has the '$F' in your format    
    #define CL_LOG_FUNC_END(message, ...)           log_output(Debug, "END ", FUNCTION_NAME_STRING, FILE_NAME_STRING, FUNC_LINE, THREAD_ID, message, ##__VA_ARGS__);

    // Logs the start of a function, it would be helpfull to has the '$F' in your format
    #define CL_LOG_FUNC_START(message, ...)         log_output(Debug, "START ", FUNCTION_NAME_STRING, FILE_NAME_STRING, FUNC_LINE, THREAD_ID, message, ##__VA_ARGS__);
#else
    // Disabled by LogLevel
    #define CL_LOG_Debug(message, ...)              {;}
    // Disabled by LogLevel
    #define CL_LOG_FUNC_END(message, ...)           {;}
    // Disabled by LogLevel
    #define CL_LOG_FUNC_START(message, ...)         {;}
#endif

// define conditional log macro for REACE
#if LOG_LEVEL_ENABLED >= 4
    #define CL_LOG_Trace(message, ...)              log_output(Trace, "", FUNCTION_NAME_STRING, FILE_NAME_STRING, FUNC_LINE, THREAD_ID, message, ##__VA_ARGS__);
    // Insert a seperatioon line in Logoutput (-------)
    #define CL_SEPERATOR()                          print_Seperator(false);
    // Insert a seperatioon line in Logoutput (=======)
    #define CL_SEPERATOR_BIG()                      print_Seperator(true);
#else
    // Disabled by LogLevel
    #define CL_LOG_Trace(message, ...) ;
    // Disabled by LogLevel
    #define CL_SEPERATOR()                          {;}
    #define CL_SEPERATOR_BIG()                      {;}
#endif


#define CL_LOG(Type, message, ...)                  CL_LOG_##Type(message, ##__VA_ARGS__)

// ------------------------------------------------------------ VALIDATION / ASSERTION ------------------------------------------------------------
#define CL_VALIDATE(expr, messageSuccess, messageFailure, abortCommand, ...)                \
        if (expr) {                                                                         \
            CL_LOG_Trace(messageSuccess, ##__VA_ARGS__)                                     \
        } else {                                                                            \
            CL_LOG_Error(messageFailure, ##__VA_ARGS__)                                     \
            abortCommand;                                                                   \
        }   

#define CL_ASSERT(expr, messageSuccess, messageFailure, ...)                                \
        if (expr) {                                                                         \
            CL_LOG_Trace(messageSuccess, ##__VA_ARGS__)                                     \
        } else {                                                                            \
            CL_LOG_Fatal(messageFailure, ##__VA_ARGS__)                                     \
            MY_DEBUG_BREAK;                                                               \
        }

// ------------------------------------------------------------ MEASURE EXECUTION TIME ------------------------------------------------------------

// Remenbers the exact time at witch this macro was called
// CAUTION! only call once in a given scope
#define CL_FUNC_DURATION_START()                struct log_time_exact Log_Duration_Calc_Struct_Start;       \
                                                Calc_Func_Duration_Start(&Log_Duration_Calc_Struct_Start);

// Calculates the time diffrence between calling [CL_FUNC_DURATION_START] and this Macro
#define CL_FUNC_DURATION()                      Calc_Func_Duration(&Log_Duration_Calc_Struct_Start);
