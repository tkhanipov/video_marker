/*

Copyright (c) 2014 Timur M. Khanipov <khanipov@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

*/

#pragma once

#include <ostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <vector>

//  ******* Logging utility *******
//
//  Allows to log messages with different importance levels
//  It dumps memory leaks information if executed under MS Visual Studio in debug mode
//
//  The logger can log any objects which are able to print themselves to std::ostream.
//
//  Usage examples:
//
//  LOG_ERROR("Error! File " << fileName << " not found")
//  LOG_DEBUG("i = " << i)
//  LOG_INFO("sound type: " << (type == vowel? "vowel" : "consonant"))    // NOTE: parentheses are needed!
//  LOG_WARNING("t = " << std::fixed << std::setprecision(1) << temperature << "; overheating")
//  LOG_TRACE(MyObject)
//
//  LOG_ASSERT(i >= 0)  // to disable this macro, use #define USE_LOG_ASSERT 0  before including the header
//
//  Logger::SetLogLevel(Logger::trace);
//  Logger::SetLogFile("mylog.txt", false)
//  Logger::Format() |= Logger::showFile | Logger::showLine
//  Logger::SetLogStream(&myFileStream, false);
//  oldFunc = Logger::SetLogFunction(myLogFunction);  // specify additional way to write log messages (for example, to a socket)
//
//  Logger::LogStatistics stats = Logger::GetLogStatistics();
//  if (stats.error > 0) alert();
//
//  RAW_LOG("This message will be logged unformatted (no time, level, function, etc) " << 1 << 2 << 3 << 4 << 5)
//
//  INSERT_AUTOLOGGER // this makes two trace records: the first at this line and the second at the end of the scope
//
//  #define LOGGING_LEVEL LOGGING_LEVEL_TRACE // this enables compilation of LOG_TRACE(msg) and LOG_DEBUG(msg)
//  #include <logger.h>
//
//  #define LOGGING_LEVEL LOGGING_LEVEL_NOTHING   // this disables all LOG_XXX(msg) calls
//  #include <logger.h>



namespace Logger {

  enum MessageLevel {trace, debug, info, warning, error};

  // Logger::format (see below) flags
  const int showFunction  = 1 << 0;   // show function name   
  const int showFile  = 1 << 1;   // show source file name
  const int showLine  = 1 << 2;   // show line number
  const int showTime  = 1 << 3;   // show time elapsed since program start (in seconds)
  
  // convenience
  struct SourceInfo
  {
    const char *function;
    const char *file;
    const unsigned line;
    explicit SourceInfo(const char *_function, const char *_file, const unsigned _line)
      : function(_function)
      , file(_file)
      , line(_line)
      { }
  };

  // RawLog-like function. Used in SetLogFunction()
  typedef void (*RawLogFunction)(const std::string &buffer, MessageLevel);

  // used in GetLogStatistics()
  struct LogStatistics
  {
    int trace, debug, info, warning, error, unknown;
    LogStatistics(int _trace, int _debug, int _info, int _warning, int _error, int _unknown)
      : trace(_trace)
      , debug(_debug)
      , info(_info)
      , warning(_warning)
      , error(_error)
      , unknown(_unknown)
      { }
  };

  // Writes message to the log with the given urgency level. This is the most important function
  void Log(MessageLevel level, const std::string &message, const SourceInfo *sourceInfo = 0);
  
  // Sets log level. All messages with level lesser than current log level will be dropped. Returns previous log level
  MessageLevel SetLogLevel(MessageLevel newLevel);

  // Redirects logging to the given file. In case of failure returns false and restores current log destination
  bool SetLogFile(const std::string &fileName, bool append = true);

  // Sets *newStream as the new logger output stream. Returns true if success
  // If needCleanUp is true then newStream will be deleted in the dtor or when changing log destination
  void SetLogStream(std::ostream *newStream, bool needCleanUp = true);

  /// Log messages format. You may set it using the |-combination of constants defined above
  long &Format();
  
  // Dumps log statistics
  void LogStats();

  // get log statistics (number of errors, warnings, etc)
  LogStatistics GetLogStatistics();

  // Writes buffer to the log without time, level, function name and all other formatting
  void RawLog(const std::string &buffer, MessageLevel);

  // Sets an additional function which will perform RawLog(), returns the previously set one
  // If newFunction is NULL, additional logging will be disabled
  RawLogFunction SetLogFunction(RawLogFunction newFunction);


  class Logger
  {
    public:
      void Log(MessageLevel level, const std::string &message, const SourceInfo *sourceInfo = 0);
      void RawLog(const std::string &buffer, MessageLevel messageLevel);
      bool SetLogFile(const std::string &fileName, bool append);
      void SetLogStream(std::ostream *newStream, bool __needCleanUp);
      RawLogFunction SetLogFunction(RawLogFunction);
      void LogStats();
      LogStatistics GetLogStatistics();
      
      long format;

    private:
      std::ostream *outputStream;
      MessageLevel logLevel;
      std::vector<unsigned long> stats;
      bool needCleanUp; // do we need to delete outputStream?
      RawLogFunction customLogFunction;
      
      Logger();
      ~Logger();

      friend class LoggerHolder;
      friend MessageLevel SetLogLevel(MessageLevel newLevel);
  };

  // singleton pattern and lazy initialization
  class LoggerHolder
  {
    public:
      LoggerHolder();
      ~LoggerHolder();
      static Logger &getLogger();
    private:
      static int count;
      static Logger *theLogger;
  };
}


namespace {
  Logger::LoggerHolder oneFile_oneHolder;   // to make sure that the Logger dtor is called after other dtors
}

// custom FULL_FUNCTION_NAME may be supplied
#ifndef FULL_FUNCTION_NAME
  #if defined (__GNUC__)
    #define FULL_FUNCTION_NAME  __PRETTY_FUNCTION__
  #elif defined (_MSC_VER)
    #define FULL_FUNCTION_NAME  __FUNCTION__
  #else
    #warning  "Function name macro is unknown"
    #define FULL_FUNCTION_NAME  "???"
  #endif
#endif

#define SAY_TO_LOG(level, msg)    { \
            std::ostringstream __logger_oss; \
            __logger_oss << msg; \
            Logger::SourceInfo si(FULL_FUNCTION_NAME, __FILE__, __LINE__); \
            Logger::Log(Logger::level, __logger_oss.str(), &si); \
          }

#define LOGGING_LEVEL_TRACE   0
#define LOGGING_LEVEL_DEBUG   1
#define LOGGING_LEVEL_INFO    2
#define LOGGING_LEVEL_WARNING   3
#define LOGGING_LEVEL_ERROR   4
#define LOGGING_LEVEL_NOTHING   5

#ifndef LOGGING_LEVEL
  #define LOGGING_LEVEL LOGGING_LEVEL_TRACE
#endif


#if LOGGING_LEVEL <= LOGGING_LEVEL_TRACE
  #define LOG_TRACE(msg)    SAY_TO_LOG(trace, msg)
#else
  #define LOG_TRACE(msg)  ;
#endif

#if LOGGING_LEVEL <= LOGGING_LEVEL_DEBUG
  #define LOG_DEBUG(msg)    SAY_TO_LOG(debug, msg)
#else
  #define LOG_DEBUG(msg)  ;
#endif

#if LOGGING_LEVEL <= LOGGING_LEVEL_INFO
  #define LOG_INFO(msg)   SAY_TO_LOG(info, msg)
#else
  #define LOG_INFO(msg) ;
#endif

#if LOGGING_LEVEL <= LOGGING_LEVEL_WARNING
  #define LOG_WARNING(msg)  SAY_TO_LOG(warning, msg)
#else
  #define LOG_WARNING(msg)  ;
#endif

#if LOGGING_LEVEL <= LOGGING_LEVEL_ERROR
  #define LOG_ERROR(msg)    SAY_TO_LOG(error, msg)
#else
  #define LOG_ERROR(msg)  ;
#endif

#ifndef USE_LOG_ASSERT
  #define USE_LOG_ASSERT 1
#endif

#if USE_LOG_ASSERT
  #define LOG_ASSERT(expr)  { if (!(expr)) LOG_ERROR("Assertion failed: " #expr)}
#else
  #define LOG_ASSERT(expr)  ;
#endif

class AutoLogger
{
  public:
    AutoLogger(const char *fn)
      :functionName(fn)
      { LOG_TRACE(">>> [" << stackLevel++ << "] " << functionName) }
    ~AutoLogger() { LOG_TRACE("<<< [" << --stackLevel << "] "  << functionName) }
  private:
    const char *functionName;
    static int stackLevel;
};

#if LOGGING_LEVEL <= LOGGING_LEVEL_TRACE
  #define INSERT_AUTOLOGGER   AutoLogger __an_autoLogger__(FULL_FUNCTION_NAME);
#else
  #define INSERT_AUTOLOGGER ;
#endif

#define RAW_LOG(msg)    { \
          std::ostringstream oss; \
          oss << msg; \
          Logger::RawLog(oss.str()); \
        }
