#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <pthread.h>
#include <libgen.h>
#include <stdlib.h>

#include "logger.h"

#define LOGGER_FORMAT_FORMAT_MESSAGE(format, ...)               sprintf(Format_Buffer, format, ##__VA_ARGS__);              \
                                                                strcat(message_out, Format_Buffer);                         \


typedef struct MessageBuffer{

    char messages[MAX_BUFFERED_MESSAGES][MAX_MEASSGE_SIZE];
    int count;
    
} MessageBuffer;


// FATAL,ERROR,WARN,INFO,DEBUG,TRACE
static const char* Console_Colour_Strings[6] = {"\x1b[1;41m", "\x1b[1;31m", "\x1b[1;93m", "\x1b[1;32m", "\x1b[1;94m", "\x1b[0;37m"};
static const char* Console_Colour_Reset = "\x1b[0;39m";
static const char* level_str[6] = {"FATAL", "ERROR", "WARN", "INFO", "DEBUG", "TRACE"};
static const char* separator = "-------------------------------------------------------------------------------------------------------";
static const char* separator_Big = "=======================================================================================================";
static FILE* logFile;

static pthread_mutex_t LogLock = PTHREAD_MUTEX_INITIALIZER;
static char* TargetFileName = "unknown.txt";
static char* TargetLogFormat = "$S[$N] $M$E";
static char* TargetLogFormat_BACKUP = "$S[$N] $M$E";
static int log_level_for_buffer = 3;
static MessageBuffer Log_Message_Buffer = { .count = 0 };

// local Functions
struct tm getLocalTime(void);
void output_Messsage(enum log_level level, const char* message);
//void Format_Messages(char* out_mes, char* log_mes, const char* format, ...);
void WriteMessagesToFile();

// Create Log-File and setup output stream
int log_init(char* LogFileName, char* LogFormat) {

    TargetFileName = LogFileName;
    TargetLogFormat = LogFormat;
    struct tm tm= getLocalTime();

    // Open File
    logFile = fopen(TargetFileName, "w");
    if (logFile == NULL) {

        CL_LOG(Error, "Error opening log file");
        return -1;
    }
    
    // print title section to start of file
    else {

        fprintf(logFile, "[%04d/%02d/%02d - %02d:%02d:%02d] Log initialized\n    Output-file: [%s]\n    Starting-format: %s\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, TargetFileName, TargetLogFormat);

        if (LOG_LEVEL_ENABLED <= 4 || LOG_LEVEL_ENABLED >= 0) {

            static const char* loc_level_str[6] = {"FATAL", " + ERROR", " + WARN", " + INFO", " + DEBUG", " + TRACE"};
            
            size_t LevelText_len = 1;
            for (int x = 0; x < LOG_LEVEL_ENABLED + 2; x++) {
                LevelText_len += strlen(loc_level_str[x]);
            }

            char* LogLevelText = malloc(LevelText_len);
            if (LogLevelText == NULL) {
                
                printf("FAILED to allocate memory to print enabled LogLevels");
                return -1;
            }
            
            LogLevelText[0] = '\0';
            for (int x = 0; x < LOG_LEVEL_ENABLED + 2; x++) {
                strcat(LogLevelText, loc_level_str[x]);
            }
            fprintf(logFile, "    LOG_LEVEL_ENABLED = %d    enabled log macros are: %s\n", LOG_LEVEL_ENABLED, LogLevelText);
            free(LogLevelText);
        }
        fprintf(logFile, "%s\n", separator_Big);
        fclose(logFile);
    }    
    
     CL_LOG(Trace, "Initialize")
    return 0;
}

// write buffered messages to file and clean up output stream
void log_shutdown(){

     CL_LOG(Trace, "Shutdown")
    WriteMessagesToFile();
}

// Output a message to the standard output stream and a log file
// !! CAUTION !! - do NOT make logs messages longer than [MAX_MESSAGE_SIZE] characters
void log_output(enum log_level level, const char* prefix, const char* funcName, char* fileName, int Line, const char* message, ...) {

    // check if message empty
    if (message[0] == '\0' && prefix[0] == '\0')
        return;

    struct tm locTime = getLocalTime();

    // Create Buffer Strings
    char message_out[MAX_MEASSGE_SIZE];
        memset(message_out, 0, sizeof(message_out));/*
    char message_log[MAX_MESSAGE_SIZE];
        memset(message_log, 0, sizeof(message_log));*/
    char message_formatted[MAX_MEASSGE_SIZE];
        memset(message_formatted, 0, sizeof(message_formatted));
    char Format_Command[2] = "0\0";
    char Format_Buffer[MAX_MEASSGE_SIZE];

    // write all arguments in to [message_formatted]
    __builtin_va_list args_ptr;
    va_start(args_ptr, message);
        vsnprintf(message_formatted, MAX_MEASSGE_SIZE, message, args_ptr);
    va_end(args_ptr);


    // Loop over Format string and build Final Message
    int FormatLen = strlen(TargetLogFormat);
    for (int x = 0; x < FormatLen; x++) {

        if (TargetLogFormat[x] == '$' && x + 1 < FormatLen) {

            Format_Command[0] = TargetLogFormat[x + 1];
            switch (Format_Command[0]) {
            
            // ------------------------------------  Basic Info  -------------------------------------------------------------------------------
            // Color Start
            case 'B':
                LOGGER_FORMAT_FORMAT_MESSAGE("%s", Console_Colour_Strings[level]);
            break;
            
            // Color End
            case 'E': 
                LOGGER_FORMAT_FORMAT_MESSAGE("%s", Console_Colour_Reset);
            break;
            
            // input text (message)
            case 'C':
                LOGGER_FORMAT_FORMAT_MESSAGE("%s%s", prefix, message_formatted)
            break;

            // Log Level
            case 'L':
                LOGGER_FORMAT_FORMAT_MESSAGE("%s", level_str[level])
            break;

            case 'Z':
                LOGGER_FORMAT_FORMAT_MESSAGE("\n")
            break;

            // Log Level
            case 'X':
                if (level == Info || level == Warn)       { LOGGER_FORMAT_FORMAT_MESSAGE(" ") }                    
            break;
            
            // Function Name
            case 'F':
                LOGGER_FORMAT_FORMAT_MESSAGE("%s", funcName)
            break;
            
            // File Name
            case 'A':
                LOGGER_FORMAT_FORMAT_MESSAGE("%s", basename(fileName))
            break;
            
            // Line
            case 'G':
                LOGGER_FORMAT_FORMAT_MESSAGE("%d", Line)
            break;

            // ------------------------------------  Time  -------------------------------------------------------------------------------
            // Clock hh:mm:ss
            case 'T':
                LOGGER_FORMAT_FORMAT_MESSAGE("%02d:%02d:%02d", locTime.tm_hour, locTime.tm_min, locTime.tm_sec)
            break;

            // Clock ss
            case 'H':
                LOGGER_FORMAT_FORMAT_MESSAGE("%02d", locTime.tm_hour)
            break;
            
            // Clock mm
            case 'M':
                LOGGER_FORMAT_FORMAT_MESSAGE("%02d", locTime.tm_min)
            break;

            // Clock ss
            case 'S':
                LOGGER_FORMAT_FORMAT_MESSAGE("%02d", locTime.tm_sec)
            break;

            // ------------------------------------  Date  -------------------------------------------------------------------------------
            // Data yyyy/mm/dd
            case 'N':
                LOGGER_FORMAT_FORMAT_MESSAGE("%04d/%02d/%02d", locTime.tm_year + 1900, locTime.tm_mon+1, locTime.tm_mday)
            break;
            
            // Year
            case 'Y':
                LOGGER_FORMAT_FORMAT_MESSAGE("%d", locTime.tm_year + 1900)
            break;
            
            // Month
            case 'O':
                LOGGER_FORMAT_FORMAT_MESSAGE("%02d", locTime.tm_mon + 1)
            break;

            // Day
            case 'D':
                LOGGER_FORMAT_FORMAT_MESSAGE("%02d", locTime.tm_mday)
            break;


            // ------------------------------------  Default  -------------------------------------------------------------------------------
            default:
                break;
            }

            x++;
        }

        else {

            strncat(message_out, &TargetLogFormat[x], 1);
            //strncat(message_log, &TargetLogFormat[x], 1);
        }
    }
    
    //Format_Messages(message_out, message_log, "\n");
    //LOGGER_FORMAT_FORMAT_MESSAGE("\n")
    output_Messsage(level, (const char*)message_out);
}

//
void output_Messsage(enum log_level level, const char* message) {
    
    // Print Message to standard output
    printf("%s", message);

    pthread_mutex_lock(&LogLock);
    // Save message in Buffer
    strncpy(Log_Message_Buffer.messages[Log_Message_Buffer.count], message, sizeof(Log_Message_Buffer.messages[0]) + 1);
    Log_Message_Buffer.messages[Log_Message_Buffer.count][sizeof(Log_Message_Buffer.messages[0]) - 1] = '\0'; // Null-terminate
    Log_Message_Buffer.count++;

    // Check if buffer full OR important message
    if (Log_Message_Buffer.count >= (MAX_BUFFERED_MESSAGES -1) || level < (6 - (unsigned int)log_level_for_buffer)) {
        
        WriteMessagesToFile();
        Log_Message_Buffer.count = 0;
    }
    
    pthread_mutex_unlock(&LogLock); 
}

//
void WriteMessagesToFile() {

    // Open the file for writing (append mode)
    FILE* file = fopen(TargetFileName, "a");
    if (file == NULL) {
        perror("Error opening the file");
        return;
    }

    // Write each buffered message to the file
    for (int i = 0; i < Log_Message_Buffer.count; ++i) 
        fputs(Log_Message_Buffer.messages[i], file);
    

    // Close the file
    fclose(file);
}

// Change Format of log messages and backup previous Format
void set_Formatting(char* LogFormat) {

    TargetLogFormat_BACKUP = TargetLogFormat;
    TargetLogFormat = LogFormat;
}

// Sets the Backup version of Format to be used as Main Format
void use_Formatting_Backup() {

    TargetLogFormat = TargetLogFormat_BACKUP;
}

//
void Format_Messages(char* out_mes, char* log_mes, const char* format, ...) {

    char Format_Buffer[MAX_MEASSGE_SIZE];

    __builtin_va_list args;
    va_start(args, format);
    vsprintf(Format_Buffer, format, args);
    va_end(args);

    strcat(out_mes, Format_Buffer);
    strcat(log_mes, Format_Buffer);
}

// get system time
struct tm getLocalTime(void) {

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    return tm;
}

//
void set_buffer_Level(int newLevel) {

    if( newLevel <= 4 && newLevel >= 0)
        log_level_for_buffer = newLevel;

    else {
        CL_LOG(Error, "Input invalid Level (0 <= newLevel <= 4), input: %d", newLevel)
        return;
    }
}

//
void print_Separator(enum log_level level, int big) {

    set_Formatting("$C$Z");
    const char* log_Separator = big ? separator_Big : separator;
    log_output(level, "", "", "", 0, log_Separator);
    use_Formatting_Backup();
}