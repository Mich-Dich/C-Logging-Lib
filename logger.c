#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <pthread.h>
#include <libgen.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <inttypes.h>
#include <dirent.h>
#include <stdbool.h>


#include "logger.h"

#define REGISTERED_THREAD_NAME_LEN_MAX 256
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define LOGGER_FORMAT_FORMAT_MESSAGE(format, ...)               sprintf(Format_Buffer, format, ##__VA_ARGS__);              \
                                                                strcat(message_out, Format_Buffer);                         \

typedef struct message_pluss_thread {
    char text[MAX_MEASSGE_SIZE];
    pthread_t thread;
} message_pluss_thread;

typedef struct MessageBuffer{
    message_pluss_thread messages[MAX_BUFFERED_MESSAGES];
    int count;
} MessageBuffer;

typedef struct ThreadNameMap {
    pthread_t thread_id;
    char name[REGISTERED_THREAD_NAME_LEN_MAX];
    struct ThreadNameMap* next;
    struct ThreadNameMap* prev;
} ThreadNameMap;

typedef struct SpecificLogLevelFormat{
    bool isInUse;
    char* Format;
} SpecificLogLevelFormat;

// FATAL,ERROR,WARN,INFO,DEBUG,TRACE
static const char* Console_Colour_Strings[LL_MAX_NUM] = {"\x1b[1;41m", "\x1b[1;31m", "\x1b[1;93m", "\x1b[1;32m", "\x1b[1;94m", "\x1b[0;37m"};
static const char* Console_Colour_Reset = "\x1b[0;39m";
static const char* level_str[LL_MAX_NUM] = {"FATAL", "ERROR", "WARN", "INFO", "DEBUG", "TRACE"};
static const char* seperator     = "-------------------------------------------------------------------------------------------------------\n";
static const char* seperator_Big = "=======================================================================================================\n";
static FILE* logFile;

static pthread_mutex_t LogLock = PTHREAD_MUTEX_INITIALIZER;
static pthread_t MainThread = 0;
static enum log_level internal_level = Trace;
static char* MainLogFileName = "unknown.txt";
static char* m_GeneralLogFormat = "[Default] [$B$F: $G$E] - $B$C$E$Z";
static char* m_GeneralLogFormat_BACKUP = "[Default] [$B$F: $G$E] - $B$C$E$Z";

static SpecificLogLevelFormat SpecificLogFormatArray[] = { 
    {false, "[$B$L$X$E] [$B$F: $G$E] - $B$C$E$Z"}, 
    {false, "[$B$L$X$E] [$B$F: $G$E] - $B$C$E$Z"}, 
    {false, "[$B$L$X$E] [$B$F: $G$E] - $B$C$E$Z"}, 
    {false, "[$B$L$X$E] [$B$F: $G$E] - $B$C$E$Z"}, 
    {false, "[$B$L$X$E] [$B$F: $G$E] - $B$C$E$Z"}, 
    {false, "[$B$L$X$E] [$B$F: $G$E] - $B$C$E$Z"},
};

static int log_level_for_buffer = 0;
static MessageBuffer Log_Message_Buffer = { .count = 0 };
static ThreadNameMap* firstEntry = NULL;
static ThreadNameMap* lastEntry = NULL;
static bool Loc_Use_seperate_Files_for_every_Thread = true;

// local Functions
struct tm getLocalTime(void);
void output_Messsage(enum log_level level, const char* message, pthread_t threadID);
void WriteMessagesToFile();
bool Create_Log_File(const char* FileName);
ThreadNameMap* add_Thread_Name_Mapping(pthread_t thread, const char* name);
ThreadNameMap* f_find_Entry(pthread_t threadID);
void remove_Entry(pthread_t threadID);
int remove_all_Files_In_Directory(const char *dirName);


// Create or reset a Log-File: [LogFileName] and setup output format & stream
// - [LogFileName] default folgfile name
// - [m_GeneralLogFormat] general format for LogLevels that are not specificly set
// - [threadID] pthread_t of main thread
// - [Use_seperate_Files_for_every_Thread] 0 = false
int log_init(char* LogFileName, char* GeneralLogFormat, pthread_t threadID, int Use_seperate_Files_for_every_Thread) {

    MainLogFileName = LogFileName;    
    m_GeneralLogFormat = GeneralLogFormat;
    MainThread = threadID;

    Loc_Use_seperate_Files_for_every_Thread = Use_seperate_Files_for_every_Thread ? true : false;

    // Replace with your directory path
    const char *directoryName = "./Logs";
    if (remove_all_Files_In_Directory(directoryName) != 0) {
        fprintf(stderr, "Error removing files in the directory.\n");
    }

    CL_LOG(Trace, "Initialize")

    register_thread_log_under_Name(threadID, MainLogFileName);
    return 0;
}

bool Create_Log_File(const char* FileName) {

    //printf(" - - - Creating Log File: %s\n", FileName);

    // Open File
    logFile = fopen(FileName, "w");
    if (logFile == NULL) {

        //printf(" !=!=!=!=!=!=! Error opening log file !=!=!=!=!=!=!");
        return false;
    }
    
    // print title section to start of file
    else {

        struct tm tm= getLocalTime();
        fprintf(logFile, "[%04d/%02d/%02d - %02d:%02d:%02d] Log initalized\n    Output-file: [%s]\n    Starting-format: %s\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, FileName, m_GeneralLogFormat);

        if (LOG_LEVEL_ENABLED <= 4 || LOG_LEVEL_ENABLED >= 0) {

            static const char* loc_level_str[6] = {"FATAL", " + ERROR", " + WARN", " + INFO", " + DEBUG", " + TRACE"};
            
            size_t LevelText_len = 1;
            for (int x = 0; x < LOG_LEVEL_ENABLED + 2; x++) {
                LevelText_len += strlen(loc_level_str[x]);
            }

            char* LogLevelText = malloc(LevelText_len);
            if (LogLevelText == NULL) {
                
                printf("  FAILED to allocate memmory to print enabled LogLevels");
                return false;
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
    return true;
}

// write bufferd messages to logFile and clean up output stream
void log_shutdown(){

     CL_LOG(Trace, "Shutdown")
    WriteMessagesToFile();
}

// Outout a message to the standart output stream and a log file
// !! CAUTION !! - do NOT make logs messages longer than MAX_MEASSGE_SIZE
void log_output(enum log_level level, const char* prefix, const char* funcName, char* fileName, int Line, pthread_t thread_id, const char* message, ...) {

    // check if message empty
    if (message[0] == '\0' && prefix[0] == '\0')
        return;

    struct tm locTime = getLocalTime();

    // Create Buffer Srings
    char message_out[MAX_MEASSGE_SIZE];
        memset(message_out, 0, sizeof(message_out));
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

/*#define LOGGER_FORMAT_FORMAT_MESSAGE(format, ...)     sprintf(Format_Buffer, format, ##__VA_ARGS__);
                                                        strcat(message_out, Format_Buffer);             */

    char* locTargetFormat = m_GeneralLogFormat;
    if (SpecificLogFormatArray[level].isInUse)
        locTargetFormat = SpecificLogFormatArray[level].Format;

    int FormatLen = strlen(locTargetFormat);
    for (int x = 0; x < FormatLen; x++) {

        if (locTargetFormat[x] == '$' && x + 1 < FormatLen) {

            Format_Command[0] = locTargetFormat[x + 1];
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
                if (level == Info || level == Warn)       { LOGGER_FORMAT_FORMAT_MESSAGE(" ") }                    
            break;
            
            // Function Name
            case 'F':
                LOGGER_FORMAT_FORMAT_MESSAGE("%s", funcName)
            break;
            
            // File Name
            case 'A':
                LOGGER_FORMAT_FORMAT_MESSAGE("%s", fileName)
            break;
            
            // File Name
            case 'P':
                LOGGER_FORMAT_FORMAT_MESSAGE("%x", (uint32_t)thread_id)
            break;
            
            // Shortend File Name
            case 'I':
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

            // Clock ss
            case 'J': {
                struct timespec spec;
                clock_gettime(CLOCK_REALTIME, &spec);
                long ms = (long)(spec.tv_nsec / 1.0e6);
                LOGGER_FORMAT_FORMAT_MESSAGE("%03ld", ms)
            }
            break;

            // ------------------------------------  Date  -------------------------------------------------------------------------------
            // Data yyyy/mm/dd
            case 'N':
                LOGGER_FORMAT_FORMAT_MESSAGE("%04d/%02d/%02d", locTime.tm_year + 1900, locTime.tm_mon+1, locTime.tm_mday)
            break;
            
            // Year
            case 'Y':
                LOGGER_FORMAT_FORMAT_MESSAGE("%04d", locTime.tm_year + 1900)
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

            strncat(message_out, &locTargetFormat[x], 1);
        }
    }
    
    output_Messsage(level, (const char*)message_out, thread_id);
}

//
void output_Messsage(enum log_level level, const char* message, pthread_t threadID) {
    
    // Print Message to standart output
    if (level <= internal_level) {

        printf("%s", message);
        fflush(stdout);
    }

    pthread_mutex_lock(&LogLock);
    // Save message in Buffer
    strncpy(Log_Message_Buffer.messages[Log_Message_Buffer.count].text, message, sizeof(Log_Message_Buffer.messages[0].text));
    Log_Message_Buffer.messages[Log_Message_Buffer.count].text[sizeof(Log_Message_Buffer.messages[Log_Message_Buffer.count].text) - 1] = '\0';
    Log_Message_Buffer.messages[Log_Message_Buffer.count].thread = threadID;
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

    // NEW 
    ThreadNameMap* loc_Entry = NULL;
    char filename[REGISTERED_THREAD_NAME_LEN_MAX];
    for (int x = 0; x < Log_Message_Buffer.count; x++) {

        if(Loc_Use_seperate_Files_for_every_Thread) {

            loc_Entry = f_find_Entry(Log_Message_Buffer.messages[x].thread);
            if(loc_Entry != NULL) {

                snprintf(filename, sizeof(filename), "%s", loc_Entry->name);
            }
            else {

                //printf("    loc_Entry: %s [tread: %lu]\n", ptr_To_String(loc_Entry), Log_Message_Buffer.messages[x].thread);
                snprintf(filename, sizeof(filename), "Logs/thread_log_%lu.log", (unsigned long)Log_Message_Buffer.messages[x].thread);
                if(access(filename, F_OK) != 0) 
                    Create_Log_File(filename);
            }
        }

        else {

            snprintf(filename, sizeof(filename), "Logs/%s.log", MainLogFileName);
            if(access(filename, F_OK) != 0) 
                Create_Log_File(filename);
        }
        

        // Open the file for writing (append mode)
        FILE* file = fopen(filename, "a");
        if (file == NULL) {

            perror("Error opening the file");
            continue;
        }
        
        fputs((const char*)Log_Message_Buffer.messages[x].text, file); 
        fclose(file);
    }
    return;
}

// 
int register_thread_log_under_Name(pthread_t threadID, const char* name) {

    char filename[REGISTERED_THREAD_NAME_LEN_MAX];
    snprintf(filename, sizeof(filename), "Logs/thread_log_%lu.log", (unsigned long)threadID);

    char newFilename[REGISTERED_THREAD_NAME_LEN_MAX];
    snprintf(newFilename, sizeof(newFilename), "Logs/%s.log", name);

    // Create a new entry in list
    add_Thread_Name_Mapping(threadID, newFilename);

    if (access(filename, F_OK) != 0)
        return -1;

    int result = rename(filename, newFilename);
    if (result != 0) 
        return -1;

    return 0;
}

// ------------------------------------------------------------------------------------------ Thread-Name Mapping ------------------------------------------------------------------------------------------

//
ThreadNameMap* add_Thread_Name_Mapping(pthread_t thread, const char* name) {

    ThreadNameMap* loc_Found = f_find_Entry(thread);
    if(loc_Found != NULL) {

        //printf("  thread already has an Entry in [ThreadNameMap], [name] was updated");
        strncpy(loc_Found->name, name, MIN(strlen(name), REGISTERED_THREAD_NAME_LEN_MAX));
        return loc_Found;
    }

    ThreadNameMap* newEntry = (ThreadNameMap*) malloc(sizeof(ThreadNameMap));
    if (newEntry == NULL) {
        
        printf("  Memory allocation failed\n");
        return NULL;
    }

    memset(newEntry, 0, sizeof(ThreadNameMap));
    strncpy(newEntry->name, name, MIN(strlen(name), REGISTERED_THREAD_NAME_LEN_MAX));
    newEntry->thread_id = thread;
    
    if (firstEntry == NULL) {   // list is Empty

        firstEntry = newEntry;
        lastEntry = newEntry;
    
    } else {                      // Add to List-End

        lastEntry->next = newEntry;
        newEntry->prev = lastEntry;
        lastEntry = newEntry;
    }
    
    return newEntry;
}

// removes entry from linked list, if found
void remove_Entry(pthread_t threadID) {

    pthread_mutex_lock(&LogLock);
    ThreadNameMap *locPointer = f_find_Entry(threadID);
    if (locPointer == NULL)
        return;
    
    if (firstEntry == lastEntry) {          // List has only one element

        firstEntry = NULL;
        lastEntry = NULL;
    }

    // List has more than one element
    else {

        if (locPointer == firstEntry){      // Is First?

            firstEntry = firstEntry->next;
            firstEntry->prev = NULL;
        
        } else if (locPointer == lastEntry) { // Is Last?

            lastEntry = lastEntry->prev;
            lastEntry->next = NULL;
        
        } else {                              // is in middle

            locPointer->prev->next = locPointer->next;
            locPointer->next->prev = locPointer->prev;
        }
    }

    free(locPointer);
    pthread_mutex_unlock(&LogLock);
    return;
}

// Iterate through list and try to find entry by thread // Returns NULL if not found
ThreadNameMap* f_find_Entry(pthread_t threadID) {
    
    //printf("    f_find_Entry()  [thread: %lu]\n", threadID);
    bool locFound = false;
    ThreadNameMap *locPointer = firstEntry;
    while (locFound == false && locPointer != NULL) {

        //printf("    Iterating over Entry List current Entry.thread: %lu] [next: %s]\n", locPointer->thread_id, ptr_To_String(locPointer->next));

        if (locPointer->thread_id == threadID)
            locFound = true;
        else
            locPointer = locPointer->next;
    }

    return locFound ? locPointer : NULL;
}

// ------------------------------------------------------------------------------------------ Formating ------------------------------------------------------------------------------------------

// Change Format of log messages and backup previous Format
void set_Formating(char* LogFormat) {

    m_GeneralLogFormat_BACKUP = m_GeneralLogFormat;
    m_GeneralLogFormat = LogFormat;
}

// Settis the Bckup version of Format to be used as Main Format
void use_Formating_Backup() {

    m_GeneralLogFormat = m_GeneralLogFormat_BACKUP;
}

//
void Set_Format_For_Specific_Log_Level(enum log_level level, char* Format) {

    SpecificLogFormatArray[level].isInUse = true;
    SpecificLogFormatArray[level].Format = Format;

}

//
void Disable_Format_For_Specific_Log_Level(enum log_level level) {

    SpecificLogFormatArray[level].isInUse = false;
}

// ------------------------------------------------------------------------------------------ Misc ------------------------------------------------------------------------------------------

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

// Print a seperator "---"
void print_Seperator(pthread_t threadID)        { output_Messsage(Trace, seperator, threadID); }

// Print a seperator "==="
void print_Seperator_Big(pthread_t threadID)    { output_Messsage(Trace, seperator_Big, threadID); }

// Set what leg level should be printed to terminal
// CAUTION! this only applies to loglevels that are enabled be LOG_LEVEL_ENABLED
void set_log_level(enum log_level new_level) {

    CL_VALIDATE(new_level < LL_MAX_NUM && new_level > Fatal, "", "Selected log level is out of bounds (1 <= [new_level: %d] <= 5)", return, new_level)

    CL_LOG(Trace, "Setting [log_level: %s]", level_str[new_level])
    internal_level = new_level;

}

//
int remove_all_Files_In_Directory(const char *dirName) {
    DIR *dir = opendir(dirName);

    if (dir == NULL) {
        perror("Error opening directory");
        return -1;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {

        if (entry->d_type == DT_REG) {
        
            // It's a regular file, remove it
            char filepath[REGISTERED_THREAD_NAME_LEN_MAX *2];
            snprintf(filepath, sizeof(filepath), "%s/%s", dirName, entry->d_name);

            if (remove(filepath) != 0) {
                perror("Error removing file");
                closedir(dir);
                return -1;
            }
        }
    }

    closedir(dir);
    return 0;
}

// ------------------------------------------------------------------------------------------ Measure Time ------------------------------------------------------------------------------------------

// remenbers the exact time at witch this function was called
// NOT FINISHED
void Calc_Func_Duration_Start(struct log_time_exact* StartTime) {

    StartTime->tm_generalTime = getLocalTime();
    clock_gettime(CLOCK_REALTIME, &StartTime->ts_exact);     

    CL_LOG(Trace, "Starting Tine measurement")
}

// Calculates the time diffrence between calling [Calc_Func_Duration_Start] and this function
// NOT FINISHED
void Calc_Func_Duration(struct log_time_exact* StartTime) {

    struct log_time_exact TimeNow;
    TimeNow.tm_generalTime = getLocalTime();
    clock_gettime(CLOCK_REALTIME, &TimeNow.ts_exact);
    
    TimeNow.tm_generalTime.tm_year -= StartTime->tm_generalTime.tm_year;
    TimeNow.tm_generalTime.tm_mon -= StartTime->tm_generalTime.tm_mon;
    TimeNow.tm_generalTime.tm_yday -= StartTime->tm_generalTime.tm_yday;
    TimeNow.tm_generalTime.tm_hour -= StartTime->tm_generalTime.tm_hour;
    TimeNow.tm_generalTime.tm_min -= StartTime->tm_generalTime.tm_min;
    TimeNow.ts_exact.tv_sec -= StartTime->ts_exact.tv_sec;
    TimeNow.ts_exact.tv_nsec -= StartTime->ts_exact.tv_nsec;

    // Create Buffer Srings
    char message_out[MAX_MEASSGE_SIZE];
        memset(message_out, 0, sizeof(message_out));
    char Format_Buffer[MAX_MEASSGE_SIZE];
        memset(Format_Buffer, 0, sizeof(Format_Buffer));

    if(TimeNow.tm_generalTime.tm_year > 0) {
        LOGGER_FORMAT_FORMAT_MESSAGE("years: %04d", TimeNow.tm_generalTime.tm_year)
    }
    
    if(TimeNow.tm_generalTime.tm_mon > 0) {
        LOGGER_FORMAT_FORMAT_MESSAGE(" months: %02d", (12 % TimeNow.tm_generalTime.tm_mon))
    }
    
    if(TimeNow.tm_generalTime.tm_yday > 0) {
        LOGGER_FORMAT_FORMAT_MESSAGE(" days: %02d", (365 % TimeNow.tm_generalTime.tm_yday))
    }
    
    if(TimeNow.tm_generalTime.tm_hour > 0) {
        LOGGER_FORMAT_FORMAT_MESSAGE(" hours: %02d", (24 % TimeNow.tm_generalTime.tm_hour))
    }
    
    if(TimeNow.tm_generalTime.tm_min > 0) {
        LOGGER_FORMAT_FORMAT_MESSAGE(" min: %02d", (60 % TimeNow.tm_generalTime.tm_min))
    }
    
    if(TimeNow.ts_exact.tv_sec > 0) {
        LOGGER_FORMAT_FORMAT_MESSAGE(" s:%02ld", (60 % TimeNow.ts_exact.tv_sec))
    }
    if(TimeNow.ts_exact.tv_nsec > 0) {
        LOGGER_FORMAT_FORMAT_MESSAGE(" nano-sec:%02ld", (60 % TimeNow.ts_exact.tv_nsec))
    }

    CL_LOG(Trace, "Ending %s", message_out);
}


