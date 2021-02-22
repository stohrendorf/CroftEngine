#include "i18nprovider.h"

#include "core/pybindmodule.h"

#include <boost/log/trivial.hpp>

namespace engine
{
I18nProvider::I18nProvider(const pybind11::dict& src, const std::string& language)
{
  for(const auto& [key, name] : EnumUtil<I18n>::all())
  {
    const auto values = src[pybind11::cast(key)];
    Expects(!values.is_none());
    if(const auto loc = core::get<std::string>(values, language))
    {
      m_i18n.emplace(key, loc.value());
    }
    else if(const auto en = core::get<std::string>(values, "en"))
    {
      m_i18n.emplace(key, en.value());
    }
    else
    {
      BOOST_LOG_TRIVIAL(error) << "Missing i18n: " << name;
      m_i18n.emplace(key, std::string{"MISSING i18n: "} + name);
    }
  }
}

std::string I18nProvider::operator()(I18n key) const
{
  return m_i18n.at(key);
}
} // namespace engine
