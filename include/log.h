#pragma once

#include <string>
#include <inttypes.h>
#include <memory>
#include <list>
#include <fstream>
#include <sstream>
#include <iostream>

namespace xie
{

  class logEvent
  {
  public:
    typedef std::shared_ptr<logEvent> ptr;
    logEvent();

  private:
    const char *m_file = nullptr; // 文件名
    int32_t m_line = 0;           // 行号
    uint32_t m_threadId = 0;      // 线程ID
    uint32_t m_fiberId = 0;       // 协程ID
    uint32_t m_elapse = 0;        // 程序运行时间ms
    uint64_t m_time;              // 时间戳
    std::string m_content;        // 内容
  };

  class LogLevel
  {
  public:
    enum Level
    {
      DEBUG,
      INFO,
      WARN,
      ERROR,
      FATAL
    };
  };

  // 日志格式
  class logFormatter
  {
  public:
    typedef std::shared_ptr<logFormatter> ptr;
    std::string format(logEvent::ptr event);
  };

  // 日志输出
  class logAppender
  {
  public:
    typedef std::shared_ptr<logAppender> ptr;
    virtual ~logAppender() {}
    virtual void log(LogLevel::Level level, logEvent::ptr event) = 0;
    void setFormat(logFormatter::ptr val) { m_formater = val; }
    logFormatter::ptr getFormat() const { return m_formater; }

  protected:
    LogLevel::Level m_level;
    logFormatter::ptr m_formater;
  };

  // 日志
  class Logger
  {
  public:
    typedef std::shared_ptr<Logger> ptr;

    Logger(const std::string &name = "root");
    void log(LogLevel::Level level, logEvent::ptr event);
    void debug(logEvent::ptr event);
    void info(logEvent::ptr event);
    void warn(logEvent::ptr event);
    void fatal(logEvent::ptr event);
    void error(logEvent::ptr event);
    void addAppender(logAppender::ptr appender);
    void delAppender(logAppender::ptr appender);
    LogLevel::Level getLevel() const { return m_level; }
    void setLevel(LogLevel::Level level) { m_level = level; }

  private:
    std::string m_name;                     // 日志名称
    LogLevel::Level m_level;                // 日志级别
    std::list<logAppender::ptr> m_appender; // Appender集合
  };

  // 输出到控制台的appender
  class StdoutLogAppender : public logAppender
  {
  public:
    typedef std::shared_ptr<StdoutLogAppender> ptr;
    void log(LogLevel::Level level, logEvent::ptr event) override;
  };
  // 输出到文件的appender
  class FileLogAppender : public logAppender
  {
  public:
    typedef std::shared_ptr<FileLogAppender> ptr;
    FileLogAppender(const std::string &filename);
    void log(LogLevel::Level level, logEvent::ptr event) override;
    bool reopen(); // 重新打开文件，文件打开成功，返回true

  private:
    std::string m_filename;
    std::ofstream m_filestream;
  };
}