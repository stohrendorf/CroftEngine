#pragma once

#include "glassert.h"

#include <boost/assert.hpp>
#include <optional>

namespace gl
{
class RenderState final
{
public:
  RenderState(const RenderState&) = default;
  RenderState(RenderState&&) = default;
  RenderState& operator=(const RenderState&) = default;
  RenderState& operator=(RenderState&&) = default;
  explicit RenderState() = default;
  ~RenderState() = default;

  void apply(bool force = false) const;

  void setBlend(bool enabled)
  {
    m_blendEnabled = enabled;
  }

  void setBlendSrc(api::BlendingFactor blend)
  {
    m_blendSrc = blend;
  }

  void setBlendDst(api::BlendingFactor blend)
  {
    m_blendDst = blend;
  }

  void setCullFace(bool enabled)
  {
    m_cullFaceEnabled = enabled;
  }

  void setCullFaceSide(api::CullFaceMode side)
  {
    m_cullFaceSide = side;
  }

  void setFrontFace(api::FrontFaceDirection winding)
  {
    m_frontFace = winding;
  }

  void setDepthTest(bool enabled)
  {
    m_depthTestEnabled = enabled;
  }

  void setDepthWrite(bool enabled)
  {
    m_depthWriteEnabled = enabled;
  }

  void setDepthFunction(api::DepthFunction func)
  {
    m_depthFunction = func;
  }

  void setLineWidth(float width)
  {
    m_lineWidth = width;
  }

  void setLineSmooth(bool enabled)
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

    [[nodiscard]] T get() const
    {
      return value.value_or(DefaultValue);
    }

    void reset()
    {
      value.reset();
    }

    void setDefault()
    {
      value = DefaultValue;
    }

    [[nodiscard]] bool isInitialized() const
    {
      return value.has_value();
    }

    bool operator!=(const DefaultedOptional<T, DefaultValue>& rhs) const
    {
      return value != rhs.value;
    }

    DefaultedOptional<T, DefaultValue>& operator=(T rhs)
    {
      value = rhs;
      return *this;
    }

    void merge(const DefaultedOptional<T, DefaultValue>& other)
    {
      if(other.isInitialized())
        *this = other;
    }
  };

  struct DefaultedOptionalF final
  {
    const float DefaultValue;

    explicit DefaultedOptionalF(float defaultValue)
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

    [[nodiscard]] float get() const
    {
      return value.value_or(DefaultValue);
    }

    void reset()
    {
      value.reset();
    }

    void setDefault()
    {
      value = DefaultValue;
    }

    [[nodiscard]] bool isInitialized() const
    {
      return value.has_value();
    }

    bool operator!=(const DefaultedOptionalF& rhs) const
    {
      return value != rhs.value;
    }

    DefaultedOptionalF& operator=(float rhs)
    {
      value = rhs;
      return *this;
    }

    void merge(const DefaultedOptionalF& other)
    {
      if(other.isInitialized())
        *this = other;
    }
  };

  // States
  DefaultedOptional<bool, false> m_cullFaceEnabled;
  DefaultedOptional<bool, true> m_depthTestEnabled;
  DefaultedOptional<bool, true> m_depthWriteEnabled;
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
