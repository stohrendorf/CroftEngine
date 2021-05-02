#pragma once

#include "api/gl.hpp"
#include "glassert.h"

#include <boost/log/trivial.hpp>
#include <gsl/gsl-lite.hpp>

namespace gl
{
// NOLINTNEXTLINE(bugprone-reserved-identifier)
template<api::ShaderType _Type>
class Shader final
{
public:
  static constexpr api::ShaderType Type = _Type;

  explicit Shader(const gsl::span<gsl::czstring>& src, const std::string& label = {})
      : m_handle{GL_ASSERT_FN(api::createShader(Type))}
  {
    Expects(m_handle != 0);
    GL_ASSERT(api::shaderSource(m_handle, gsl::narrow<api::core::SizeType>(src.size()), src.data(), nullptr));
    GL_ASSERT(api::compileShader(m_handle));

    auto success = static_cast<int>(api::Boolean::False);
    GL_ASSERT(api::getShader(m_handle, api::ShaderParameterName::CompileStatus, &success));
    if(success != static_cast<int>(api::Boolean::True))
    {
      int32_t length = 0;
      GL_ASSERT(api::getShader(m_handle, api::ShaderParameterName::InfoLogLength, &length));
      if(length == 0)
      {
        length = 4096;
      }
      if(length > 0)
      {
        std::vector<char> infoLog(length, '\0');
        GL_ASSERT(api::getShaderInfoLog(m_handle, length, nullptr, infoLog.data()));
        infoLog.back() = '\0';
        std::string result = infoLog.data();
        BOOST_LOG_TRIVIAL(error) << "Failed to compile shader program";
        BOOST_LOG_TRIVIAL(error) << infoLog.data();
        BOOST_THROW_EXCEPTION(std::runtime_error("Failed to compile shader program"));
      }
    }

    if(!label.empty())
    {
      GL_ASSERT(api::objectLabel(api::ObjectIdentifier::Shader, m_handle, -1, label.c_str()));
    }
  }

  Shader(const Shader&) = delete;
  Shader(Shader&&) = delete;
  Shader& operator=(const Shader&) = delete;
  Shader& operator=(Shader&&) = delete;

  ~Shader()
  {
    GL_ASSERT(api::deleteShader(m_handle));
  }

  [[nodiscard]] auto getHandle() const noexcept
  {
    return m_handle;
  }

private:
  const uint32_t m_handle;
};

using FragmentShader = Shader<api::ShaderType::FragmentShader>;
using VertexShader = Shader<api::ShaderType::VertexShader>;
} // namespace gl
