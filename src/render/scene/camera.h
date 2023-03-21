#pragma once

#include <gl/buffer.h>
#include <glm/gtc/matrix_transform.hpp>
#include <type_safe/flag_set.hpp>

namespace render::scene
{
class Node;

struct CameraMatrices
{
  enum class DirtyFlag
  {
    BufferData,
    Projection,
    ViewProjection,
    _flag_set_size [[maybe_unused]]
  };

  glm::mat4 projection{1.0f};
  glm::mat4 view{1.0f};
  glm::mat4 viewProjection{1.0f};
  glm::vec4 viewport{0};
  float aspectRatio = 1;
  float nearPlane = 0;
  float farPlane = 1;
  float _pad = 0;
};
static_assert(sizeof(CameraMatrices) % 16 == 0);

class Camera final
{
  friend class Node;

public:
  Camera(float fieldOfView, const glm::vec2& viewport, float nearPlane, float farPlane)
      : m_fieldOfView{fieldOfView}
      , m_matricesBuffer{"camera-matrices-ubo", gl::api::BufferUsage::DynamicDraw, m_matrices}
  {
    m_dirty.set_all();
    m_matrices.aspectRatio = viewport.x / viewport.y;
    m_matrices.viewport = glm::vec4{viewport, 0, 0};
    m_matrices.nearPlane = nearPlane;
    m_matrices.farPlane = farPlane;

    m_matricesBuffer.setSubData(m_matrices, 0);
  }

  Camera(const Camera&) = delete;
  Camera(Camera&&) = delete;
  Camera& operator=(Camera&&) = delete;
  Camera& operator=(const Camera&) = delete;

  ~Camera() = default;

  void setFieldOfView(float fieldOfView) noexcept
  {
    m_fieldOfView = fieldOfView;
    m_dirty.set(CameraMatrices::DirtyFlag::Projection);
    m_dirty.set(CameraMatrices::DirtyFlag::ViewProjection);
    m_dirty.set(CameraMatrices::DirtyFlag::BufferData);
  }

  [[nodiscard]] float getAspectRatio() const noexcept
  {
    return m_matrices.aspectRatio;
  }

  void setViewport(const glm::vec2& viewport)
  {
    if(glm::vec2{m_matrices.viewport} == viewport)
      return;

    m_matrices.aspectRatio = viewport.x / viewport.y;
    m_matrices.viewport = glm::vec4{viewport, 0, 0};
    m_dirty.set(CameraMatrices::DirtyFlag::Projection);
    m_dirty.set(CameraMatrices::DirtyFlag::ViewProjection);
    m_dirty.set(CameraMatrices::DirtyFlag::BufferData);
  }

  [[nodiscard]] float getNearPlane() const noexcept
  {
    return m_matrices.nearPlane;
  }

  [[nodiscard]] float getFarPlane() const noexcept
  {
    return m_matrices.farPlane;
  }

  [[nodiscard]] const glm::mat4& getViewMatrix() const noexcept
  {
    return m_matrices.view;
  }

  void setViewMatrix(const glm::mat4& m) noexcept
  {
    m_matrices.view = m;
    m_dirty.set(CameraMatrices::DirtyFlag::ViewProjection);
    m_dirty.set(CameraMatrices::DirtyFlag::BufferData);
  }

  [[nodiscard]] auto getInverseViewMatrix() const
  {
    return glm::inverse(m_matrices.view);
  }

  [[nodiscard]] const glm::mat4& getProjectionMatrix() const
  {
    if(m_dirty.is_set(CameraMatrices::DirtyFlag::Projection))
    {
      m_matrices.projection
        = glm::perspective(m_fieldOfView, m_matrices.aspectRatio, m_matrices.nearPlane, m_matrices.farPlane);
      m_dirty.reset(CameraMatrices::DirtyFlag::Projection);
      m_dirty.set(CameraMatrices::DirtyFlag::BufferData);
    }

    return m_matrices.projection;
  }

  [[nodiscard]] const glm::mat4& getViewProjectionMatrix() const
  {
    if(m_dirty.is_set(CameraMatrices::DirtyFlag::ViewProjection))
    {
      m_matrices.viewProjection = getProjectionMatrix() * getViewMatrix();
      m_dirty.reset(CameraMatrices::DirtyFlag::ViewProjection);
      m_dirty.set(CameraMatrices::DirtyFlag::BufferData);
    }

    return m_matrices.viewProjection;
  }

  [[nodiscard]] glm::vec3 getPosition() const
  {
    return glm::vec3{getInverseViewMatrix()[3]};
  }

  [[nodiscard]] glm::vec3 getRotatedVector(const glm::vec3& v) const
  {
    auto rs = getInverseViewMatrix();
    rs[3].x = rs[3].y = rs[3].z = 0; // zero out translation component
    return glm::vec3{rs * glm::vec4{v, 1}};
  }

  [[nodiscard]] glm::vec3 getFrontVector() const
  {
    return getRotatedVector(glm::vec3{0, 0, -1});
  }

  [[nodiscard]] glm::vec3 getUpVector() const
  {
    return getRotatedVector(glm::vec3{0, 1, 0});
  }

  [[nodiscard]] glm::vec3 getRightVector() const
  {
    return getRotatedVector(glm::vec3{1, 0, 0});
  }

  [[nodiscard]] auto getFieldOfViewY() const noexcept
  {
    return m_fieldOfView;
  }

  [[nodiscard]] const gl::UniformBuffer<CameraMatrices>& getMatricesBuffer() const
  {
    if(m_dirty.any())
    {
      (void)getProjectionMatrix();
      (void)getViewProjectionMatrix();
    }
    if(m_dirty.is_set(CameraMatrices::DirtyFlag::BufferData))
    {
      m_matricesBuffer.setSubData(m_matrices, 0);
      m_dirty.reset(CameraMatrices::DirtyFlag::BufferData);
    }

    BOOST_ASSERT(m_dirty.none());
    return m_matricesBuffer;
  }

private:
  float m_fieldOfView;

  mutable type_safe::flag_set<CameraMatrices::DirtyFlag> m_dirty;
  mutable CameraMatrices m_matrices{};
  mutable gl::UniformBuffer<CameraMatrices> m_matricesBuffer;
};
} // namespace render::scene
