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
    Projection,
    InvView,
    ViewProjection,
    InvViewProjection,
    _flag_set_size
  };

  glm::mat4 projection{1.0f};
  glm::mat4 inverseProjection{1.0f};
  glm::mat4 view{1.0f};
  glm::mat4 inverseView{1.0f};
  glm::mat4 viewProjection{1.0f};
  glm::mat4 inverseViewProjection{1.0f};
  float aspectRatio = 1;
  float near = 0;
  float far = 1;
};

class Camera final
{
  friend class Node;

public:
  Camera(float fieldOfView, float aspectRatio, float nearPlane, float farPlane)
      : m_fieldOfView{fieldOfView}
  {
    m_dirty.set_all();
    m_matrices.aspectRatio = aspectRatio;
    m_matrices.near = nearPlane;
    m_matrices.far = farPlane;
  }

  Camera(const Camera&) = delete;

  Camera(Camera&&) = delete;

  Camera& operator=(Camera&&) = delete;

  Camera& operator=(const Camera&) = delete;

  ~Camera() = default;

  void setFieldOfView(float fieldOfView)
  {
    m_fieldOfView = fieldOfView;
    m_dirty.set(CameraMatrices::DirtyFlag::Projection);
    m_dirty.set(CameraMatrices::DirtyFlag::ViewProjection);
    m_dirty.set(CameraMatrices::DirtyFlag::InvViewProjection);
  }

  [[nodiscard]] float getAspectRatio() const
  {
    return m_matrices.aspectRatio;
  }

  void setAspectRatio(float aspectRatio)
  {
    m_matrices.aspectRatio = aspectRatio;
    m_dirty.set(CameraMatrices::DirtyFlag::Projection);
    m_dirty.set(CameraMatrices::DirtyFlag::ViewProjection);
    m_dirty.set(CameraMatrices::DirtyFlag::InvViewProjection);
  }

  [[nodiscard]] float getNearPlane() const
  {
    return m_matrices.near;
  }

  [[nodiscard]] float getFarPlane() const
  {
    return m_matrices.far;
  }

  [[nodiscard]] const glm::mat4& getViewMatrix() const
  {
    return m_matrices.view;
  }

  // ReSharper disable once CppMemberFunctionMayBeConst
  void setViewMatrix(const glm::mat4& m)
  {
    m_matrices.view = m;
    m_dirty.set(CameraMatrices::DirtyFlag::ViewProjection);
    m_dirty.set(CameraMatrices::DirtyFlag::InvViewProjection);
    m_dirty.set(CameraMatrices::DirtyFlag::InvView);
  }

  [[nodiscard]] const glm::mat4& getInverseViewMatrix() const
  {
    if(m_dirty.is_set(CameraMatrices::DirtyFlag::InvView))
    {
      m_matrices.inverseView = inverse(m_matrices.view);
      m_dirty.reset(CameraMatrices::DirtyFlag::InvView);
    }

    return m_matrices.inverseView;
  }

  [[nodiscard]] const glm::mat4& getProjectionMatrix() const
  {
    if(m_dirty.is_set(CameraMatrices::DirtyFlag::Projection))
    {
      m_matrices.projection = glm::perspective(m_fieldOfView, m_matrices.aspectRatio, m_matrices.near, m_matrices.far);
      m_matrices.inverseProjection = glm::inverse(m_matrices.projection);
      m_dirty.reset(CameraMatrices::DirtyFlag::Projection);
    }

    return m_matrices.projection;
  }

  [[nodiscard]] const glm::mat4& getViewProjectionMatrix() const
  {
    if(m_dirty.is_set(CameraMatrices::DirtyFlag::ViewProjection))
    {
      m_matrices.viewProjection = getProjectionMatrix() * getViewMatrix();
      m_dirty.reset(CameraMatrices::DirtyFlag::ViewProjection);
    }

    return m_matrices.viewProjection;
  }

  [[nodiscard]] const glm::mat4& getInverseViewProjectionMatrix() const
  {
    if(m_dirty.is_set(CameraMatrices::DirtyFlag::InvViewProjection))
    {
      m_matrices.inverseViewProjection = inverse(getViewProjectionMatrix());
      m_dirty.reset(CameraMatrices::DirtyFlag::InvViewProjection);
    }

    return m_matrices.inverseViewProjection;
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

  [[nodiscard]] auto getFieldOfViewY() const
  {
    return m_fieldOfView;
  }

  [[nodiscard]] const auto& getMatricesBuffer() const
  {
    if(m_dirty.any())
    {
      if(m_dirty.is_set(CameraMatrices::DirtyFlag::InvView))
      {
        m_matrices.inverseView = inverse(m_matrices.view);
        m_dirty.reset(CameraMatrices::DirtyFlag::InvView);
      }

      if(m_dirty.is_set(CameraMatrices::DirtyFlag::Projection))
      {
        m_matrices.projection
          = glm::perspective(m_fieldOfView, m_matrices.aspectRatio, m_matrices.near, m_matrices.far);
        m_dirty.reset(CameraMatrices::DirtyFlag::Projection);
      }

      if(m_dirty.is_set(CameraMatrices::DirtyFlag::ViewProjection))
      {
        m_matrices.viewProjection = m_matrices.projection * m_matrices.view;
        m_dirty.reset(CameraMatrices::DirtyFlag::ViewProjection);
      }

      if(m_dirty.is_set(CameraMatrices::DirtyFlag::InvViewProjection))
      {
        m_matrices.inverseViewProjection = inverse(m_matrices.viewProjection);
        m_dirty.reset(CameraMatrices::DirtyFlag::InvViewProjection);
      }

      m_matricesBuffer.setData(m_matrices, gl::api::BufferUsageARB::StreamDraw);
    }

    BOOST_ASSERT(m_dirty.none());
    return m_matricesBuffer;
  }

private:
  float m_fieldOfView;

  mutable type_safe::flag_set<CameraMatrices::DirtyFlag> m_dirty;
  mutable CameraMatrices m_matrices{};
  mutable gl::UniformBuffer<CameraMatrices> m_matricesBuffer{"camera-matrices-ubo"};
};
} // namespace render::scene
