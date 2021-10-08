#pragma once

#include "api/gl.hpp"
#include "glassert.h"

namespace gl
{
class FenceSync final
{
public:
  explicit FenceSync()
      : m_sync{GL_ASSERT_FN(api::fenceSync(api::SyncCondition::SyncGpuCommandsComplete, api::SyncBehaviorFlags::None))}
  {
  }

  ~FenceSync()
  {
    GL_ASSERT(api::deleteSync(m_sync));
  }

  void wait() const
  {
    GL_ASSERT(api::waitSync(m_sync, api::SyncBehaviorFlags::None, api::TimeoutIgnored));
  }

  // NOLINTNEXTLINE(modernize-use-nodiscard)
  api::SyncStatus clientWait() const
  {
    return GL_ASSERT_FN(api::clientWaitSync(m_sync, api::SyncObjectMask::SyncFlushCommandsBit, api::TimeoutIgnored));
  }

  // NOLINTNEXTLINE(modernize-use-nodiscard)
  static api::SyncStatus block()
  {
    FenceSync sync{};
    sync.wait();
    return sync.clientWait();
  }

private:
  const api::core::Sync m_sync;
};
} // namespace gl