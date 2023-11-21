/*
 * Copyright (c) 2020 rxi
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <pthread.h>
#include <libgen.h>
#include <stdlib.h>

#include "logger.h"

typedef struct MessageBuffer {

    char messages[MAX_BUFFERED_MESSAGES][MAX_MESSAGE_SIZE];
    int count;

} MessageBuffer;


// FATAL,ERROR,WARN,INFO,DEBUG,TRACE
static const char *colour_strings[6] = {"0;41", "1;31", "1;33", "1;32", "1;34", "1;0"};
static const char *level_str[6] = {"[FATAL]", "[ERROR]", "[WARN] ", "[INFO] ", "[DEBUG]", "[TRACE]"};
static FILE *logFile;

static pthread_mutex_t LogLock = PTHREAD_MUTEX_INITIALIZER;
static char *TargetFileName = "unknown.txt";
static char *TargetLogFormat = "$S[$N] §M$E";
static char *TargetLogFormat_BACKUP = "$S[$N] §M$E";
static int log_level_for_buffer = 3;
static MessageBuffer Log_Message_Buffer = {.count = 0};

// local Functions
struct tm getLocalTime(void);

void output_Messsage(enum log_level level, char *message, const char *message_log);

void Format_Messages(char *out_mes, char *log_mes, const char *format, ...);

void WriteMessagesToFile();

// Create Log-File and setup output stream
int log_init(char *LogFileName, char *LogFormat) {

    TargetFileName = LogFileName;
    TargetLogFormat = LogFormat;
    struct tm tm = getLocalTime();

    // Open File
    logFile = fopen(TargetFileName, "w");
    if (logFile == NULL) {

        CL_ERROR("Error opening log file");
        return -1;
    }

        // print title section to start of file
    else {

        fprintf(logFile,
                "[%04d/%02d/%02d - %02d:%02d:%02d] Log initialized\n    Output-file: [%s]\n    Starting-format: %s\n",
                tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, TargetFileName,
                TargetLogFormat);

        if (LOG_LEVEL_ENABLED <= 4 || LOG_LEVEL_ENABLED >= 0) {

            static const char *loc_level_str[6] = {"FATAL", " + ERROR", " + WARN", " + INFO", " + DEBUG", " + TRACE"};
            char *LogLevelText[100];
            memset(LogLevelText, 0, sizeof(LogLevelText));

            for (int x = 0; x < LOG_LEVEL_ENABLED + 2; x++) {
                strcat(LogLevelText, loc_level_str[x]);
            }
            fprintf(logFile, "    LOG_LEVEL_ENABLED = %d    enabled log macros are: %s\n", LOG_LEVEL_ENABLED,
                    LogLevelText);

        }
        fprintf(logFile,
                "-------------------------------------------------------------------------------------------------------");
        fclose(logFile);
    }

    CL_TRACE("Initialize")
    return 1;
}

// write buffered messages to file and clean up output stream
void log_shutdown() {

    CL_TRACE("Shutdown")
    WriteMessagesToFile();
}

// Output a message to the standard output stream and a log file
// !! CAUTION !! - do NOT make logs messages over 32K characters
void log_output(enum log_level level, const char *message, const char *funcName, const char *fileName, ...) {

    // check if message empty
    if (message[0] == '\0')
        return;

    struct tm locTime = getLocalTime();

    // Create Buffer Strings
    char message_out[MAX_MESSAGE_SIZE];
    memset(message_out, 0, sizeof(message_out));
    char message_log[MAX_MESSAGE_SIZE];
    memset(message_log, 0, sizeof(message_log));
    char message_formatted[MAX_MESSAGE_SIZE];
    memset(message_formatted, 0, sizeof(message_formatted));
    char Format_Command[2] = "0\0";
    char Format_Buffer[MAX_MESSAGE_SIZE];

    // write all arguments in to [message_formatted]
    __builtin_va_list args_ptr;
    va_start(args_ptr, message);
    vsnprintf(message_formatted, MAX_MESSAGE_SIZE, message, args_ptr);
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
                    Format_Messages(message_out, message_log, "%s", message_formatted);
                    break;

                    // Log Level
                case 'L':
                    Format_Messages(message_out, message_log, "%s", level_str[level]);
                    break;

                    // Function Name
                case 'F':
                    Format_Messages(message_out, message_log, "%s", funcName);
                    break;

                    // File Name
                case 'A': {
                    const char *baseFileName = basename(fileName);
                    Format_Messages(message_out, message_log, "%s", baseFileName);
                    break;
                }
                    // ------------------------------------  Time  -------------------------------------------------------------------------------
                    // Clock hh:mm:ss
                case 'T':
                    Format_Messages(message_out, message_log, "%02d:%02d:%02d", locTime.tm_hour, locTime.tm_min,
                                    locTime.tm_sec);
                    break;

                    // Clock ss
                case 'H':
                    Format_Messages(message_out, message_log, "%02d", locTime.tm_hour);
                    break;

                    // Clock mm
                case 'M':
                    Format_Messages(message_out, message_log, "%02d", locTime.tm_min);
                    break;

                    // Clock ss
                case 'S':
                    Format_Messages(message_out, message_log, "%02d", locTime.tm_sec);
                    break;

                    // ------------------------------------  Date  -------------------------------------------------------------------------------
                    // Data yyyy/mm/dd
                case 'N':
                    Format_Messages(message_out, message_log, "%04d/%02d/%02d", locTime.tm_year + 1900,
                                    locTime.tm_mon + 1, locTime.tm_mday);
                    break;

                    // Year
                case 'Y':
                    Format_Messages(message_out, message_log, "%04d", locTime.tm_year + 1900);
                    break;

                    // Month
                case 'O':
                    Format_Messages(message_out, message_log, "%02d", locTime.tm_mon + 1);
                    break;

                    // Day
                case 'D':
                    Format_Messages(message_out, message_log, "%02d", locTime.tm_mday);
                    break;

                    // ------------------------------------  Default  -------------------------------------------------------------------------------
                default:
                    break;
            }

            x++;
        } else {

            strncat(message_out, &TargetLogFormat[x], 1);
            strncat(message_log, &TargetLogFormat[x], 1);
        }
    }

    Format_Messages(message_out, message_log, "\n");
    output_Messsage(level, message_out, message_log);
}

//
void output_Messsage(enum log_level level, char *message, const char *message_log) {

    // Print Message to standard output
    printf(message);

    // Save message in Buffer
    strncpy(Log_Message_Buffer.messages[Log_Message_Buffer.count], message_log,
            sizeof(Log_Message_Buffer.messages[0]) + 1);
    Log_Message_Buffer.messages[Log_Message_Buffer.count][sizeof(Log_Message_Buffer.messages[0]) -
                                                          1] = '\0'; // Null-terminate
    Log_Message_Buffer.count++;

    // Check if buffer full OR important message
    if (Log_Message_Buffer.count >= (MAX_BUFFERED_MESSAGES - 1) || level < (6 - log_level_for_buffer)) {

        WriteMessagesToFile();
        Log_Message_Buffer.count = 0;
    }
}

void WriteMessagesToFile() {

    pthread_mutex_lock(&LogLock);
    // Open the file for writing (append mode)
    FILE *file = fopen(TargetFileName, "a");
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
    pthread_mutex_unlock(&LogLock);
}

// Change Format of log messages and backup previous Format
void set_Formatting(char *LogFormat) {

    TargetLogFormat_BACKUP = TargetLogFormat;
    TargetLogFormat = LogFormat;
}

// Sets the Backup version of Format to be used as Main Format
void use_Formatting_Backup() {

    TargetLogFormat = TargetLogFormat_BACKUP;
}

//
void Format_Messages(char *out_mes, char *log_mes, const char *format, ...) {

    char Format_Buffer[MAX_MESSAGE_SIZE];

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
const char *append_prefix(const char *prefix, const char *message) {

    //size_t prefixLength = strlen(prefix);
    //size_t originalLength = strlen(message);
    const char *result = malloc(strlen(prefix) + strlen(message) + 1);
    if (result == NULL) {

        printf("[%s] malloc FAILED to prefix function name", FUNCTION_NAME_STRING);
        return message;
    } else {

        strcpy(result, prefix);
        strcat(result, message);
    }
    return result;
}

//
void set_buffer_Level(int newLevel) {

    if (newLevel <= 4 && newLevel >= 0)
        log_level_for_buffer = newLevel;

    else {
        CL_ERROR("Input invalid Level (0 <= newLevel <= 4), input: %d", newLevel)
        return;
    }
}