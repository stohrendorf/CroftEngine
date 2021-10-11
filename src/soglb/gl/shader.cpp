#include "shader.h"

#include "api/gl.hpp"
#include "glassert.h"

#include <boost/log/trivial.hpp>
#include <boost/throw_exception.hpp>
#include <iosfwd>
#include <stdexcept>
#include <vector>

namespace gl
{
// NOLINTNEXTLINE(bugprone-reserved-identifier)
template<api::ShaderType _Type>
Shader<_Type>::Shader(const gsl::span<gsl::czstring>& src, const std::string_view& label)
    : m_handle{GL_ASSERT_FN(api::createShader(Type))}
{
  Expects(m_handle != 0);
  GL_ASSERT(api::shaderSource(m_handle, gsl::narrow<api::core::SizeType>(src.size()), src.data(), nullptr));
  GL_ASSERT(api::compileShader(m_handle));

  if(const auto log = getInfoLog(); !log.empty())
    BOOST_LOG_TRIVIAL(debug) << "Shader info log: " << log;

  auto success = static_cast<int>(api::Boolean::False);
  GL_ASSERT(api::getShader(m_handle, api::ShaderParameterName::CompileStatus, &success));
  if(success != static_cast<int>(api::Boolean::True))
  {
    BOOST_LOG_TRIVIAL(error) << "Failed to compile shader program " << label;
    BOOST_THROW_EXCEPTION(std::runtime_error("Failed to compile shader program"));
  }

  if(!label.empty())
  {
    GL_ASSERT(api::objectLabel(
      api::ObjectIdentifier::Shader, m_handle, gsl::narrow<api::core::SizeType>(label.size()), label.data()));
  }
}

// NOLINTNEXTLINE(bugprone-reserved-identifier)
template<api::ShaderType _Type>
std::string Shader<_Type>::getInfoLog() const
{
  int32_t length = 0;
  GL_ASSERT(api::getShader(m_handle, api::ShaderParameterName::InfoLogLength, &length));
  if(length == 0)
  {
    length = 4096;
  }
  if(length <= 0)
    return {};

  std::vector<char> infoLog(length, '\0');
  GL_ASSERT(api::getShaderInfoLog(m_handle, length, nullptr, infoLog.data()));
  infoLog.back() = '\0';
  return infoLog.data();
}

template class Shader<api::ShaderType::FragmentShader>;
template class Shader<api::ShaderType::ComputeShader>;
template class Shader<api::ShaderType::GeometryShader>;
template class Shader<api::ShaderType::TessControlShader>;
template class Shader<api::ShaderType::TessEvaluationShader>;
template class Shader<api::ShaderType::VertexShader>;
} // namespace gl
