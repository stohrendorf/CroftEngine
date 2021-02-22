#pragma once

#include "i18n.h"

#include <boost/format.hpp>
#include <pybind11/pybind11.h>

namespace engine
{
class I18nProvider
{
public:
  explicit I18nProvider(const pybind11::dict& src, const std::string& language);

  std::string operator()(I18n key) const;

  template<typename... Args>
  std::string operator()(I18n key, Args&&... args) const
  {
    return (boost::format(operator()(key)) % ... % std::forward<Args>(args)).str();
  }

private:
  std::unordered_map<I18n, std::string> m_i18n;
};
} // namespace engine
