#include "log.h"
#include <map>
#include <functional>

namespace xie
{
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

  class MessageFormatItem : public logFormatter::formatItem
  {
  public:
    MessageFormatItem(const std::string &str) : formatItem(str) {}
    void format(std::shared_ptr<Logger> logger, std::ofstream &ofs, LogLevel::Level level, logEvent::ptr event) override
    {
      ofs << event->getContent();
    }
  };
  class LevelFormatItem : public logFormatter::formatItem
  {
  public:
    void format(std::shared_ptr<Logger> logger, std::ofstream &ofs, LogLevel::Level level, logEvent::ptr event) override
    {
      ofs << LogLevel::toString(level);
    }
  };
  class ElapseFormatItem : public logFormatter::formatItem
  {
  public:
    void format(std::shared_ptr<Logger> logger, std::ofstream &ofs, LogLevel::Level level, logEvent::ptr event) override
    {
      ofs << event->getElapse();
    }
  };
  class ThreadIDFormatItem : public logFormatter::formatItem
  {
  public:
    void format(std::shared_ptr<Logger> logger, std::ofstream &ofs, LogLevel::Level level, logEvent::ptr event) override
    {
      ofs << event->getThreadid();
    }
  };
  class FiberIDFormatItem : public logFormatter::formatItem
  {
  public:
    void format(std::shared_ptr<Logger> logger, std::ofstream &ofs, LogLevel::Level level, logEvent::ptr event) override
    {
      ofs << event->getFiberID();
    }
  };
  class DataTimeFormatItem : public logFormatter::formatItem
  {
  public:
    DataTimeFormatItem(const std::string &format = "%Y:%m:%d %H:%M:%S") : m_format(format) {}
    void format(std::shared_ptr<Logger> logger, std::ofstream &ofs, LogLevel::Level level, logEvent::ptr event) override
    {
      ofs << event->getTime();
    }

  private:
    std::string m_format;
  };
  class FileFormatItem : public logFormatter::formatItem
  {
  public:
    void format(std::shared_ptr<Logger> logger, std::ofstream &ofs, LogLevel::Level level, logEvent::ptr event) override
    {
      ofs << event->getFile();
    }
  };
  class LineFormatItem : public logFormatter::formatItem
  {
  public:
    void format(std::shared_ptr<Logger> logger, std::ofstream &ofs, LogLevel::Level level, logEvent::ptr event) override
    {
      ofs << event->getLine();
    }
  };
  class NewLineFormatItem : public logFormatter::formatItem
  {
  public:
    void format(std::shared_ptr<Logger> logger, std::ofstream &ofs, LogLevel::Level level, logEvent::ptr event) override
    {
      ofs << std::endl;
    }
  };
  class NameFormatItem : public logFormatter::formatItem
  {
  public:
    void format(std::shared_ptr<Logger> logger, std::ofstream &ofs, LogLevel::Level level, logEvent::ptr event) override
    {
      ofs << logger->getName();
    }
  };
  class StringFormatItem : public logFormatter::formatItem
  {
  public:
    StringFormatItem(const std::string &str) : m_str(str), formatItem(str) {}
    void format(std::shared_ptr<Logger> logger, std::ofstream &ofs, LogLevel::Level level, logEvent::ptr event) override
    {
      ofs << m_str;
    }

  private:
    std::string m_str;
  };

  void Logger::addAppender(logAppender::ptr appender)
  {
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

  Logger::Logger(const std::string &name) : m_name(name) {}

  void Logger::log(LogLevel::Level level, logEvent::ptr event)
  {
    if (level >= m_level)
    {
      for (auto &i : m_appender)
      {
        i->log(, level, event);
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

  FileLogAppender::FileLogAppender(const std::string &filename) : m_filename(filename) {}

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
        if (isspace(m_pattern[n]))
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
      }
      if (fmt_statue == 0)
      {
        if (!str.empty())
        {
          vec.push_back(std::make_tuple(str, "", 0));
        }
        str1 = m_pattern.substr(i + 1, n - i - 1);
        vec.push_back(std::make_tuple(str1, fmt, 1));
        i = n;
      }
      else if (fmt_statue == 1)
      {
        std::cout << "pattern error: " << m_pattern << " - " << m_pattern.substr(i) << std::endl;
        vec.push_back(std::make_tuple("<<error>>", fmt, 0));
      }
      else if (fmt_statue == 1)
      {
        if (!str.empty())
        {
          vec.push_back(std::make_tuple(str, "", 0));
        }
        vec.push_back(std::make_tuple(str1, fmt, 1));
        i = n;
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
/*
        XX(m, MessageFormatItem),
        XX(p,LevelFormatItem),
        XX(r,ElapseFormatItem),
        XX(c,NameFormatItem),
        XX(t,ThreadIDFormatItem),
        XX(n,NewLineFormatItem),
        XX(d,DataTimeFormatItem),
        XX(f,FileFormatItem),
        XX(l,LineFormatItem)*/
#undef XX
    };
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

}