#pragma once

#include <string>
#include <vector>
#include <inttypes.h>
#include <memory>
#include <list>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include "singleton.h"
#include "util.h"

#define XIE_LOG_LEVEL(logger, level) \
  if (logger->getLevel() <= level)   \
  xie::LogEventWrap(xie::logEvent::ptr(new xie::logEvent(level, logger, __FILE__, __LINE__, xie::getThreadID(), xie::getFiberID(), 0, time(0)))).getSS()
#define XIE_LOG_DEBUG(logger) XIE_LOG_LEVEL(logger, xie::LogLevel::DEBUG)
#define XIE_LOG_INFO(logger) XIE_LOG_LEVEL(logger, xie::LogLevel::INFO)
#define XIE_LOG_WARN(logger) XIE_LOG_LEVEL(logger, xie::LogLevel::WARN)
#define XIE_LOG_ERROR(logger) XIE_LOG_LEVEL(logger, xie::LogLevel::ERROR)
#define XIE_LOG_FATAL(logger) XIE_LOG_LEVEL(logger, xie::LogLevel::FATAL)

#define XIE_LOG_FMT_LEVEL(logger, level, fmt, ...) \
  if (logger->getLevel() <= level)                 \
  xie::LogEventWrap(xie::logEvent::ptr(new xie::logEvent(level, logger, __FILE__, __LINE__, xie::getThreadID(), xie::getFiberID(), 0, time(0)))).getEvent()->format(fmt, __VA_ARGS__)
#define XIE_LOG_FMT_DEBUG(logger, fmt, ...) XIE_LOG_FMT_LEVEL(logger, xie::LogLevel::DEBUG, fmt, __VA_ARGS__)
#define XIE_LOG_FMT_INFO(logger, fmt, ...) XIE_LOG_FMT_LEVEL(logger, xie::LogLevel::INFO, fmt, __VA_ARGS__)
#define XIE_LOG_FMT_WARN(logger, fmt, ...) XIE_LOG_FMT_LEVEL(logger, xie::LogLevel::WARN, fmt, __VA_ARGS__)
#define XIE_LOG_FMT_ERROR(logger, fmt, ...) XIE_LOG_FMT_LEVEL(logger, xie::LogLevel::ERROR, fmt, __VA_ARGS__)
#define XIE_LOG_FMT_FATAL(logger, fmt, ...) XIE_LOG_FMT_LEVEL(logger, xie::LogLevel::FATAL, fmt, __VA_ARGS__)

#define XIE_LOG_ROOT() xie::LogMgr::GetInstance()->getRoot()

namespace xie
{
  class Logger;
  // 日志级别
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
    static const char *toString(LogLevel::Level level);
  };
  class logEvent
  {
  public:
    typedef std::shared_ptr<logEvent> ptr;
    logEvent(LogLevel::Level level, std::shared_ptr<Logger> logger, const char *file, int32_t line, uint32_t threadID, uint32_t fiberID, uint32_t elapse, uint64_t time);
    const char *getFile() const { return m_file; }
    int32_t getLine() const { return m_line; }
    uint32_t getThreadid() const { return m_threadId; }
    uint32_t getFiberID() const { return m_fiberId; }
    uint32_t getElapse() const { return m_elapse; }
    uint64_t getTime() const { return m_time; }
    std::string getContent() const { return m_sscontent.str(); }
    std::stringstream &getss() { return m_sscontent; }
    std::shared_ptr<Logger> getLogger() { return m_logger; }
    LogLevel::Level getLevel() { return m_level; }
    void format(const char *fmt, ...);
    void format(const char *fmt, va_list all);

  private:
    const char *m_file = nullptr;  // 文件名
    int32_t m_line = 0;            // 行号
    uint32_t m_threadId = 0;       // 线程ID
    uint32_t m_fiberId = 0;        // 协程ID
    uint32_t m_elapse = 0;         // 程序运行时间ms
    uint64_t m_time;               // 时间戳
    std::stringstream m_sscontent; // 内容

    std::shared_ptr<Logger> m_logger;
    LogLevel::Level m_level;
  };

  class LogEventWrap
  {
  public:
    LogEventWrap(logEvent::ptr e);
    ~LogEventWrap();
    std::stringstream &getSS();
    logEvent::ptr getEvent() { return m_event; }

  private:
    logEvent::ptr m_event;
  };

  // 日志格式
  class logFormatter
  {
  public:
    typedef std::shared_ptr<logFormatter> ptr;
    logFormatter(const std::string &pattern);
    std::string format(std::shared_ptr<Logger> logger, LogLevel::Level level, logEvent::ptr event); // 按照一定格式解析logEvent

    class formatItem
    {
    public:
      typedef std::shared_ptr<formatItem> ptr;
      // formatItem(const std::string &fmt = "") {}
      virtual ~formatItem() {}
      virtual void format(std::shared_ptr<Logger> logger, std::ostream &ofs, LogLevel::Level level, logEvent::ptr event) = 0;
    };

    void init(); // 初始化日志格式

  private:
    std::string m_pattern;
    std::vector<formatItem::ptr> m_item;
  };

  // 日志输出
  class logAppender
  {
  public:
    typedef std::shared_ptr<logAppender> ptr;
    virtual ~logAppender() {} // 设置为虚析构函数吗，可以使子类调用自己的析构函数
    virtual void log(std::shared_ptr<Logger> logger, LogLevel::Level level, logEvent::ptr event) = 0;
    void setFormat(logFormatter::ptr val) { m_formater = val; }
    logFormatter::ptr getFormat() const { return m_formater; }
    void setLevel(LogLevel::Level levle) { m_level = levle; }

  protected:
    LogLevel::Level m_level = LogLevel::DEBUG;
    logFormatter::ptr m_formater;
  };

  // 日志
  class Logger : public std::enable_shared_from_this<Logger>
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
    const std::string &getName() const { return m_name; }

  private:
    std::string m_name;                     // 日志名称
    LogLevel::Level m_level;                // 日志级别
    std::list<logAppender::ptr> m_appender; // Appender集合
    logFormatter::ptr m_formatter;
  };

  // 输出到控制台的appender
  class StdoutLogAppender : public logAppender
  {
  public:
    typedef std::shared_ptr<StdoutLogAppender> ptr;
    void log(std::shared_ptr<Logger> logger, LogLevel::Level level, logEvent::ptr event) override;
  };
  // 输出到文件的appender
  class FileLogAppender : public logAppender
  {
  public:
    typedef std::shared_ptr<FileLogAppender> ptr;
    FileLogAppender(const std::string &filename);
    void log(std::shared_ptr<Logger> logger, LogLevel::Level level, logEvent::ptr event) override;
    bool reopen(); // 重新打开文件，文件打开成功，返回true

  private:
    std::string m_filename;
    std::ofstream m_filestream;
  };

  class LogManager
  {
  public:
    LogManager();
    Logger::ptr getLogger(const std::string &name);
    void init();
    Logger::ptr getRoot() const { return m_root; }

  private:
    std::map<std::string, Logger::ptr> m_loggers;
    Logger::ptr m_root;
  };
  typedef Singleton<LogManager> LogMgr;
}