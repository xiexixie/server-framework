#include <iostream>
#include "log.h"

int main()
{
  xie::Logger::ptr logger(new xie::Logger);
  logger->addAppender(xie::logAppender::ptr(new xie::StdoutLogAppender));
  xie::logEvent::ptr event(new xie::logEvent(__FILE__, __LINE__, 0, 1, 2, time(0)));
  logger->log(xie::LogLevel::DEBUG, event);
}