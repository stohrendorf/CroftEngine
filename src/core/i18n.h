#pragma once

#include <boost/format.hpp>
#include <clocale>
#include <cstring>
#include <filesystem>
#include <string>
#include <utility>

#ifdef _MSC_VER
#  define fprintf
#endif
#include <libintl.h>
#ifdef _MSC_VER
#  undef fprintf
#endif

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

inline const char* dcpgettext(const char* domain, const char* msg_ctxt, const char* msgid, int category)
{
  auto msg_ctxt_id = std::string{msg_ctxt} + '\004' + msgid;
  const auto translation = dcgettext(domain, msg_ctxt_id.c_str(), category);
  if(msg_ctxt_id == translation)
  {
    return msgid;
  }
  else
  {
    return translation;
  }
}

inline const char* dpgettext(const char* domain, const char* msg_ctxt, const char* msgid)
{
  return dcpgettext(domain, msg_ctxt, msgid, LC_MESSAGES);
}

inline const char* pgettext(const char* msg_ctxt, const char* msgid)
{
  return dpgettext(textdomain(nullptr), msg_ctxt, msgid);
}

template<typename... Args>
inline std::string pgettext(const char* msg_ctxt, const char* msgid, Args&&... args)
{
  return (boost::format(pgettext(msg_ctxt, msgid)) % ... % std::forward<Args>(args)).str();
}

inline const char* dcnpgettext(
  const char* domain, const char* msg_ctxt, const char* msgid, const char* msgid_plural, unsigned long n, int category)
{
  const auto msg_ctxt_id = std::string{msg_ctxt} + '\004' + msgid;
  const auto translation = dcngettext(domain, msg_ctxt_id.c_str(), msgid_plural, n, category);
  if(translation == msg_ctxt_id || std::strcmp(translation, msgid_plural) == 0)
  {
    return n == 1 ? msgid : msgid_plural;
  }
  else
  {
    return translation;
  }
}

inline const char*
  dnpgettext(const char* domain, const char* msg_ctxt, const char* msgid, const char* msgid_plural, unsigned long n)
{
  return dcnpgettext(domain, msg_ctxt, msgid, msgid_plural, n, LC_MESSAGES);
}

inline const char* npgettext(const char* msg_ctxt, const char* msgid, const char* msgid_plural, unsigned long n)
{
  return dnpgettext(textdomain(nullptr), msg_ctxt, msgid, msgid_plural, n);
}

namespace core
{
extern void setLocale(const std::filesystem::path& poDir, const std::string& locale);
}
