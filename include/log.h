#pragma once

#include <string>
#include <vector>
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
    const char *getFile() const { return m_file; }
    int32_t getLine() const { return m_line; }
    uint32_t getThreadid() const { return m_threadId; }
    uint32_t getFiberID() const { return m_fiberId; }
    uint32_t getElapse() const { return m_elapse; }
    uint64_t getTime() const { return m_time; }
    const std::string &getContent() const { return m_content; }

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
    static const char *toString(LogLevel::Level level);
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
      formatItem(const std::string &fmt = "") {}
      virtual ~formatItem() {}
      virtual void format(std::shared_ptr<Logger> logger, std::ofstream &ofs, LogLevel::Level level, logEvent::ptr event) = 0;
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
    const std::string &getName() const { return m_name; }

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
}