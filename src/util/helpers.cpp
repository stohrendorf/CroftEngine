#include "helpers.h"

#include <boost/log/trivial.hpp>

namespace util
{
std::string unescape(const std::string& escaped)
{
  std::string result;
  bool addUmlautDots = false;
  for(const char c : escaped)
  {
    if(c == '~')
    {
      addUmlautDots = true;
      continue;
    }

    if(addUmlautDots)
    {
      switch(c)
      {
      case 'a': result += u8"ä"; break;
      case 'o': result += u8"ö"; break;
      case 'u': result += u8"ü"; break;
      case 'A': result += u8"Ä"; break;
      case 'O': result += u8"Ö"; break;
      case 'U': result += u8"Ü"; break;
      default:
        BOOST_LOG_TRIVIAL(warning) << "Cannot add umlaut dots to character " << c;
        result += c;
        break;
      }
      addUmlautDots = false;
      continue;
    }

    if(c == '=')
    {
      result += u8"ß";
      continue;
    }

    result += c;
  }

  return result;
}
} // namespace util
