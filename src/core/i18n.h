#pragma once

#include <boost/format.hpp>
#include <libintl.h>

// NOLINTNEXTLINE(bugprone-reserved-identifier)
inline const char* _(const char* message)
{
  return gettext(message);
}

inline const char* N_(const char* message)
{
  return message;
}

template<typename... Args>
// NOLINTNEXTLINE(bugprone-reserved-identifier)
inline std::string _(const char* message, Args&&... args)
{
  return (boost::format(_(message)) % ... % std::forward<Args>(args)).str();
}

inline const char* P_(const char* singular, const char* plural, unsigned long n)
{
  return ngettext(singular, plural, n);
}

template<typename... Args>
inline std::string P_(const char* singular, const char* plural, unsigned long n, Args&&... args)
{
  return (boost::format(P_(singular, plural, n)) % ... % std::forward<Args>(args)).str();
}
