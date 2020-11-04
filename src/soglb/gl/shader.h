#pragma once

#include "api/soglb_core.hpp"
#include "bindableresource.h"

#include <gsl-lite.hpp>

namespace gl
{
template<api::ShaderType _Type>
class Shader final
{
public:
  static constexpr api::ShaderType Type = _Type;

  explicit Shader(const std::string& label = {})
      : m_handle{GL_ASSERT_FN(api::createShader(Type))}
  {
    Expects(m_handle != 0);

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

  // ReSharper disable once CppMemberFunctionMayBeConst
  void setSource(const std::string& src)
  {
    gsl::czstring data[1]{src.c_str()};
    GL_ASSERT(api::shaderSource(m_handle, 1, data, nullptr));
  }

  // ReSharper disable once CppMemberFunctionMayBeConst
  void setSource(gsl::czstring src[], const api::core::SizeType n)
  {
    GL_ASSERT(api::shaderSource(m_handle, n, src, nullptr));
  }

  // ReSharper disable once CppMemberFunctionMayBeConst
  void compile()
  {
    GL_ASSERT(api::compileShader(m_handle));
  }

  auto compile(gsl::czstring src[], const api::core::SizeType n)
  {
    setSource(src, n);
    compile();
    return getCompileStatus();
  }

  auto compile(const gsl::czstring src)
  {
    gsl::czstring srcs[]{src};
    return compile(srcs, 1);
  }

  [[nodiscard]] bool getCompileStatus() const
  {
    auto success = static_cast<int>(api::Boolean::False);
    GL_ASSERT(api::getShader(m_handle, api::ShaderParameterName::CompileStatus, &success));
    return success == static_cast<int>(api::Boolean::True);
  }

  [[nodiscard]] std::string getInfoLog() const
  {
    int32_t length = 0;
    GL_ASSERT(api::getShader(m_handle, api::ShaderParameterName::InfoLogLength, &length));
    if(length == 0)
    {
      length = 4096;
    }
    if(length > 0)
    {
      const gsl::owner<gsl::zstring> infoLog = new char[length];
      GL_ASSERT(api::getShaderInfoLog(m_handle, length, nullptr, infoLog));
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
};

using FragmentShader = Shader<api::ShaderType::FragmentShader>;
using VertexShader = Shader<api::ShaderType::VertexShader>;
} // namespace gl
