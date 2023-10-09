#pragma once

#include <memory>
#include <string>
#include <sstream>
#include <map>
#include <boost/lexical_cast.hpp>
#include "log.h"

namespace xie
{
  class ConfigVarBase
  {
  public:
    typedef std::shared_ptr<ConfigVarBase> ptr;
    ConfigVarBase(const std::string &name, const std::string &description = "") : m_name(name), m_description(description) {}
    virtual ~ConfigVarBase() {}
    const std::string &GetName() const { return m_name; }
    const std::string &GetDescription() const { return m_description; }
    virtual std::string toString() = 0;
    virtual bool fromString(const std::string &val) = 0;

  protected:
    std::string m_name;
    std::string m_description;
  };

  template <typename T>
  class ConfigVar : public ConfigVarBase
  {
  public:
    typedef std::shared_ptr<ConfigVar> ptr;
    ConfigVar(const std::string &name, const std::string &description, const T &val) : m_val(val), ConfigVarBase(name, description) {}
    std::string toString() override
    {
      try
      {
        return boost::lexical_cast<std::string>(m_val);
      }
      catch (std::exception &e)
      {
        XIE_LOG_ERROR(XIE_LOG_ROOT()) << "ConFigVar:toString exception" << e.what() << " convert: " << typeid(m_val).name() << "to string.";
      }
      return "";
    }
    bool fromString(const std::string &val) override
    {
      try
      {
        m_val = boost::lexical_cast<T>(val);
      }
      catch (std::exception &e)
      {
        XIE_LOG_ERROR(XIE_LOG_ROOT()) << "ConFigVar:toString exception" << e.what() << " convert: string to" << typeid(m_val).name();
      }
      return false;
    }
    const T GetValue() const { return m_val; }
    void setValue(const T &v) { m_val = v; }

  private:
    T m_val;
  };

  class Config
  {
  public:
    typedef std::map<std::string, ConfigVarBase::ptr> ConfigVarMap;

    template <typename T>
    static typename ConfigVar<T>::ptr Lookup(const std::string &name)
    {
      auto it = s_datas.find(name);
      if (it == s_datas.end())
      {
        return nullptr;
      }
      return std::dynamic_pointer_cast<ConfigVar<T>>(it->second);
    }

    template <typename T>
    static typename ConfigVar<T>::ptr Lookup(const std::string &name, const T &value, const std::string &description = "")
    {
      auto tmp = Lookup<T>(name);
      if (tmp)
      {
        XIE_LOG_INFO(XIE_LOG_ROOT()) << "Look up name: " << name << " exists";
        return tmp;
      }
      if (name.find_first_not_of("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz._0123456789") != std::string::npos)
      {
        XIE_LOG_ERROR(XIE_LOG_ROOT()) << "Look up name invalid: " << name;
        throw std::invalid_argument(name);
      }
      typename ConfigVar<T>::ptr v(new ConfigVar<T>(name, description, value));
      s_datas[name] = v;
      return v;
    }

  private:
    static ConfigVarMap s_datas;
  };
}