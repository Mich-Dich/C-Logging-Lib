#pragma once

#include <time.h>
#include <pthread.h>
#include <errno.h>

// This enables the compilation of various logging levels (FATAL & ERROR are always on)
//  0    =>   FATAL + ERROR
//  1    =>   FATAL + ERROR + WARN
//  2    =>   FATAL + ERROR + WARN + INFO
//  3    =>   FATAL + ERROR + WARN + INFO + DEBUG
//  4    =>   FATAL + ERROR + WARN + INFO + DEBUG + TRACE
#define LOG_LEVEL_ENABLED 4

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

// ------------------------------------------------------------------------------ Main Functions ------------------------------------------------------------------------------

int log_init(char* LogFileName, char* GeneralLogFormat, pthread_t threadID, int Use_separate_Files_for_every_Thread) ;
void log_shutdown();
void log_output(enum log_level level, const char* prefix, const char* funcName, char* fileName, int Line, pthread_t thread_id, const char* message, ...);
void set_log_level(enum log_level new_level);
void print_Separator(pthread_t threadID);
void print_Separator_Big(pthread_t threadID);
void Calc_Func_Duration_Start(struct log_time_exact* StartTime);
void Calc_Func_Duration(struct log_time_exact* StartTime);
int register_thread_log_under_Name(pthread_t threadID, const char* name);

/*  Formatting the LogMessages can be customized with the following tags
    to format all following Log Messages use: set_Formatting(char* format);
    e.g. set_Formatting("$B[$T] $L [$F]  $C$E")  or set_Formatting("$BTime:[$M $S] $L $E ==> $C")
    
    $T		Time				hh:mm:ss
    $H		Hour				hh
    $M		Minute				mm
    $S		Second				ss
    $J		MilliSecond		mm

    $N		Date				yyyy:mm:dd:
    $Y		Date Year			yyyy
    $O		Date Month			mm
    $D		Date Day			dd

    $F		Func. Name			main, foo
    $A		File Name			C:\Project\main.c C:\Project\foo.c
    $I		shortened File Name	main.c foo.c
    $G		Line				1, 42
    $P      thread id          b5bff640

    $L		LogLevel			[TRACE], [DEBUG] … [FATAL]
    $X		Alignment			add space for "INFO" & "WARN"
    $B		Color Begin			from here the color starts
    $E		Color End			from here the color ends
    $C		Text				Formatted Message with variables
    $Z		New Line			Adds a new Line to the log*/
void set_Formatting(char* format);
void use_Formatting_Backup();
void Set_Format_For_Specific_Log_Level(enum log_level level, char* Format);
void Disable_Format_For_Specific_Log_Level(enum log_level level);

// Define witch log levels should be written to log file directly and witch should be buffered
//  0    =>   write all logs directly to log file
//  1    =>   buffer: TRACE
//  2    =>   buffer: TRACE + DEBUG
//  3    =>   buffer: TRACE + DEBUG + INFO
//  4    =>   buffer: TRACE + DEBUG + INFO + WARN
void set_buffer_Level(int newLevel);


// ------------------------------------------------------------------------------ Helper Functions ------------------------------------------------------------------------------

// checks pointers and returns " NULL" or "valid"
static inline const char* ptr_To_String(void* pointer) { return (pointer == NULL) ? " NULL" : "valid"; }

// ------------------------------------------------------------------------------ LOGGING ------------------------------------------------------------------------------

#define MAX_MESSAGE_SIZE        2048
#define MAX_BUFFERED_MESSAGES   20
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

// ------------------------------------------------------------------------------ LOGGING ------------------------------------------------------------------------------

#define CL_LOG_Fatal(message, ...)                  do{ log_output(Fatal, "", __func__, __FILE__, __LINE__, THREAD_ID, message, ##__VA_ARGS__); } while(0);
#define CL_LOG_Error(message, ...)                  do{ log_output(Error, "", __func__, __FILE__, __LINE__, THREAD_ID, message, ##__VA_ARGS__); } while(0);


// define conditional log macro for WARNINGS
#if LOG_LEVEL_ENABLED >= 1
    #define CL_LOG_Warn(message, ...)               do{ log_output(Warn, "", __func__, __FILE__, __LINE__, THREAD_ID, message, ##__VA_ARGS__); } while(0);
#else
    // Disabled by LogLevel
    #define CL_LOG_Warn(message, ...)               do{} while(0);
#endif

// define conditional log macro for INFO
#if LOG_LEVEL_ENABLED >= 2
    #define CL_LOG_Info(message, ...)               do{ log_output(Info, "", __func__, __FILE__, __LINE__, THREAD_ID, message, ##__VA_ARGS__); } while(0);
                 
#else
    // Disabled by LogLevel
    #define CL_LOG_Info(message, ...)               do{} while(0);
#endif

// define conditional log macro for DEBUG
#if LOG_LEVEL_ENABLED >= 3
    #define CL_LOG_Debug(message, ...)              do{ log_output(Debug, "", __func__, __FILE__, __LINE__, THREAD_ID, message, ##__VA_ARGS__); } while(0);

    // Logs the end of a function, it would be helpful to has the '$F' in your format
    #define CL_LOG_FUNC_END(message, ...)           do{ log_output(Debug, "END ", __func__, __FILE__, __LINE__, THREAD_ID, message, ##__VA_ARGS__); } while(0);

    // Logs the start of a function, it would be helpful to has the '$F' in your format
    #define CL_LOG_FUNC_START(message, ...)         do{ log_output(Debug, "START ", __func__, __FILE__, __LINE__, THREAD_ID, message, ##__VA_ARGS__); } while(0);
#else
    // Disabled by LogLevel
    #define CL_LOG_Debug(message, ...)              do{} while(0);
    // Disabled by LogLevel
    #define CL_LOG_FUNC_END(message, ...)           do{} while(0);
    // Disabled by LogLevel
    #define CL_LOG_FUNC_START(message, ...)         do{} while(0);
#endif

// define conditional log macro for TRACE
#if LOG_LEVEL_ENABLED >= 4
    #define CL_LOG_Trace(message, ...)              do{ log_output(Trace, "", __func__, __FILE__, __LINE__, THREAD_ID, message, ##__VA_ARGS__); } while(0);
    // Insert a separation line in Log output (-------)
    #define CL_SEPARATOR()                          do{ print_Separator(THREAD_ID); } while(0);
    // Insert a separation line in Log output (=======)
    #define CL_SEPARATOR_BIG()                      do{ print_Separator_Big(THREAD_ID); } while(0);
#else
    // Disabled by LogLevel
    #define CL_LOG_Trace(message, ...) ;
    // Disabled by LogLevel
    #define CL_SEPARATOR()                          do{} while(0);
    #define CL_SEPARATOR_BIG()                      do{} while(0);
#endif


#define CL_LOG(Type, message, ...)                  CL_LOG_##Type(message, ##__VA_ARGS__)

// ------------------------------------------------------------------------------ VALIDATION / ASSERTION ------------------------------------------------------------------------------
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

// ------------------------------------------------------------------------------ MEASURE EXECUTION TIME ------------------------------------------------------------------------------

// Remembers the exact time at witch this macro was called
// CAUTION! only call once in a given scope
#define CL_FUNC_DURATION_START()                struct log_time_exact Log_Duration_Calc_Struct_Start;       \
                                                Calc_Func_Duration_Start(&Log_Duration_Calc_Struct_Start);

// Calculates the time difference between calling [CL_FUNC_DURATION_START] and this Macro
#define CL_FUNC_DURATION()                      Calc_Func_Duration(&Log_Duration_Calc_Struct_Start);
