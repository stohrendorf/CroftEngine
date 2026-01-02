#pragma once

#include "api/gl.hpp"
#include "glassert.h"

#include <cstdint>
#include <filesystem>
#include <gsl-lite/gsl-lite.hpp>
#include <string>
#include <string_view>
#include <vector>

namespace gl
{
// NOLINTNEXTLINE(bugprone-reserved-identifier)
template<api::ShaderType _Type>
class Shader final
{
public:
  static constexpr api::ShaderType Type = _Type;

  explicit Shader(const gsl_lite::span<gsl_lite::czstring>& src, const std::string_view& label);

  Shader(const Shader&) = delete;
  Shader(Shader&&) = delete;
  void operator=(const Shader&) = delete;
  void operator=(Shader&&) = delete;

  ~Shader()
  {
    GL_ASSERT(api::deleteShader(m_handle));
  }

  [[nodiscard]] auto getHandle() const noexcept
  {
    return m_handle;
  }

  [[nodiscard]] std::string getInfoLog() const;

  [[nodiscard]] static Shader create(const std::filesystem::path& sourcePath,
                                     const std::vector<std::string>& defines,
                                     const std::string_view& label);

  [[nodiscard]] static Shader create(const std::filesystem::path& sourcePath,
                                     const std::string& source,
                                     const std::vector<std::string>& defines,
                                     const std::string_view& label);

private:
  uint32_t m_handle;
};

using FragmentShader = Shader<api::ShaderType::FragmentShader>;
using VertexShader = Shader<api::ShaderType::VertexShader>;
using GeometryShader = Shader<api::ShaderType::GeometryShader>;
} // namespace gl
