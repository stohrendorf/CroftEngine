#include "Camera.h"
#include "Node.h"

#include <glm/gtc/matrix_transform.hpp>

namespace render
{
namespace scene
{
Camera::Camera(const float fieldOfView, const float aspectRatio, const float nearPlane, const float farPlane)
    : m_fieldOfView{ fieldOfView }
      , m_aspectRatio{ aspectRatio }
      , m_nearPlane{ nearPlane }
      , m_farPlane{ farPlane }
{
    m_dirty.set_all();
}

float Camera::getFieldOfView() const
{
    return m_fieldOfView;
}

void Camera::setFieldOfView(const float fieldOfView)
{
    m_fieldOfView = fieldOfView;
    m_dirty.set( DirtyFlag::Projection );
    m_dirty.set( DirtyFlag::ViewProjection );
    m_dirty.set( DirtyFlag::InvViewProjection );
}

float Camera::getAspectRatio() const
{
    return m_aspectRatio;
}

void Camera::setAspectRatio(const float aspectRatio)
{
    m_aspectRatio = aspectRatio;
    m_dirty.set( DirtyFlag::Projection );
    m_dirty.set( DirtyFlag::ViewProjection );
    m_dirty.set( DirtyFlag::InvViewProjection );
}

float Camera::getNearPlane() const
{
    return m_nearPlane;
}

float Camera::getFarPlane() const
{
    return m_farPlane;
}

const glm::mat4& Camera::getViewMatrix() const
{
    return m_view;
}

const glm::mat4& Camera::getInverseViewMatrix() const
{
    if( m_dirty.is_set( DirtyFlag::InvView ) )
    {
        m_inverseView = inverse( m_view );
        m_dirty.reset( DirtyFlag::InvView );
    }

    return m_inverseView;
}

const glm::mat4& Camera::getProjectionMatrix() const
{
    if( m_dirty.is_set( DirtyFlag::Projection ) )
    {
        m_projection = glm::perspective( m_fieldOfView, m_aspectRatio, m_nearPlane, m_farPlane );
        m_dirty.reset( DirtyFlag::Projection );
    }

    return m_projection;
}

const glm::mat4& Camera::getViewProjectionMatrix() const
{
    if( m_dirty.is_set( DirtyFlag::ViewProjection ) )
    {
        m_viewProjection = getProjectionMatrix() * getViewMatrix();
        m_dirty.reset( DirtyFlag::ViewProjection );
    }

    return m_viewProjection;
}

const glm::mat4& Camera::getInverseViewProjectionMatrix() const
{
    if( m_dirty.is_set( DirtyFlag::InvViewProjection ) )
    {
        m_inverseViewProjection = inverse( getViewProjectionMatrix() );
        m_dirty.reset( DirtyFlag::InvViewProjection );
    }

    return m_inverseViewProjection;
}

// ReSharper disable once CppMemberFunctionMayBeConst
void Camera::setViewMatrix(const glm::mat4& m)
{
    m_view = m;
    m_dirty.set( DirtyFlag::ViewProjection );
    m_dirty.set( DirtyFlag::InvViewProjection );
    m_dirty.set( DirtyFlag::InvView );
}
}
}
