#pragma once

#include "api/gl.hpp" // IWYU pragma: export
#include "glassert.h"
#include "sampler.h"
#include "texture.h"

#include <gslu.h>
#include <type_traits>
#include <utility>

namespace gl
{
// NOLINTNEXTLINE(bugprone-reserved-identifier)
template<typename TTexture>
class TextureHandle final
{
public:
  TextureHandle(const TextureHandle<TTexture>&) = delete;
  TextureHandle(TextureHandle<TTexture>&&) = delete;
  void operator=(const TextureHandle<TTexture>&) = delete;
  void operator=(TextureHandle<TTexture>&&) = delete;

  using Texture = TTexture;
  static_assert(std::is_base_of_v<::gl::Texture, Texture>);

  explicit TextureHandle(gslu::nn_shared<Texture> texture, gslu::nn_unique<Sampler>&& sampler)
      : m_texture{std::move(texture)}
      , m_sampler{std::move(sampler)}
      , m_handle{GL_ASSERT_FN(api::getTextureSamplerHandle(m_texture->getHandle(), m_sampler->getHandle()))}
  {
    gsl_Ensures(m_handle != 0);
    if(m_texture->m_textureHandleReferences[m_handle]++ == 0)
    {
      GL_ASSERT(api::makeTextureHandleResident(m_handle));
    }
    else
    {
      gsl_Assert(gl::api::isTextureHandleResident(m_handle));
    }
  }

  ~TextureHandle()
  {
    if(--m_texture->m_textureHandleReferences[m_handle] == 0)
    {
// #define NVIDIA_NSIGHT_HACK
#ifdef NVIDIA_NSIGHT_HACK
      api::makeTextureHandleNonResident(m_handle);
      api::getError();
#else
      GL_ASSERT(api::makeTextureHandleNonResident(m_handle));
#endif
      m_texture->m_textureHandleReferences.erase(m_handle);
    }
    else
    {
      gsl_Assert(gl::api::isTextureHandleResident(m_handle));
    }
  }

  [[nodiscard]] auto getHandle() const
  {
    return m_handle;
  }

  [[nodiscard]] const auto& getTexture() const
  {
    return m_texture;
  }

private:
  gslu::nn_shared<Texture> m_texture;
  gslu::nn_unique<Sampler> m_sampler;
  uint64_t m_handle;
};
} // namespace gl
