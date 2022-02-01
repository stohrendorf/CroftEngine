#pragma once

#include "api/gl.hpp" // IWYU pragma: export
#include "detail/argsholder.h"
#include "glassert.h"

#include <glm/gtc/type_ptr.hpp>
#include <gslu.h>
#include <string_view>
#include <tuple>
#include <utility>

namespace gl
{
class Sampler final
{
public:
  Sampler(const Sampler&) = delete;
  void operator=(const Sampler&) = delete;
  Sampler(Sampler&&) = delete;
  void operator=(Sampler&&) = delete;

  explicit Sampler(const std::string_view& label)
  {
    GL_ASSERT(api::genSampler(1, &m_handle));
    Expects(m_handle != 0);

    set(api::SamplerParameterI::TextureWrapS, api::TextureWrapMode::ClampToEdge);
    set(api::SamplerParameterI::TextureWrapT, api::TextureWrapMode::ClampToEdge);
    set(api::TextureMinFilter::Nearest);
    set(api::TextureMagFilter::Nearest);
    Expects(api::isSampler(m_handle));

    if(!label.empty())
      GL_ASSERT(api::objectLabel(
        api::ObjectIdentifier::Sampler, m_handle, gsl::narrow<api::core::SizeType>(label.size()), label.data()));
  }

  ~Sampler()
  {
    GL_ASSERT(api::deleteSampler(1, &m_handle));
  }

  [[nodiscard]] auto getHandle() const
  {
    return m_handle;
  }

  Sampler& set(const api::TextureMinFilter value)
  {
    GL_ASSERT(
      api::samplerParameter(getHandle(), api::SamplerParameterI::TextureMinFilter, static_cast<int32_t>(value)));
    return *this;
  }

  Sampler& set(const api::TextureMagFilter value)
  {
    GL_ASSERT(
      api::samplerParameter(getHandle(), api::SamplerParameterI::TextureMagFilter, static_cast<int32_t>(value)));
    return *this;
  }

  Sampler& set(const api::TextureCompareMode value)
  {
    GL_ASSERT(
      api::samplerParameter(getHandle(), api::SamplerParameterI::TextureCompareMode, static_cast<int32_t>(value)));
    return *this;
  }

  Sampler& set(const api::DepthFunction value)
  {
    GL_ASSERT(
      api::samplerParameter(getHandle(), api::SamplerParameterI::TextureCompareFunc, static_cast<int32_t>(value)));
    return *this;
  }

  Sampler& set(const api::SamplerParameterI param, const api::TextureWrapMode value)
  {
    GL_ASSERT(api::samplerParameter(getHandle(), param, static_cast<int32_t>(value)));
    return *this;
  }

  Sampler& set(const api::SamplerParameterF param, const float value)
  {
    GL_ASSERT(api::samplerParameter(getHandle(), param, value));
    return *this;
  }

  Sampler& setBorderColor(const glm::vec4& value)
  {
    GL_ASSERT(api::samplerParameter(getHandle(), api::SamplerParameterF::TextureBorderColor, glm::value_ptr(value)));
    return *this;
  }

private:
  uint32_t m_handle = 0;
};
} // namespace gl

template<typename... Args>
inline gslu::nn_unique<gl::Sampler> operator|(gslu::nn_unique<gl::Sampler>&& sampler,
                                              const gl::detail::ArgsHolder<Args...>& setter)
{
  std::apply(
    [&sampler](const Args&... args)
    {
      sampler->set(args...);
    },
    setter.args);
  return std::move(sampler);
}
