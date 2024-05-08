#include "i18n.h"

#include <boost/log/trivial.hpp>
#include <cstdlib>
#include <gsl/gsl-lite.hpp>
#include <sstream>

namespace core
{
void setLocale(const std::filesystem::path& poDir, const std::string& locale)
{
  BOOST_LOG_TRIVIAL(info) << "Using locales from " << poDir;

  if(const auto* const result = setlocale(LC_MESSAGES, locale.c_str()); result != nullptr)
  {
    BOOST_LOG_TRIVIAL(trace) << "gettext setlocale result: " << result;
  }
  else
  {
    BOOST_LOG_TRIVIAL(warning) << "failed to set gettext locale";
  }

#ifdef WIN32
  gsl_Assert(_putenv_s("LANG", locale.c_str()) == 0);
  BOOST_LOG_TRIVIAL(trace) << "gettext text domain: " << textdomain("croftengine");
  if(wbindtextdomain("croftengine", poDir.c_str()) == nullptr)
  {
    BOOST_LOG_TRIVIAL(warning) << "failed to bind text domain";
  }
#else
  gsl_Assert(setenv("LANG", locale.c_str(), true) == 0);
#endif

  if(const auto result = bindtextdomain("croftengine", poDir.string().c_str()); result != nullptr)
  {
    BOOST_LOG_TRIVIAL(trace) << "gettext bindtextdomain result: " << result;
  }
  else
  {
    BOOST_LOG_TRIVIAL(warning) << "failed to bind text domain";
  }

  if(const auto result = bind_textdomain_codeset("croftengine", "UTF-8"); result != nullptr)
  {
    BOOST_LOG_TRIVIAL(trace) << "gettext bind_textdomain_codeset result: " << result;
  }
  else
  {
    BOOST_LOG_TRIVIAL(warning) << "failed to set textdomain codeset";
  }

  if(const auto result = textdomain("croftengine"))
  {
    BOOST_LOG_TRIVIAL(trace) << "gettext textdomain result: " << result;
  }
  else
  {
    BOOST_LOG_TRIVIAL(warning) << "failed to set textdomain";
  }

  if(/* translators: insert the locale here, this is an internal check if valid translations exist */ _(
       "translation-test-message")
     == std::string{"translation-test-message"})
  {
    BOOST_LOG_TRIVIAL(warning) << "Missing translations for " << locale;
  }
}
} // namespace core
