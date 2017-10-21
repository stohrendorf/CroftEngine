#include "Camera.h"
#include "Node.h"

#include <glm/gtc/matrix_transform.hpp>

// Camera dirty bits
#define CAMERA_DIRTY_PROJ 2
#define CAMERA_DIRTY_VIEW_PROJ 4
#define CAMERA_DIRTY_INV_VIEW 8
#define CAMERA_DIRTY_INV_VIEW_PROJ 16
#define CAMERA_DIRTY_ALL (CAMERA_DIRTY_PROJ | CAMERA_DIRTY_VIEW_PROJ | CAMERA_DIRTY_INV_VIEW | CAMERA_DIRTY_INV_VIEW_PROJ)

namespace gameplay
{
    Camera::Camera(float fieldOfView, float aspectRatio, float nearPlane, float farPlane)
            : _fieldOfView( fieldOfView )
            , _aspectRatio( aspectRatio )
            , _nearPlane( nearPlane )
            , _farPlane( farPlane )
            , _bits( CAMERA_DIRTY_ALL )
    {
    }

    float Camera::getFieldOfView() const
    {
        return _fieldOfView;
    }

    void Camera::setFieldOfView(float fieldOfView)
    {
        _fieldOfView = fieldOfView;
        _bits |= CAMERA_DIRTY_PROJ | CAMERA_DIRTY_VIEW_PROJ | CAMERA_DIRTY_INV_VIEW_PROJ;
    }

    float Camera::getAspectRatio() const
    {
        return _aspectRatio;
    }

    float Camera::getNearPlane() const
    {
        return _nearPlane;
    }

    float Camera::getFarPlane() const
    {
        return _farPlane;
    }

    const glm::mat4& Camera::getViewMatrix() const
    {
        return _view;
    }

    const glm::mat4& Camera::getInverseViewMatrix() const
    {
        if( _bits & CAMERA_DIRTY_INV_VIEW )
        {
            _inverseView = glm::inverse( _view );

            _bits &= ~CAMERA_DIRTY_INV_VIEW;
        }

        return _inverseView;
    }

    const glm::mat4& Camera::getProjectionMatrix() const
    {
        if( _bits & CAMERA_DIRTY_PROJ )
        {
            _projection = glm::perspective( _fieldOfView, _aspectRatio, _nearPlane, _farPlane );
            _bits &= ~CAMERA_DIRTY_PROJ;
        }

        return _projection;
    }

    const glm::mat4& Camera::getViewProjectionMatrix() const
    {
        if( _bits & CAMERA_DIRTY_VIEW_PROJ )
        {
            _viewProjection = getProjectionMatrix() * getViewMatrix();

            _bits &= ~CAMERA_DIRTY_VIEW_PROJ;
        }

        return _viewProjection;
    }

    const glm::mat4& Camera::getInverseViewProjectionMatrix() const
    {
        if( _bits & CAMERA_DIRTY_INV_VIEW_PROJ )
        {
            _inverseViewProjection = glm::inverse( getViewProjectionMatrix() );

            _bits &= ~CAMERA_DIRTY_INV_VIEW_PROJ;
        }

        return _inverseViewProjection;
    }

    void Camera::setViewMatrix(const glm::mat4& m)
    {
        _view = m;
        _bits |= CAMERA_DIRTY_PROJ | CAMERA_DIRTY_VIEW_PROJ | CAMERA_DIRTY_INV_VIEW_PROJ | CAMERA_DIRTY_INV_VIEW;
    }
}
