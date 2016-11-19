#include "Base.h"
#include "Transform.h"

#include <glm/gtc/matrix_transform.hpp>

namespace gameplay
{
    Transform::Transform() = default;


    Transform::Transform(const glm::quat& rotation, const glm::vec3& translation)
    {
        set(rotation, translation);
    }


    Transform::Transform(const glm::mat4& rotation, const glm::vec3& translation)
    {
        set(rotation, translation);
    }


    Transform::Transform(const Transform& copy)
    {
        set(copy);
    }


    Transform::~Transform() = default;


    const glm::mat4& Transform::getMatrix() const
    {
        if( _matrixDirty )
        {
            // Compose the matrix in TRS order since we use column-major matrices with column vectors and
            // multiply M*v (as opposed to XNA and DirectX that use row-major matrices with row vectors and multiply v*M).
            _matrix = glm::translate(glm::mat4(1), _translation) * glm::mat4_cast(_rotation);

            _matrixDirty = false;
        }

        return _matrix;
    }

    const glm::quat& Transform::getRotation() const
    {
        return _rotation;
    }


    const glm::vec3& Transform::getTranslation() const
    {
        return _translation;
    }


    void Transform::rotate(float qx, float qy, float qz, float qw)
    {
        glm::quat q(qx, qy, qz, qw);
        _rotation *= q;
        dirty();
    }


    void Transform::rotate(const glm::quat& rotation)
    {
        _rotation *= rotation;
        dirty();
    }


    void Transform::rotate(const glm::vec3& axis, float angle)
    {
        glm::quat rotationQuat{ angle, axis };
        _rotation = glm::normalize(_rotation * rotationQuat);
        dirty();
    }


    void Transform::rotateX(float angle)
    {
        glm::quat rotationQuat{ angle, { 1,0,0 } };
        _rotation *= rotationQuat;
        dirty();
    }


    void Transform::rotateY(float angle)
    {
        glm::quat rotationQuat{ angle, { 0,1,0 } };
        _rotation *= rotationQuat;
        dirty();
    }


    void Transform::rotateZ(float angle)
    {
        glm::quat rotationQuat{ angle, { 0,0,1 } };
        _rotation *= rotationQuat;
        dirty();
    }


    void Transform::set(const glm::quat& rotation, const glm::vec3& translation)
    {
        _rotation = rotation;
        _translation = translation;
        dirty();
    }


    void Transform::set(const glm::mat4& rotation, const glm::vec3& translation)
    {
        glm::quat rotationQuat = glm::quat_cast(rotation);
        _rotation = rotationQuat;
        _translation = translation;
        dirty();
    }


    void Transform::set(const glm::vec3& axis, float angle, const glm::vec3& translation)
    {
        _rotation = glm::angleAxis(angle, axis);
        _translation = translation;
        dirty();
    }


    void Transform::set(const Transform& transform)
    {
        _rotation = transform._rotation;
        _translation = transform._translation;
        dirty();
    }


    void Transform::setIdentity()
    {
        _rotation = glm::quat();
        _translation = { 0, 0, 0 };
        dirty();
    }


    void Transform::setRotation(const glm::quat& rotation)
    {
        _rotation = rotation;
        dirty();
    }


    void Transform::setRotation(float qx, float qy, float qz, float qw)
    {
        _rotation = { qx, qy, qz, qw };
        dirty();
    }


    void Transform::setRotation(const glm::vec3& axis, float angle)
    {
        _rotation = glm::angleAxis(angle, axis);
        dirty();
    }


    void Transform::setTranslation(const glm::vec3& translation)
    {
        _translation = translation;
        dirty();
    }


    void Transform::setTranslation(float tx, float ty, float tz)
    {
        _translation = { tx, ty, tz };
        dirty();
    }


    void Transform::setTranslationX(float tx)
    {
        _translation.x = tx;
        dirty();
    }


    void Transform::setTranslationY(float ty)
    {
        _translation.y = ty;
        dirty();
    }


    void Transform::setTranslationZ(float tz)
    {
        _translation.z = tz;
        dirty();
    }


    void Transform::translate(float tx, float ty, float tz)
    {
        _translation.x += tx;
        _translation.y += ty;
        _translation.z += tz;
        dirty();
    }


    void Transform::translate(const glm::vec3& translation)
    {
        _translation += translation;
        dirty();
    }


    void Transform::translateX(float tx)
    {
        _translation.x += tx;
        dirty();
    }


    void Transform::translateY(float ty)
    {
        _translation.y += ty;
        dirty();
    }


    void Transform::translateZ(float tz)
    {
        _translation.z += tz;
        dirty();
    }


    void Transform::dirty()
    {
        _matrixDirty = true;
        transformChanged();
    }


    bool Transform::isDirty() const
    {
        return _matrixDirty;
    }


    void Transform::addListener(Transform::Listener* listener, long cookie)
    {
        BOOST_ASSERT(listener);

        TransformListener l;
        l.listener = listener;
        l.cookie = cookie;
        _listeners.push_back(l);
    }


    void Transform::removeListener(Transform::Listener* listener)
    {
        BOOST_ASSERT(listener);

        for( auto itr = _listeners.begin(); itr != _listeners.end(); ++itr )
        {
            if( (*itr).listener == listener )
            {
                _listeners.erase(itr);
                break;
            }
        }
    }


    void Transform::transformChanged()
    {
        for( auto itr = _listeners.begin(); itr != _listeners.end(); ++itr )
        {
            TransformListener& l = *itr;
            BOOST_ASSERT(l.listener);
            l.listener->transformChanged(this, l.cookie);
        }
    }
}
