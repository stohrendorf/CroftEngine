#pragma once

#include "api/gl.hpp"
#include "glassert.h"

#include <cstdint>
#include <gsl/gsl-lite.hpp>
#include <string>

namespace gl
{
// NOLINTNEXTLINE(bugprone-reserved-identifier)
template<api::ShaderType _Type>
class Shader final
{
public:
  static constexpr api::ShaderType Type = _Type;

  explicit Shader(const gsl::span<gsl::czstring>& src, const std::string& label = {});

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
