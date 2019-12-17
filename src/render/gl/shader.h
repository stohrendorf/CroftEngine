#pragma once

#include "bindableresource.h"
#include "gsl-lite.hpp"

#include <render/gl/api/soglb_core.hpp>

namespace render::gl
{
class Shader final
{
public:
  explicit Shader(const ::gl::ShaderType type, const std::string& label = {})
      : m_handle{GL_ASSERT_FN(::gl::createShader(type))}
      , m_type{type}
  {
    BOOST_ASSERT(type == ::gl::ShaderType::VertexShader || type == ::gl::ShaderType::FragmentShader);
    Expects(m_handle != 0);

    if(!label.empty())
    {
      GL_ASSERT(::gl::objectLabel(::gl::ObjectIdentifier::Shader, m_handle, -1, label.c_str()));
    }
  }

  Shader(const Shader&) = delete;

  Shader(Shader&&) = delete;

  Shader& operator=(const Shader&) = delete;

  Shader& operator=(Shader&&) = delete;

  ~Shader()
  {
    GL_ASSERT(::gl::deleteShader(m_handle));
  }

  [[nodiscard]] auto getType() const noexcept
  {
    return m_type;
  }

  // ReSharper disable once CppMemberFunctionMayBeConst
  void setSource(const std::string& src)
  {
    gsl::czstring data[1]{src.c_str()};
    GL_ASSERT(::gl::shaderSource(m_handle, 1, data, nullptr));
  }

  // ReSharper disable once CppMemberFunctionMayBeConst
  void setSource(gsl::czstring src[], const ::gl::core::SizeType n)
  {
    GL_ASSERT(::gl::shaderSource(m_handle, n, src, nullptr));
  }

  // ReSharper disable once CppMemberFunctionMayBeConst
  void compile()
  {
    GL_ASSERT(::gl::compileShader(m_handle));
  }

  [[nodiscard]] bool getCompileStatus() const
  {
    auto success = static_cast<int>(::gl::Boolean::False);
    GL_ASSERT(::gl::getShader(m_handle, ::gl::ShaderParameterName::CompileStatus, &success));
    return success == static_cast<int>(::gl::Boolean::True);
  }

  [[nodiscard]] std::string getInfoLog() const
  {
    int32_t length = 0;
    GL_ASSERT(::gl::getShader(m_handle, ::gl::ShaderParameterName::InfoLogLength, &length));
    if(length == 0)
    {
      length = 4096;
    }
    if(length > 0)
    {
      const gsl::owner<gsl::zstring> infoLog = new char[length];
      GL_ASSERT(::gl::getShaderInfoLog(m_handle, length, nullptr, infoLog));
      infoLog[length - 1] = '\0';
      std::string result = infoLog;
      delete[] infoLog;
      return result;
    }

    return {};
  }

  [[nodiscard]] auto getHandle() const noexcept
  {
    return m_handle;
  }

private:
  const uint32_t m_handle;

  const ::gl::ShaderType m_type;
};
} // namespace render::gl
