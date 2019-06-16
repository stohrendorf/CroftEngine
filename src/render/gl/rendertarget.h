#pragma once

#include "bindableresource.h"

namespace render
{
namespace gl
{
class RenderTarget : public BindableResource
{
protected:
    explicit RenderTarget(const Allocator& allocator,
                          const Binder& binder,
                          const Deleter& deleter,
                          const ObjectIdentifier identifier,
                          const std::string& label)
        : BindableResource{allocator, binder, deleter, identifier, label}
    {
    }

public:
    virtual ::gl::GLint getWidth() const noexcept = 0;

    virtual ::gl::GLint getHeight() const noexcept = 0;
};
} // namespace gl
} // namespace render
