#include "log.h"
#include <stdarg.h>
#include <map>
#include <functional>

namespace xie
{
  logEvent::logEvent(LogLevel::Level level, std::shared_ptr<Logger> logger, const char *file, int32_t line, uint32_t threadID, uint32_t fiberID, uint32_t elapse, uint64_t time) : m_file(file), m_line(line), m_threadId(threadID), m_fiberId(fiberID), m_elapse(elapse), m_time(time), m_logger(logger), m_level(level) {}
  const char *LogLevel::toString(LogLevel::Level level)
  {
    switch (level)
    {
#define XX(name)       \
  case LogLevel::name: \
    return #name;      \
    break;

      XX(DEBUG);
      XX(INFO);
      XX(WARN);
      XX(ERROR);
      XX(FATAL);
#undef XX
    default:
      return "UNKNOW";
    }
    return "UNKNOW";
  }
  void logEvent::format(const char *fmt, ...)
  {
    va_list all;
    va_start(all, fmt);
    format(fmt, all);
    va_end(all);
  }
  void logEvent::format(const char *fmt, va_list all)
  {
    char *buf = nullptr;
    int len = vasprintf(&buf, fmt, all);
    if (len != -1)
    {
      m_sscontent << std::string(buf, len);
      free(buf);
    }
  }

  LogEventWrap::LogEventWrap(logEvent::ptr e) : m_event(e)
  {
  }
  LogEventWrap::~LogEventWrap()
  {
    m_event->getLogger()->log(m_event->getLevel(), m_event);
  }
  std::stringstream &LogEventWrap::getSS()
  {
    return m_event->getss();
  }

  class MessageFormatItem : public logFormatter::formatItem
  {
  public:
    MessageFormatItem(const std::string &str = "") {}
    void format(std::shared_ptr<Logger> logger, std::ostream &ofs, LogLevel::Level level, logEvent::ptr event) override
    {
      ofs << event->getContent();
    }
  };
  class LevelFormatItem : public logFormatter::formatItem
  {
  public:
    LevelFormatItem(const std::string &str = "") {}

    void format(std::shared_ptr<Logger> logger, std::ostream &ofs, LogLevel::Level level, logEvent::ptr event) override
    {
      ofs << LogLevel::toString(level);
    }
  };
  class ElapseFormatItem : public logFormatter::formatItem
  {
  public:
    ElapseFormatItem(const std::string &str = "") {}

    void format(std::shared_ptr<Logger> logger, std::ostream &ofs, LogLevel::Level level, logEvent::ptr event) override
    {
      ofs << event->getElapse();
    }
  };
  class ThreadIDFormatItem : public logFormatter::formatItem
  {
  public:
    ThreadIDFormatItem(const std::string &str = "") {}

    void format(std::shared_ptr<Logger> logger, std::ostream &ofs, LogLevel::Level level, logEvent::ptr event) override
    {
      ofs << event->getThreadid();
    }
  };
  class FiberIDFormatItem : public logFormatter::formatItem
  {
  public:
    FiberIDFormatItem(const std::string &str = "") {}

    void format(std::shared_ptr<Logger> logger, std::ostream &ofs, LogLevel::Level level, logEvent::ptr event) override
    {
      ofs << event->getFiberID();
    }
  };
  class DataTimeFormatItem : public logFormatter::formatItem
  {
  public:
    DataTimeFormatItem(const std::string &format = "%Y-%m-%d %H:%M:%S") : m_format(format)
    {
      if (m_format.empty())
      {
        m_format = "%Y-%m-%d %H:%M:%S";
      }
    }
    void format(std::shared_ptr<Logger> logger, std::ostream &ofs, LogLevel::Level level, logEvent::ptr event) override
    {
      struct tm tm;
      time_t time = event->getTime();
      localtime_r(&(time), &tm);
      char buf[64];
      strftime(buf, 64, m_format.c_str(), &tm);
      ofs << buf;
    }

  private:
    std::string m_format;
  };
  class FileFormatItem : public logFormatter::formatItem
  {
  public:
    FileFormatItem(const std::string &str = "") {}

    void format(std::shared_ptr<Logger> logger, std::ostream &ofs, LogLevel::Level level, logEvent::ptr event) override
    {
      ofs << event->getFile();
    }
  };
  class LineFormatItem : public logFormatter::formatItem
  {
  public:
    LineFormatItem(const std::string &str = "") {}

    void format(std::shared_ptr<Logger> logger, std::ostream &ofs, LogLevel::Level level, logEvent::ptr event) override
    {
      ofs << event->getLine();
    }
  };
  class NewLineFormatItem : public logFormatter::formatItem
  {
  public:
    NewLineFormatItem(const std::string &str = "") {}

    void format(std::shared_ptr<Logger> logger, std::ostream &ofs, LogLevel::Level level, logEvent::ptr event) override
    {
      ofs << std::endl;
    }
  };
  class NameFormatItem : public logFormatter::formatItem
  {
  public:
    NameFormatItem(const std::string &str = "") {}

    void format(std::shared_ptr<Logger> logger, std::ostream &ofs, LogLevel::Level level, logEvent::ptr event) override
    {
      ofs << logger->getName();
    }
  };
  class StringFormatItem : public logFormatter::formatItem
  {
  public:
    StringFormatItem(const std::string &str) : m_str(str) {}
    void format(std::shared_ptr<Logger> logger, std::ostream &ofs, LogLevel::Level level, logEvent::ptr event) override
    {
      ofs << m_str;
    }

  private:
    std::string m_str;
  };
  class TabFormatItem : public logFormatter::formatItem
  {
  public:
    TabFormatItem(const std::string &str) : m_str(str) {}
    void format(std::shared_ptr<Logger> logger, std::ostream &ofs, LogLevel::Level level, logEvent::ptr event) override
    {
      ofs << "\t";
    }

  private:
    std::string m_str;
  };

  void Logger::addAppender(logAppender::ptr appender)
  {
    if (!appender->getFormat())
    {
      appender->setFormat(m_formatter);
    }
    m_appender.push_back(appender);
  }

  void Logger::delAppender(logAppender::ptr appender)
  {
    for (auto it = m_appender.begin(); it != m_appender.end(); it++)
    {
      if (*it == appender)
      {
        m_appender.erase(it);
        break;
      }
    }
  }

  Logger::Logger(const std::string &name) : m_name(name), m_level(LogLevel::DEBUG)
  {
    m_formatter.reset(new logFormatter("%d%T%t%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"));
  }

  void Logger::log(LogLevel::Level level, logEvent::ptr event)
  {
    if (level >= m_level)
    {
      auto self = shared_from_this();
      for (auto &i : m_appender)
      {
        i->log(self, level, event);
      }
    }
  }

  void Logger::debug(logEvent::ptr event)
  {
    log(LogLevel::DEBUG, event);
  }
  void Logger::info(logEvent::ptr event)
  {
    log(LogLevel::INFO, event);
  }
  void Logger::warn(logEvent::ptr event)
  {
    log(LogLevel::WARN, event);
  }
  void Logger::fatal(logEvent::ptr event)
  {
    log(LogLevel::FATAL, event);
  }
  void Logger::error(logEvent::ptr event)
  {
    log(LogLevel::ERROR, event);
  }

  FileLogAppender::FileLogAppender(const std::string &filename) : m_filename(filename)
  {
    reopen();
  }

  bool FileLogAppender::reopen()
  {
    if (m_filestream)
    {
      m_filestream.close();
    }
    m_filestream.open(m_filename);
    return !!m_filestream; //!!非0转为1，0还是0
  }

  void FileLogAppender::log(std::shared_ptr<Logger> logger, LogLevel::Level level, logEvent::ptr event)
  {
    if (level >= m_level)
    {
      m_filestream << m_formater->format(logger, level, event);
    }
  }

  void StdoutLogAppender::log(std::shared_ptr<Logger> logger, LogLevel::Level level, logEvent::ptr event)
  {
    if (level >= m_level)
    {
      std::cout << m_formater->format(logger, level, event);
    }
  }

  logFormatter::logFormatter(const std::string &pattern)
  {
    m_pattern = pattern;
    init();
  }
  std::string logFormatter::format(std::shared_ptr<Logger> logger, LogLevel::Level level, logEvent::ptr event)
  {
    std::stringstream ss;
    for (auto &i : m_item)
    {
      i->format(logger, ss, level, event);
    }
    return ss.str();
  }

  /*
      %m--消息体
      %p--level
      %r--启动到输出日志的时间
      %c--日志名称
      %t--线程id
      %n--回车换行
      %d--时间戳
      %f--文件名
      %l--行号
      %T--tab
      %F--协程id
  */
  void logFormatter::init()
  {
    std::vector<std::tuple<std::string, std::string, int>> vec; // str,format,type
    std::string str;
    for (size_t i = 0; i < m_pattern.size(); i++)
    {
      if (m_pattern[i] != '%')
      {
        str.append(1, m_pattern[i]);
        continue;
      }
      if ((i + 1) < m_pattern.size() && m_pattern[i + 1] == '%')
      {
        str.append(1, '%');
        continue;
      }
      size_t n = i + 1;
      int fmt_statue = 0;
      size_t fmt_begin = 0;

      std::string str1;
      std::string fmt;
      while (n < m_pattern.size())
      {
        if (!isalpha(m_pattern[n]) && m_pattern[n] != '{' && m_pattern[n] != '}')
        {
          break;
        }
        if (fmt_statue == 0)
        {
          if (m_pattern[n] == '{')
          {
            str1 = m_pattern.substr(i + 1, n - i - 1);
            fmt_statue = 1;
            fmt_begin = n;
            n++;
            continue;
          }
        }
        if (fmt_statue == 1)
        {
          if (m_pattern[n] == '}')
          {
            fmt = m_pattern.substr(fmt_begin + 1, n - fmt_begin - 1);
            fmt_statue = 2;
            break;
          }
        }
        n++;
      }
      if (fmt_statue == 0)
      {
        if (!str.empty())
        {
          vec.push_back(std::make_tuple(str, "", 0));
          str.clear();
        }
        str1 = m_pattern.substr(i + 1, n - i - 1);
        vec.push_back(std::make_tuple(str1, fmt, 1));
        i = n - 1;
      }
      else if (fmt_statue == 1)
      {
        // std::cout << "pattern error: " << m_pattern << " - " << m_pattern.substr(i) << std::endl;
        vec.push_back(std::make_tuple("<<error>>", fmt, 0));
      }
      else if (fmt_statue == 1)
      {
        if (!str.empty())
        {
          vec.push_back(std::make_tuple(str, "", 0));
          str.clear();
        }
        vec.push_back(std::make_tuple(str1, fmt, 1));
        i = n - 1;
      }
    }

    if (!str.empty())
    {
      vec.push_back(std::make_tuple(str, "", 0));
    }

    static std::map<std::string, std::function<formatItem::ptr(const std::string &str)>> s_format_items = {
#define XX(str, C)                                                           \
  {                                                                          \
    #str, [](const std::string &fmt) { return formatItem::ptr(new C(fmt)); } \
  }

        XX(m, MessageFormatItem),
        XX(p, LevelFormatItem),
        XX(r, ElapseFormatItem),
        XX(c, NameFormatItem),
        XX(t, ThreadIDFormatItem),
        XX(n, NewLineFormatItem),
        XX(d, DataTimeFormatItem),
        XX(f, FileFormatItem),
        XX(l, LineFormatItem),
        XX(T, TabFormatItem),
        XX(F, FiberIDFormatItem)
#undef XX
    };

    for (auto &i : vec)
    {
      if (std::get<2>(i) == 0)
      {
        m_item.push_back(formatItem::ptr(new StringFormatItem(std::get<0>(i))));
      }
      else
      {
        auto it = s_format_items.find(std::get<0>(i));
        if (it == s_format_items.end())
        {
          m_item.push_back(formatItem::ptr(new StringFormatItem("<<error_format %" + std::get<0>(i) + ">>")));
        }
        else
        {
          m_item.push_back(it->second(std::get<1>(i)));
        }
      }
      // std::cout << "{" << std::get<0>(i) << "} - {" << std::get<1>(i) << "} - {" << std::get<2>(i) << "}" << std::endl;
    }
  }

}