#pragma once

#include "bindableresource.h"

namespace render::gl
{
class RenderTarget : public BindableResource
{
protected:
  explicit RenderTarget(const Allocator& allocator,
                        const Binder& binder,
                        const Deleter& deleter,
                        const ::gl::ObjectIdentifier identifier,
                        const std::string& label)
      : BindableResource{allocator, binder, deleter, identifier, label}
  {
  }

public:
  [[nodiscard]] virtual int32_t getWidth() const noexcept = 0;

  [[nodiscard]] virtual int32_t getHeight() const noexcept = 0;
};
} // namespace render
