#pragma once

#include <unistd.h>
#include <inttypes.h>

namespace xie
{
  // 获取当前线程id,以int类型返回
  uint32_t getThreadID();
  uint32_t getFiberID();
}