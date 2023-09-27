#include <iostream>
#include "log.h"
#include "util.h"

int main()
{
  xie::Logger::ptr logger(new xie::Logger);
  logger->addAppender(xie::logAppender::ptr(new xie::StdoutLogAppender));
  xie::logAppender::ptr fileappender(new xie::FileLogAppender("./log.txt"));
  logger->addAppender(fileappender);
  xie::logFormatter::ptr fmt(new xie::logFormatter("%d%T%m%n"));
  fileappender->setFormat(fmt);
  fileappender->setLevel(xie::LogLevel::ERROR);
  // xie::logEvent::ptr event(new xie::logEvent(__FILE__, __LINE__, xie::getThreadID(), xie::getFiberID(), 0, time(0)));
  // event->getss() << "test logger";
  // logger->log(xie::LogLevel::DEBUG, event);
  XIE_LOG_DEBUG(logger) << "test logger debug";
  XIE_LOG_INFO(logger) << "test logger info";
  XIE_LOG_FMT_ERROR(logger, "test debugfmt %s", "aa");
}