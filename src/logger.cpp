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

#if defined _MSC_VER && defined _DEBUG
  #define MSVS_LEAK_CHECK
  #define _CRTDBG_MAP_ALLOC
  #include <stdlib.h>
  #include <crtdbg.h>
#endif

#include <iostream>
#include <fstream>
#include <iomanip>
#include <ctime>
#include <sstream>
#include "logger.h"

namespace Logger {

MessageLevel SetLogLevel(MessageLevel newLevel)
{
  Logger &logger = LoggerHolder::getLogger();
  MessageLevel old = logger.logLevel;
  logger.logLevel = newLevel;
  return old;
}
  
void Log(MessageLevel level, const std::string &message, const SourceInfo *sourceInfo)
{
  LoggerHolder::getLogger().Log(level, message, sourceInfo);
}

void RawLog(const std::string &buf, MessageLevel messageLevel)
{
  LoggerHolder::getLogger().RawLog(buf, messageLevel);
}

bool SetLogFile(const std::string &fileName, bool append)
{
  return LoggerHolder::getLogger().SetLogFile(fileName, append);
}

void LogStats()
{
  LoggerHolder::getLogger().LogStats();
}

LogStatistics GetLogStatistics()
{
  return LoggerHolder::getLogger().GetLogStatistics();
}

long &Format()
{
  return LoggerHolder::getLogger().format;
}

void SetLogStream(std::ostream *newStream, bool needCleanUp)
{
  LoggerHolder::getLogger().SetLogStream(newStream, needCleanUp);
}

RawLogFunction SetLogFunction(RawLogFunction newFunction)
{
  return LoggerHolder::getLogger().SetLogFunction(newFunction);
}

Logger::Logger()
  : logLevel(trace)
  , outputStream(&std::cerr)
  , format(showTime | showFunction)
  , stats(error - trace + 2, 0)   // +2 because we're gonna use stats[error+1] for unknown levels
  , needCleanUp(false)
  , customLogFunction(0)
{
  Log(info, "************ Logging started ************");
  const time_t timer = time(0);
  char buf[256];
  sprintf(buf, "Current date & time: %s", ctime(&timer));
  Log(info, buf);
}

void Logger::RawLog(const std::string &buf, MessageLevel messageLevel)
{
  *outputStream << buf << std::flush;
  if (customLogFunction)
    customLogFunction(buf, messageLevel);
}

void Logger::Log(MessageLevel level, const std::string &msg, const SourceInfo *si)
{
  if (level < trace || level > error)
    level = MessageLevel(error + 1);
  stats[level]++;

  if (level < logLevel)
    return;

  std::ostringstream oss;
  clock_t ticks;
  if (format & showTime && (ticks = clock()) != -1)
    oss << std::left << std::setw(8) << std::fixed << std::setprecision(2) << double(ticks) / CLOCKS_PER_SEC << ' ';

  oss.width(8);
  switch (level)
  {
    case trace:
      oss << "TRACE";
      break;
    case debug:
      oss << "DEBUG";
      break;
    case info:
      oss << "INFO";
      break;
    case warning:
      oss << "WARNING";
      break;
    case error:
      oss << "ERROR";
      break;
    default:
      oss << "?";
      break;
  }

  oss.width(1);
  oss << "  " << msg;

  if (si && format & showFunction)
    oss << " [" << si->function << "]";
  if (si && format & showFile)
  {
    if (format & showLine)
      oss << " [" << si->file << ":" << si->line << "]";
    else
      oss << " [" << si->file << "]";
  }
  oss << std::endl;
  
  RawLog(oss.str(), level);
}

void Logger::LogStats()
{
  std::stringstream oss;
  oss << std::endl;
  oss <<  "******* LOG STATISTICS *******" << std::endl;
  oss << "Error  : " << stats[error] << std::endl;
  oss << "Warning: " << stats[warning] << std::endl;
  oss << "Info   : " << stats[info] << std::endl;
  oss << "Debug  : " << stats[debug] << std::endl;
  oss << "Trace  : " << stats[trace] << std::endl;
  oss << "Unknown: " << stats[error+1] << std::endl;
  oss << std::endl;
  RawLog(oss.str(), info);
}

LogStatistics Logger::GetLogStatistics()
{
  return LogStatistics(stats[trace], stats[debug], stats[info], stats[warning], stats[error], stats[error+1]);
}

RawLogFunction Logger::SetLogFunction(RawLogFunction newFunction)
{
  RawLogFunction oldFunc = customLogFunction;
  if (newFunction)
    Log(info, "*** Setting new additional logging function ***");
  else
    Log(info, "*** Disabling additional logging function ***");
  customLogFunction = newFunction;
  return oldFunc;
}

void Logger::SetLogStream(std::ostream *newStream, bool __needCleanUp)
{
  if (!newStream)
  {
    Log(error, "cannot log to a NULL-stream");
    return;
  }
  
  Log(info, "*** Log destination changed ***");

  if (needCleanUp)
    delete outputStream;

  outputStream = newStream;
  needCleanUp = __needCleanUp;
  Log(info, "************ Logging started ************");
  const time_t timer = time(0);
  char buf[256];
  sprintf(buf, "Current date & time: %s", ctime(&timer));
  Log(info, buf);
}

bool Logger::SetLogFile(const std::string &fileName, bool append)
{
  Log(info, "*** Changing log destination to " + fileName);
  std::ofstream *fp = new std::ofstream(fileName.c_str(), append?  std::ios::app | std::ios::out  : std::ios::out);
  if (!*fp)
  {
    Log(error, "Failed to change log destination. Using the previously set one");
    delete fp;
    return false;
  }

  SetLogStream(fp, true);
  return true;
}

Logger::~Logger()
{
  Log(info, "************ Logging ended ************");
  LogStats();
  if (needCleanUp)
    delete outputStream;
}

int LoggerHolder::count = 0;
Logger *LoggerHolder::theLogger = 0;

LoggerHolder::LoggerHolder()
{
  count++;
}

LoggerHolder::~LoggerHolder()
{
  if (--count == 0 && theLogger)
  {
    delete theLogger;
    
#if defined MSVS_LEAK_CHECK
    _CrtDumpMemoryLeaks();
#endif
    
    theLogger = 0;
  }
}

Logger &LoggerHolder::getLogger()
{
  if (!theLogger)
    theLogger = new Logger;
  return *theLogger;
}

} // namespace Logger

int AutoLogger::stackLevel = 0;
