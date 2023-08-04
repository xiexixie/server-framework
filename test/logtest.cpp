#include <iostream>
#include "log.h"
#include "util.h"

int main()
{
  xie::Logger::ptr logger(new xie::Logger);
  logger->addAppender(xie::logAppender::ptr(new xie::StdoutLogAppender));
  xie::logEvent::ptr event(new xie::logEvent(__FILE__, __LINE__, xie::getThreadID(), xie::getFiberID(), 0, time(0)));
  event->getss() << "test logger";
  logger->log(xie::LogLevel::DEBUG, event);
}