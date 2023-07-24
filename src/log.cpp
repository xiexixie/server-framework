#include "log.h"

namespace xie
{
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
        i->log(level, event);
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

  void FileLogAppender::log(LogLevel::Level level, logEvent::ptr event)
  {
    if (level >= m_level)
    {
      m_filestream << m_formater->format(event);
    }
  }

  void StdoutLogAppender::log(LogLevel::Level level, logEvent::ptr event)
  {
    if (level >= m_level)
    {
      std::cout << m_formater->format(event);
    }
  }

}