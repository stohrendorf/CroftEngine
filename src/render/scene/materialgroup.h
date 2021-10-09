#pragma once

#include "rendermode.h"

#include <boost/throw_exception.hpp>
#include <memory>
#include <stdexcept>

namespace render::scene
{
class Material;

class MaterialGroup final
{
public:
  MaterialGroup() = default;

  [[nodiscard]] const auto& get(const RenderMode mode) const
  {
    switch(mode)
    {
    case RenderMode::Full:
      return m_full;
    case RenderMode::CSMDepthOnly:
      return m_csmDepthOnly;
    case RenderMode::DepthOnly:
      return m_depthOnly;
    default:
      BOOST_THROW_EXCEPTION(std::domain_error("Invalid render mode"));
    }
  }

  MaterialGroup& set(const RenderMode& mode, const std::shared_ptr<Material>& material)
  {
    switch(mode)
    {
    case RenderMode::Full:
      m_full = material;
      break;
    case RenderMode::CSMDepthOnly:
      m_csmDepthOnly = material;
      break;
    case RenderMode::DepthOnly:
      m_depthOnly = material;
      break;
    default:
      BOOST_THROW_EXCEPTION(std::domain_error("Invalid render mode"));
    }
    return *this;
  }

private:
  std::shared_ptr<Material> m_full{nullptr};
  std::shared_ptr<Material> m_csmDepthOnly{nullptr};
  std::shared_ptr<Material> m_depthOnly{nullptr};
};
} // namespace render::scene
