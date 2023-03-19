#pragma once

#include "api/gl.hpp" // IWYU pragma: export
#include "glassert.h"
#include "resource.h"

#include <boost/assert.hpp>
#include <functional>
#include <string_view>

namespace gl
{
// NOLINTNEXTLINE(bugprone-reserved-identifier)
template<api::ObjectIdentifier _Identifier>
class BindableResource : public Resource<_Identifier>
{
public:
  using Resource<_Identifier>::Identifier;
  using Resource<_Identifier>::getHandle;
  using Allocator = typename Resource<_Identifier>::Allocator;
  using Deleter = typename Resource<_Identifier>::Deleter;

  void bind() const
  {
    if(getHandle() == 0)
      return;

    GL_ASSERT(m_binder(getHandle()));
  }

  void unbind() const
  {
    GL_ASSERT(m_binder(0));
  }

protected:
  using Binder = std::function<void(uint32_t)>;

  explicit BindableResource(Allocator allocator, Binder binder, Deleter deleter, const std::string_view& label)
      : Resource<Identifier>{allocator, deleter, label}
      , m_binder{std::move(binder)}
  {
    BOOST_ASSERT(static_cast<bool>(m_binder));
  }

  virtual ~BindableResource()
  {
    if(getHandle() != 0)
      unbind();
  }

private:
  Binder m_binder;
};
} // namespace gl
