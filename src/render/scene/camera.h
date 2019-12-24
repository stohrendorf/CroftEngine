#pragma once

#include "type_safe/flag_set.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace render::scene
{
class Node;

class Camera final
{
  friend class Node;

public:
  Camera(float fieldOfView, float aspectRatio, float nearPlane, float farPlane)
      : m_fieldOfView{fieldOfView}
      , m_aspectRatio{aspectRatio}
      , m_nearPlane{nearPlane}
      , m_farPlane{farPlane}
  {
    m_dirty.set_all();
  }

  Camera(const Camera&) = delete;

  Camera(Camera&&) = delete;

  Camera& operator=(Camera&&) = delete;

  Camera& operator=(const Camera&) = delete;

  ~Camera() = default;

  void setFieldOfView(float fieldOfView)
  {
    m_fieldOfView = fieldOfView;
    m_dirty.set(DirtyFlag::Projection);
    m_dirty.set(DirtyFlag::ViewProjection);
    m_dirty.set(DirtyFlag::InvViewProjection);
  }

  [[nodiscard]] float getAspectRatio() const
  {
    return m_aspectRatio;
  }

  void setAspectRatio(float aspectRatio)
  {
    m_aspectRatio = aspectRatio;
    m_dirty.set(DirtyFlag::Projection);
    m_dirty.set(DirtyFlag::ViewProjection);
    m_dirty.set(DirtyFlag::InvViewProjection);
  }

  [[nodiscard]] float getNearPlane() const
  {
    return m_nearPlane;
  }

  [[nodiscard]] float getFarPlane() const
  {
    return m_farPlane;
  }

  [[nodiscard]] const glm::mat4& getViewMatrix() const
  {
    return m_view;
  }

  // ReSharper disable once CppMemberFunctionMayBeConst
  void setViewMatrix(const glm::mat4& m)
  {
    m_view = m;
    m_dirty.set(DirtyFlag::ViewProjection);
    m_dirty.set(DirtyFlag::InvViewProjection);
    m_dirty.set(DirtyFlag::InvView);
  }

  [[nodiscard]] const glm::mat4& getInverseViewMatrix() const
  {
    if(m_dirty.is_set(DirtyFlag::InvView))
    {
      m_inverseView = inverse(m_view);
      m_dirty.reset(DirtyFlag::InvView);
    }

    return m_inverseView;
  }

  [[nodiscard]] const glm::mat4& getProjectionMatrix() const
  {
    if(m_dirty.is_set(DirtyFlag::Projection))
    {
      m_projection = glm::perspective(m_fieldOfView, m_aspectRatio, m_nearPlane, m_farPlane);
      m_dirty.reset(DirtyFlag::Projection);
    }

    return m_projection;
  }

  [[nodiscard]] const glm::mat4& getViewProjectionMatrix() const
  {
    if(m_dirty.is_set(DirtyFlag::ViewProjection))
    {
      m_viewProjection = getProjectionMatrix() * getViewMatrix();
      m_dirty.reset(DirtyFlag::ViewProjection);
    }

    return m_viewProjection;
  }

  [[nodiscard]] const glm::mat4& getInverseViewProjectionMatrix() const
  {
    if(m_dirty.is_set(DirtyFlag::InvViewProjection))
    {
      m_inverseViewProjection = inverse(getViewProjectionMatrix());
      m_dirty.reset(DirtyFlag::InvViewProjection);
    }

    return m_inverseViewProjection;
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

  [[nodiscard]] auto getFieldOfViewX() const
  {
    return m_fieldOfView * m_aspectRatio;
  }

  [[nodiscard]] auto getFieldOfViewY() const
  {
    return m_fieldOfView;
  }

private:
  enum class DirtyFlag
  {
    Projection,
    InvView,
    ViewProjection,
    InvViewProjection,
    _flag_set_size
  };

  float m_fieldOfView;
  float m_aspectRatio;
  float m_nearPlane;
  float m_farPlane;
  mutable glm::mat4 m_view{1.0f};
  mutable glm::mat4 m_projection{1.0f};
  mutable glm::mat4 m_viewProjection{1.0f};
  mutable glm::mat4 m_inverseView{1.0f};
  mutable glm::mat4 m_inverseViewProjection{1.0f};
  mutable type_safe::flag_set<DirtyFlag> m_dirty;
};
} // namespace render::scene
