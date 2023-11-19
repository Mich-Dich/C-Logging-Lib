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
                                                                strcat(message_log, Format_Buffer);


typedef struct MessageBuffer{

    char messages[MAX_BUFFERED_MESSAGES][MAX_MEASSGE_SIZE];
    int count;
    
} MessageBuffer;


// FATAL,ERROR,WARN,INFO,DEBUG,TRACE
static const char* colour_strings[6] = {"0;41", "1;31", "1;33", "1;32", "1;34", "1;0"};
static const char* level_str[6] = {"FATAL", "ERROR", "WARN", "INFO", "DEBUG", "TRACE"};
static const char* seperator = "-------------------------------------------------------------------------------------------------------";
static const char* seperator_Big = "=======================================================================================================";
static FILE* logFile;

static pthread_mutex_t LogLock = PTHREAD_MUTEX_INITIALIZER;
static char* TargetFileName = "unknown.txt";
static char* TargetLogFormat = "$S[$N] §M$E";
static char* TargetLogFormat_BACKUP = "$S[$N] §M$E";
static int log_level_for_buffer = 3;
static MessageBuffer Log_Message_Buffer = { .count = 0 };

// local Functions
struct tm getLocalTime(void);
void output_Messsage(enum log_level level, const char* message, const char* message_log);
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

        CL_ERROR("Error opening log file");
        return -1;
    }
    
    // print title section to start of file
    else {

        fprintf(logFile, "[%04d/%02d/%02d - %02d:%02d:%02d] Log initalized\n    Output-file: [%s]\n    Starting-format: %s\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, TargetFileName, TargetLogFormat);

        if (LOG_LEVEL_ENABLED <= 4 || LOG_LEVEL_ENABLED >= 0) {

            static const char* loc_level_str[6] = {"FATAL", " + ERROR", " + WARN", " + INFO", " + DEBUG", " + TRACE"};
            
            size_t LevelText_len = 1;
            for (int x = 0; x < LOG_LEVEL_ENABLED + 2; x++) {
                LevelText_len += strlen(loc_level_str[x]);
            }

            char* LogLevelText = malloc(LevelText_len);
            if (LogLevelText == NULL) {
                
                printf("FAILED to allocate memmory to print enabled LogLevels");
                return -1;
            }
            
            LogLevelText[0] = '\0';
            for (int x = 0; x < LOG_LEVEL_ENABLED + 2; x++) {
                strcat(LogLevelText, loc_level_str[x]);
            }
            fprintf(logFile, "    LOG_LEVEL_ENABLED = %d    enabled log macros are: %s\n", LOG_LEVEL_ENABLED, LogLevelText);
            free(LogLevelText);
        }
        fprintf(logFile, "%s\n", seperator_Big);
        fclose(logFile);
    }    
    
    CL_TRACE("Initialize")
    return 0;
}

// write bufferd messages to file and clean up output stream
void log_shutdown(){

    CL_TRACE("Shutdown")
    WriteMessagesToFile();
}

// Outout a message to the standart output stream and a log file
// !! CAUTION !! - do NOT make logs messages longer than [MAX_MEASSGE_SIZE] characters
void log_output(enum log_level level, const char* prefix, const char* funcName, char* fileName, const char* message, ...) {

    // check if message empty
    if (message[0] == '\0' && prefix[0] == '\0')
        return;

    struct tm locTime = getLocalTime();

    // Create Buffer Srings
    char message_out[MAX_MEASSGE_SIZE];
        memset(message_out, 0, sizeof(message_out));
    char message_log[MAX_MEASSGE_SIZE];
        memset(message_log, 0, sizeof(message_log));
    char message_formated[MAX_MEASSGE_SIZE];
        memset(message_formated, 0, sizeof(message_formated));
    char Format_Command[2] = "0\0";
    char Format_Buffer[MAX_MEASSGE_SIZE];

    // write all arguments in to [message_formated]
    __builtin_va_list args_ptr;
    va_start(args_ptr, message);
        vsnprintf(message_formated, MAX_MEASSGE_SIZE, message, args_ptr);
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
                sprintf(Format_Buffer, "\033[%sm", colour_strings[level]);
                strncat(message_out, Format_Buffer, 7 + sizeof(colour_strings[level]));
                break;
            
            // Color End
            case 'E':
                sprintf(Format_Buffer, "\033[0m");
                strncat(message_out, Format_Buffer, 7);
                break;

            // input text (message)
            case 'C':
                LOGGER_FORMAT_FORMAT_MESSAGE("%s%s", prefix, message_formated)
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
                if (level == LL_INFO || level == LL_WARN)       { LOGGER_FORMAT_FORMAT_MESSAGE(" ") }                    
            break;
            
            // Function Name
            case 'F':
                LOGGER_FORMAT_FORMAT_MESSAGE("%s", funcName)
            break;
            
            // File Name
            case 'A':
                LOGGER_FORMAT_FORMAT_MESSAGE("%s", basename(fileName))
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
            strncat(message_log, &TargetLogFormat[x], 1);
        }
    }
    
    //Format_Messages(message_out, message_log, "\n");
    //LOGGER_FORMAT_FORMAT_MESSAGE("\n")
    output_Messsage(level, (const char*)message_out, message_log);
}

//
void output_Messsage(enum log_level level, const char* message, const char* message_log) {
    
    // Print Message to standart output
    printf("%s", message);

    pthread_mutex_lock(&LogLock);
    // Save message in Buffer
    strncpy(Log_Message_Buffer.messages[Log_Message_Buffer.count], message_log, sizeof(Log_Message_Buffer.messages[0]) + 1);
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
    for (int i = 0; i < Log_Message_Buffer.count; ++i) {
        fprintf(file, "%s", Log_Message_Buffer.messages[i]);
    }

    // Close the file
    fclose(file);
}

// Change Format of log messages and backup previous Format
void set_Formating(char* LogFormat) {

    TargetLogFormat_BACKUP = TargetLogFormat;
    TargetLogFormat = LogFormat;
}

// Settis the Bckup version of Format to be used as Main Format
void use_Formating_Backup() {

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
        CL_ERROR("Input invalid Level (0 <= newLevel <= 4), input: %d", newLevel)
        return;
    }
}

//
void print_Seperator(enum log_level level, int big) {

    set_Formating("$C$Z");
    const char* loc_Sperator = big ? seperator_Big : seperator;
    log_output(level, "", "", "", loc_Sperator);
    use_Formating_Backup();
}