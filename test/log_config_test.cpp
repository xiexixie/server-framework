#include "log.h"
#include "config.h"
#include <iostream>

xie::ConfigVar<int>::ptr g_imt_value_cinfig = xie::Config::Lookup("system.port", (int)8080, "system port");

int main()
{
  XIE_LOG_INFO(XIE_LOG_ROOT()) << g_imt_value_cinfig->GetValue();
  XIE_LOG_INFO(XIE_LOG_ROOT()) << g_imt_value_cinfig->toString();

  std::cout << "hello\n";
  return 0;
}