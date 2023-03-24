#include "helpers.h"

#include "core/i18n.h"
#include "ui/util.h"

#include <boost/format.hpp>
#include <boost/log/trivial.hpp>
#include <boost/throw_exception.hpp>
#include <chrono>
#include <cstdlib>
#include <glm/gtc/type_ptr.hpp>
#include <gsl/gsl-lite.hpp>
#include <sstream>
#include <stdexcept>

namespace util
{
std::string unescape(const std::string& escaped)
{
  std::string result;
  bool addUmlautDots = false;
  for(const char c : escaped)
  {
    if(c == ui::UmlautDots)
    {
      addUmlautDots = true;
      continue;
    }

    if(addUmlautDots)
    {
      switch(c)
      {
      case 'a':
        result += "ä";
        break;
      case 'o':
        result += "ö";
        break;
      case 'u':
        result += "ü";
        break;
      case 'A':
        result += "Ä";
        break;
      case 'O':
        result += "Ö";
        break;
      case 'U':
        result += "Ü";
        break;
      default:
        BOOST_LOG_TRIVIAL(warning) << "Cannot add umlaut dots to character " << c;
        result += c;
        break;
      }
      addUmlautDots = false;
      continue;
    }

    if(c == ui::SzLig)
    {
      result += "ß";
      continue;
    }

    result += c;
  }

  return result;
}

std::string escape(const std::string& unescaped)
{
  std::string result;
  for(const uint8_t c : unescaped)
  {
    switch(c)
    {
    case 0xE4:
      result += ui::UmlautDots;
      result += 'a';
      break;
    case 0xC4:
      result += ui::UmlautDots;
      result += 'A';
      break;
    case 0xF6:
      result += ui::UmlautDots;
      result += 'o';
      break;
    case 0xD6:
      result += ui::UmlautDots;
      result += 'O';
      break;
    case 0xFC:
      result += ui::UmlautDots;
      result += 'u';
      break;
    case 0xDC:
      result += ui::UmlautDots;
      result += 'U';
      break;
    case 0xDF:
      result += ui::SzLig;
      break;
    default:
      result += gsl::narrow_cast<char>(c);
      break;
    }
  }

  return result;
}

std::filesystem::path ensureFileExists(const std::filesystem::path& path)
{
  if(!std::filesystem::is_regular_file(path))
  {
    BOOST_LOG_TRIVIAL(fatal) << "Could not find required file " << path;
    BOOST_THROW_EXCEPTION(std::runtime_error("required file not found"));
  }
  return path;
}

core::TRVec pitch(const core::TRVec& vec, const core::Angle& rot)
{
  const auto s = sin(rot);
  const auto c = cos(rot);
  return core::TRVec{(vec.Z.cast<float>() * s + vec.X.cast<float>() * c).cast<core::Length>(),
                     vec.Y,
                     (vec.Z.cast<float>() * c - vec.X.cast<float>() * s).cast<core::Length>()};
}

core::TRVec pitch(const core::Length& len, const core::Angle& rot, const core::Length& dy)
{
  return core::TRVec{sin(len, rot), dy, cos(len, rot)};
}

constexpr float BitShiftHackThreshold = 1.0f / (1u << 14u);

core::Length cos(const core::Length& len, const core::Angle& rot)
{
  auto tmp = len.cast<float>() * cos(rot);
  if(tmp.get() < -BitShiftHackThreshold)
    tmp -= (1_len).cast<float>();
  return tmp.cast<core::Length>();
}

core::Length sin(const core::Length& len, const core::Angle& rot)
{
  auto tmp = len.cast<float>() * sin(rot);
  if(tmp.get() < -BitShiftHackThreshold)
    tmp -= (1_len).cast<float>();
  return tmp.cast<core::Length>();
}

glm::mat4 mix(const glm::mat4& a, const glm::mat4& b, const float bias)
{
  glm::mat4 result{0.0f};
  const auto ap = value_ptr(a);
  const auto bp = value_ptr(b);
  const auto rp = value_ptr(result);
  for(int i = 0; i < 16; ++i)
    rp[i] = ap[i] * (1 - bias) + bp[i] * bias;
  return result;
}

int16_t rand15s()
{
  return static_cast<int16_t>(rand15() - Rand15Max / 2);
}

int16_t rand15()
{
  // NOLINTNEXTLINE(cert-msc50-cpp, concurrency-mt-unsafe)
  return gsl::narrow_cast<int16_t>(std::rand() % Rand15Max);
}

std::string toTimeStr(const core::Seconds& t)
{
  static constexpr std::chrono::seconds Minute = std::chrono::seconds{60};
  static constexpr std::chrono::seconds Hour = 60 * Minute;
  if(t.get() >= Hour.count())
  {
    return /* translators: TR charmap encoding */ _("%1%:%2$02d:%3$02d",
                                                    t.get() / Hour.count(),
                                                    (t.get() / Minute.count())
                                                      % std::chrono::duration_cast<std::chrono::minutes>(Hour).count(),
                                                    t.get() % Minute.count());
  }
  else
  {
    return /* translators: TR charmap encoding */ _(
      "%1$02d:%2$02d", t.get() / Minute.count(), t.get() % Minute.count());
  }
}
} // namespace util
