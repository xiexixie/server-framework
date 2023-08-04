#include "util.h"

namespace xie
{
  uint32_t getThreadID()
  {
    return getpid();
  }

  uint32_t getFiberID()
  {
    return 0;
  }
}