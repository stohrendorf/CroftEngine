#pragma once

#include "glassert.h"

#include <boost/assert.hpp>
#include <optional>

namespace gl
{
class RenderState final
{
public:
  RenderState(const RenderState&) noexcept = default;
  RenderState(RenderState&&) noexcept = default;
  RenderState& operator=(const RenderState&) noexcept = default;
  RenderState& operator=(RenderState&&) noexcept = default;
  explicit RenderState() noexcept = default;
  ~RenderState() noexcept = default;

  void apply(bool force = false) const;

  void setBlend(const bool enabled)
  {
    m_blendEnabled = enabled;
  }

  void setBlendSrc(const api::BlendingFactor blend)
  {
    m_blendSrc = blend;
  }

  void setBlendDst(const api::BlendingFactor blend)
  {
    m_blendDst = blend;
  }

  void setCullFace(const bool enabled)
  {
    m_cullFaceEnabled = enabled;
  }

  void setCullFaceSide(const api::CullFaceMode side)
  {
    m_cullFaceSide = side;
  }

  void setFrontFace(const api::FrontFaceDirection winding)
  {
    m_frontFace = winding;
  }

  void setDepthTest(const bool enabled)
  {
    m_depthTestEnabled = enabled;
  }

  void setDepthWrite(const bool enabled)
  {
    m_depthWriteEnabled = enabled;
  }

  void setDepthClamp(const bool enabled)
  {
    m_depthClampEnabled = enabled;
  }

  void setDepthFunction(const api::DepthFunction func)
  {
    m_depthFunction = func;
  }

  void setLineWidth(const float width)
  {
    m_lineWidth = width;
  }

  void setLineSmooth(const bool enabled)
  {
    m_lineSmooth = enabled;
  }

  static void initDefaults();

  static void enableDepthWrite();

  void merge(const RenderState& other);

private:
  template<typename T, const T DefaultValue>
  struct DefaultedOptional final
  {
    std::optional<T> value{};

    [[nodiscard]] T get() const noexcept
    {
      return value.value_or(DefaultValue);
    }

    void reset() noexcept
    {
      value.reset();
    }

    void setDefault() noexcept
    {
      value = DefaultValue;
    }

    [[nodiscard]] bool isInitialized() const noexcept
    {
      return value.has_value();
    }

    bool operator!=(const DefaultedOptional<T, DefaultValue>& rhs) const noexcept
    {
      return value != rhs.value;
    }

    DefaultedOptional<T, DefaultValue>& operator=(T rhs) noexcept
    {
      value = rhs;
      return *this;
    }

    void merge(const DefaultedOptional<T, DefaultValue>& other) noexcept
    {
      if(other.isInitialized())
        *this = other;
    }
  };

  struct DefaultedOptionalF final
  {
    const float DefaultValue;

    explicit DefaultedOptionalF(const float defaultValue) noexcept
        : DefaultValue{defaultValue}
    {
    }

    DefaultedOptionalF& operator=(const DefaultedOptionalF& rhs)
    {
      BOOST_ASSERT(DefaultValue == rhs.DefaultValue);

      value = rhs.value;
      return *this;
    }

    std::optional<float> value{};

    [[nodiscard]] float get() const noexcept
    {
      return value.value_or(DefaultValue);
    }

    void reset() noexcept
    {
      value.reset();
    }

    void setDefault() noexcept
    {
      value = DefaultValue;
    }

    [[nodiscard]] bool isInitialized() const noexcept
    {
      return value.has_value();
    }

    bool operator!=(const DefaultedOptionalF& rhs) const noexcept
    {
      return value != rhs.value;
    }

    DefaultedOptionalF& operator=(float rhs) noexcept
    {
      value = rhs;
      return *this;
    }

    void merge(const DefaultedOptionalF& other) noexcept
    {
      if(other.isInitialized())
        *this = other;
    }
  };

  // States
  DefaultedOptional<bool, true> m_cullFaceEnabled;
  DefaultedOptional<bool, true> m_depthTestEnabled;
  DefaultedOptional<bool, true> m_depthWriteEnabled;
  DefaultedOptional<bool, false> m_depthClampEnabled;
  DefaultedOptional<api::DepthFunction, api::DepthFunction::Less> m_depthFunction;
  DefaultedOptional<bool, true> m_blendEnabled;
  DefaultedOptional<api::BlendingFactor, api::BlendingFactor::SrcAlpha> m_blendSrc;
  DefaultedOptional<api::BlendingFactor, api::BlendingFactor::OneMinusSrcAlpha> m_blendDst;
  DefaultedOptional<api::CullFaceMode, api::CullFaceMode::Back> m_cullFaceSide;
  DefaultedOptional<api::FrontFaceDirection, api::FrontFaceDirection::Cw> m_frontFace;
  DefaultedOptionalF m_lineWidth{1.0f};
  DefaultedOptional<bool, true> m_lineSmooth;

  static inline RenderState& getCurrentState();
};
} // namespace gl
