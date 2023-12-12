# C Logging Library

## Overview

Welcome to the C Logging Library repository! This library is designed to provide a simple and efficient logging solution for C applications. While the current version is tailored for Linux environments, we have exciting plans to expand its features and support multiple platforms in the future.

## Features

### Usage

```C
#include <logger.h>

// Call this at the start of your program to set LogFile name and message formatting
int log_init("log_file.log", "$B[$T] $L [$F] $C$E$Z", pthread_self(), 0);

// You can change the message formatting at runtime for all following messages
void set_formatting("$B[$T] $A-$F$E $C$Z");

// You can also revert back to your previous Format
void use_formatting_Backup();

// Define witch log levels should be written to log file directly and witch should be buffered
//  0    =>   write all logs directly to log file
//  1    =>   buffer: TRACE
//  2    =>   buffer: TRACE + DEBUG
//  3    =>   buffer: TRACE + DEBUG + INFO
//  4    =>   buffer: TRACE + DEBUG + INFO + WARN
void set_buffer_Level(4);

// To log some information use one of the following macros (use standard C formatting)
CL_LOG(Trace, "int: %d, string: %s", someInt, someStr)
CL_LOG(Debug, "int: %d, string: %s", someInt, someStr)
CL_LOG(Info, "int: %d, string: %s", someInt, someStr)
CL_LOG(Warn, "int: %d, string: %s", someInt, someStr)
CL_LOG(Error, "int: %d, string: %s", someInt, someStr)
CL_LOG(Fatal, "int: %d, string: %s", someInt, someStr)

// Call this to automatically log function Starts 
CL_LOG_FUNC_START("")                           // No Args
CL_LOG_FUNC_START("start param1: %d", someInt)  // With Args

// Call this to automatically log the successfully End of a function
CL_LOG_FUNC_END("")                             // No Args
CL_LOG_FUNC_END("start param1: %d", someInt)    // With Args

// rename the log-file of a pthread
int register_thread_log_under_Name(pthread_t threadID, const char* name);

// Change general-format of all following log-messages and backup old general-format
void set_Formatting(char* format);

// Use the Backup version of general-format to be used as Main Format
void use_Formatting_Backup();

// 
void Set_Format_For_Specific_Log_Level(enum log_level level, char* Format);

//
void Disable_Format_For_Specific_Log_Level(enum log_level level);

// Use this validation to make check some condition and log different messages
CL_VALIDATE(expr, messageSuccess, messageFailure)

// Use this assert to make sure your condition is true
// The CL_ASSERT macro uses a '__debugbreak()' if condition != true 
CL_ASSERT(expr, messageSuccess, messageFailure, RetVal, ...)

// Call this at the end of your program to push all buffered messages into the log file
void log_shutdown();

```

### formatting
formatting the LogMessages can be customized with the following tags<br>
to format all following Log Messages use: set_formatting(char* format);<br>
e.g. set_formatting("$B[$T] $L [$F]  $C$E")  or set_formatting("$BTime:[$M $S] $L $E ==> $C")

| Code | Description  | Format/Example                    |
|------|--------------|-----------------------------------|
| $T   | Time         | hh:mm:ss                          |
| $H   | Time Hour    | hh                                |
| $M   | Time Min.    | mm                                |
| $S   | Time Sec.    | ss                                |
|      |              |                                   |
| $N   | Date         | yyyy:mm:dd                        |
| $Y   | Year         | yyyy                              |
| $O   | Month        | mm                                |
| $D   | Day          | dd                                |
|      |              |                                   |
| $L   | LogLevel     | TRACE, DEBUG … FATAL              |
| $X   | LogLevel     | add " " if logLevel INFO or DEBUG |
| $F   | Func. Name   | main, foo                         |
| $A   | File Name    | main.c foo.c                      |
| $B   | Color Begin	 | from here the color starts        |
| $E   | Color End    | from here the color ends          |
| $C   | Log message  | Formatted Message with variables  |
| $Z   | new Line     | Add a Line Break To your message  |

### Implemented Features

1. **Log Levels:**
   - TRACE, DEBUG, INFO, WARNING, ERROR, and FATAL levels are supported to give you fine-grained control over the verbosity of your logs.

2. **Log Formatting:**
   - Customize log message formats, including timestamp, log level, and other relevant information, to enhance readability and analysis.

3. **Conditional Logging:**
   - Choose to enable or disable logging for specific log levels or modules based on your needs.

4. **Configurability:**
   - Enable dynamic configuration adjustments to logging settings, allowing developers to adapt the library to different environments easily.

5. **Buffering:**
   - Optimize logging performance with buffering mechanisms to reduce the overhead of frequent disk or network writes.

6. **Multithreading:**
    - Robust thread safety mechanisms, ensuring the library works seamlessly in multithreading environments.

7. **Thread Safety:**
   - Enhancements to achieve solid thread safety, effectively preventing race conditions and ensuring reliable logging in concurrent applications.

8. **Thread-Specific Logging:**
   - Achieve enhanced thread isolation by allowing each pthread (POSIX threads) to log to its dedicated log file. This feature ensures a clear separation of log entries based on the originating thread.Runtime Log 

9. **File Renaming:**
   - Enable the renaming of log files at runtime to facilitate better organization and management. This feature provides flexibility in updating log file names based on specific events or conditions, enhancing log file tracking and analysis.

10. **Log-Level Specific Formatting:**
   - Tailor the format of log messages for each log level independently. This feature allows you to customize the appearance of log entries based on their severity, making it easier to identify and prioritize issues during analysis.

### Planned Features

3. **Platform Support:**
   - Expand the library to support multiple platforms, making it versatile for various deployment scenarios.

4. **Log Rotation:**
   - Implement log rotation to prevent log files from growing too large, with configurable options for size limits, retention, and rotation intervals.

5. **Error Handling:**
   - Implement robust error handling mechanisms to gracefully handle situations like log file write failures and provide informative error messages.

6. **Asynchronous Logging:**
   - Introduce asynchronous logging capabilities to minimize performance impact and improve the responsiveness of your application.

## Getting Started

To get started with the C Logging Library, follow these steps:

1. Clone the repository as a submodule
2. Include the library header in your source code: `#include "logging.h"`
3. Start logging using the provided functions.

For more detailed instructions and examples, refer to the documentation.


## License

This project is licensed under the [MIT License](LICENSE). Feel free to use, modify, and distribute the library according to the terms of the license.

Happy logging! 📝
