# C Logging Library

## Overview

Welcome to the C Logging Library repository! This library is designed to provide a simple and efficient logging solution for C applications. While the current version is tailored for Linux environments, we have exciting plans to expand its features and support multiple platforms in the future.

## Features

### Formating
Formating the LogMessages can be customised with the following tags<br>
to format all following Log Messages use: set_Formating(char* format);<br>
e.g. set_Formating("$B[$T] $L [$F]  $C$E")  or set_Formating("$BTime:[$M $S] $L $E ==> $C")

| Code | Description | Format/Example  |
|------|-------------|-----------------|
| $T   | Time        | hh:mm:ss        |
| $H   | Time Hour   | hh              |
| $M   | Time Min.   | mm              |
| $S   | Time Sec.   | ss              |
|      |             |                 |
| $N   | Date        | yyyy:mm:dd      |
| $Y	 | Year	      | yyyy            |
| $O	 | Month	      | mm              |
| $D	 | Day	      | dd              |
|      |             |                 |
| $L	 | LogLevel    | [TRACE], [DEBUG]|
| $F	 | Func. Name  | main, foo       |
| $A	 | File Name	| main.c foo.c    |
| $B	 | Color Begin	| only on Linux   |
| $E	 | Color End	| only on Linux   |
| $C	 | Log message |                 |


### Usage

<pre>
```C
#include <logger.h>

// Call this at the start of your programm
int log_init("logfile.log", "$B[$T] $L [$F] $C$E");

// You can change the message formating at runtime
void set_Formating("$B[$T] $A-$F$E $C");

// You can also revert back to your previous Format
void use_Formating_Backup();

// Define witch log levels should be written to log file directly and witch should be buffered
//  0    =>   write all logs directly to log file
//  1    =>   buffer: TRACE
//  2    =>   buffer: TRACE + DEBUG
//  3    =>   buffer: TRACE + DEBUG + INFO
//  4    =>   buffer: TRACE + DEBUG + INFO + WARN
void set_buffer_Level(2);

// To log some information use one of the following macros
CS_TRACE("Your message goes here us standart formating: int: %d, string: %s", someInt, someString)
CS_DEBUG("Your message goes here us standart formating: int: %d, string: %s", someInt, someString)
CS_INFO(" Your message goes here us standart formating: int: %d, string: %s", someInt, someString)
CS_WARN(" Your message goes here us standart formating: int: %d, string: %s", someInt, someString)
CS_ERROR("Your message goes here us standart formating: int: %d, string: %s", someInt, someString)
CS_FATAL("Your message goes here us standart formating: int: %d, string: %s", someInt, someString)

// Call this to automaticly log function Starts
CL_LOG_FUNC_START("")
CL_LOG_FUNC_START("start param1: %d", someInt)

// Call this to automaticly log the successfull End of a function
CL_LOG_FUNC_END("")
CL_LOG_FUNC_END("start param1: %d", someInt)

// Use this vaitation to make check some consition and log diffrent messages
CL_VALIDATE(expr, messageSuccess, messageFailure)

// Use this assert to make sure your condition is true
// The CL_ASSERT macro inserts a 'return RetVal’ if consition != true 
CL_ASSERT(expr, messageSuccess, messageFailure, RetVal, ...)


// Call this at the end of your programm to push all buffered messages into the log file
void log_shutdown();

```
</pre>

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

### Planned Features

1. **Multithreading:**
   - Introduce thread safety mechanisms to ensure the library works seamlessly in multithreaded environments.

2. **Platform Support:**
   - Expand the library to support multiple platforms, making it versatile for various deployment scenarios.

3. **Log Rotation:**
   - Implement log rotation to prevent log files from growing too large, with configurable options for size limits, retention, and rotation intervals.

4. **Thread Safety:**
   - Enhance the library's thread safety to prevent race conditions and ensure reliable logging in concurrent applications.

5. **Integration with System Logging:**
   - Provide integration with the system logging facility on different platforms, enhancing interoperability.

6. **Error Handling:**
   - Implement robust error handling mechanisms to gracefully handle situations like log file write failures and provide informative error messages.

7. **Asynchronous Logging:**
   - Introduce asynchronous logging capabilities to minimize performance impact and improve the responsiveness of your application.

## Getting Started

To get started with the C Logging Library, follow these steps:

1. Clone the repository as a submodule
2. Include the library header in your source code: `#include "logging.h"`
3. Start logging using the provided functions.

For more detailed instructions and examples, refer to the documentation.

## Contributing

We welcome contributions! If you have ideas, bug reports, or want to contribute code, please open an issue or submit a pull request. Check out our [contribution guidelines](CONTRIBUTING.md) for more details.

## License

This project is licensed under the [MIT License](LICENSE). Feel free to use, modify, and distribute the library according to the terms of the license.

Happy logging! 📝
