#include "Base.h"
#include "Transform.h"

#include <glm/gtc/matrix_transform.hpp>

namespace gameplay
{
    int Transform::_suspendTransformChanged(0);
    std::vector<std::shared_ptr<Transform>> Transform::_transformsChanged;


    Transform::Transform()
        : _matrixDirtyBits(0)
        , _listeners()
        , _scale{1, 1, 1}
    {
    }


    Transform::Transform(const glm::vec3& scale, const glm::quat& rotation, const glm::vec3& translation)
        : _matrixDirtyBits(0)
        , _listeners()
    {
        set(scale, rotation, translation);
    }


    Transform::Transform(const glm::vec3& scale, const glm::mat4& rotation, const glm::vec3& translation)
        : _matrixDirtyBits(0)
        , _listeners()
    {
        set(scale, rotation, translation);
    }


    Transform::Transform(const Transform& copy)
        : _matrixDirtyBits(0)
        , _listeners()
    {
        set(copy);
    }


    Transform::~Transform() = default;


    void Transform::suspendTransformChanged()
    {
        _suspendTransformChanged++;
    }


    void Transform::resumeTransformChanged()
    {
        if( _suspendTransformChanged == 0 ) // We haven't suspended transformChanged() calls, so do nothing.
            return;

        if( _suspendTransformChanged == 1 )
        {
            // Call transformChanged() on all transforms in the list
            size_t transformCount = _transformsChanged.size();
            for( size_t i = 0; i < transformCount; i++ )
            {
                auto t = _transformsChanged.at(i);
                BOOST_ASSERT(t);
                t->transformChanged();
            }

            // Go through list and reset DIRTY_NOTIFY bit. The list could potentially be larger here if the
            // transforms we were delaying calls to transformChanged() have any child nodes.
            transformCount = _transformsChanged.size();
            for( size_t i = 0; i < transformCount; i++ )
            {
                auto t = _transformsChanged.at(i);
                BOOST_ASSERT(t);
                t->_matrixDirtyBits &= ~DIRTY_NOTIFY;
            }

            // empty list for next frame.
            _transformsChanged.clear();
        }
        _suspendTransformChanged--;
    }


    bool Transform::isTransformChangedSuspended()
    {
        return (_suspendTransformChanged > 0);
    }


    const glm::mat4& Transform::getMatrix() const
    {
        if( _matrixDirtyBits )
        {
            // Compose the matrix in TRS order since we use column-major matrices with column vectors and
            // multiply M*v (as opposed to XNA and DirectX that use row-major matrices with row vectors and multiply v*M).
            _matrix = glm::translate(glm::mat4(1), _translation);
            _matrix = _matrix * glm::mat4_cast(_rotation);
            _matrix = glm::scale(_matrix, _scale);

            _matrixDirtyBits &= ~DIRTY_TRANSLATION & ~DIRTY_ROTATION & ~DIRTY_SCALE;
        }

        return _matrix;
    }


    const glm::vec3& Transform::getScale() const
    {
        return _scale;
    }


    void Transform::getScale(glm::vec3* scale) const
    {
        BOOST_ASSERT(scale);
        *scale = _scale;
    }


    float Transform::getScaleX() const
    {
        return _scale.x;
    }


    float Transform::getScaleY() const
    {
        return _scale.y;
    }


    float Transform::getScaleZ() const
    {
        return _scale.z;
    }


    const glm::quat& Transform::getRotation() const
    {
        return _rotation;
    }


    void Transform::getRotation(glm::quat* rotation) const
    {
        BOOST_ASSERT(rotation);
        *rotation = _rotation;
    }


    void Transform::getRotation(glm::mat4* rotation) const
    {
        BOOST_ASSERT(rotation);
        *rotation = glm::mat4_cast(_rotation);
    }


    float Transform::getRotation(glm::vec3* axis) const
    {
        BOOST_ASSERT(axis);
        *axis = glm::axis(_rotation);
        return glm::angle(_rotation);
    }


    const glm::vec3& Transform::getTranslation() const
    {
        return _translation;
    }


    void Transform::getTranslation(glm::vec3* translation) const
    {
        BOOST_ASSERT(translation);
        *translation = _translation;
    }


    float Transform::getTranslationX() const
    {
        return _translation.x;
    }


    float Transform::getTranslationY() const
    {
        return _translation.y;
    }


    float Transform::getTranslationZ() const
    {
        return _translation.z;
    }


    void Transform::rotate(float qx, float qy, float qz, float qw)
    {
        glm::quat q(qx, qy, qz, qw);
        _rotation *= q;
        dirty(DIRTY_ROTATION);
    }


    void Transform::rotate(const glm::quat& rotation)
    {
        _rotation *= rotation;
        dirty(DIRTY_ROTATION);
    }


    void Transform::rotate(const glm::vec3& axis, float angle)
    {
        glm::quat rotationQuat{ angle, axis };
        _rotation = glm::normalize(_rotation * rotationQuat);
        dirty(DIRTY_ROTATION);
    }


    void Transform::rotate(const glm::mat4& rotation)
    {
        _rotation *= glm::quat_cast(rotation);
        dirty(DIRTY_ROTATION);
    }


    void Transform::rotateX(float angle)
    {
        glm::quat rotationQuat{ angle, { 1,0,0 } };
        _rotation *= rotationQuat;
        dirty(DIRTY_ROTATION);
    }


    void Transform::rotateY(float angle)
    {
        glm::quat rotationQuat{ angle, { 0,1,0 } };
        _rotation *= rotationQuat;
        dirty(DIRTY_ROTATION);
    }


    void Transform::rotateZ(float angle)
    {
        glm::quat rotationQuat{ angle, { 0,0,1 } };
        _rotation *= rotationQuat;
        dirty(DIRTY_ROTATION);
    }


    void Transform::scale(float scale)
    {
        _scale *= scale;
        dirty(DIRTY_SCALE);
    }


    void Transform::scale(float sx, float sy, float sz)
    {
        _scale.x *= sx;
        _scale.y *= sy;
        _scale.z *= sz;
        dirty(DIRTY_SCALE);
    }


    void Transform::scale(const glm::vec3& scale)
    {
        _scale.x *= scale.x;
        _scale.y *= scale.y;
        _scale.z *= scale.z;
        dirty(DIRTY_SCALE);
    }


    void Transform::scaleX(float sx)
    {
        _scale.x *= sx;
        dirty(DIRTY_SCALE);
    }


    void Transform::scaleY(float sy)
    {
        _scale.y *= sy;
        dirty(DIRTY_SCALE);
    }


    void Transform::scaleZ(float sz)
    {
        _scale.z *= sz;
        dirty(DIRTY_SCALE);
    }


    void Transform::set(const glm::vec3& scale, const glm::quat& rotation, const glm::vec3& translation)
    {
        _scale = scale;
        _rotation = rotation;
        _translation = translation;
        dirty(DIRTY_TRANSLATION | DIRTY_ROTATION | DIRTY_SCALE);
    }


    void Transform::set(const glm::vec3& scale, const glm::mat4& rotation, const glm::vec3& translation)
    {
        _scale = scale;
        glm::quat rotationQuat = glm::quat_cast(rotation);
        _rotation = rotationQuat;
        _translation = translation;
        dirty(DIRTY_TRANSLATION | DIRTY_ROTATION | DIRTY_SCALE);
    }


    void Transform::set(const glm::vec3& scale, const glm::vec3& axis, float angle, const glm::vec3& translation)
    {
        _scale = scale;
        _rotation = glm::quat(angle, axis);
        _translation = translation;
        dirty(DIRTY_TRANSLATION | DIRTY_ROTATION | DIRTY_SCALE);
    }


    void Transform::set(const Transform& transform)
    {
        _scale = transform._scale;
        _rotation = transform._rotation;
        _translation = transform._translation;
        dirty(DIRTY_TRANSLATION | DIRTY_ROTATION | DIRTY_SCALE);
    }


    void Transform::setIdentity()
    {
        _scale = { 1.0f, 1.0f, 1.0f };
        _rotation = glm::quat();
        _translation = { 0, 0, 0 };
        dirty(DIRTY_TRANSLATION | DIRTY_ROTATION | DIRTY_SCALE);
    }


    void Transform::setScale(float scale)
    {
        _scale = { scale, scale, scale };
        dirty(DIRTY_SCALE);
    }


    void Transform::setScale(float sx, float sy, float sz)
    {
        _scale = { sx, sy, sz };
        dirty(DIRTY_SCALE);
    }


    void Transform::setScale(const glm::vec3& scale)
    {
        _scale = scale;
        dirty(DIRTY_SCALE);
    }


    void Transform::setScaleX(float sx)
    {
        _scale.x = sx;
        dirty(DIRTY_SCALE);
    }


    void Transform::setScaleY(float sy)
    {
        _scale.y = sy;
        dirty(DIRTY_SCALE);
    }


    void Transform::setScaleZ(float sz)
    {
        _scale.z = sz;
        dirty(DIRTY_SCALE);
    }


    void Transform::setRotation(const glm::quat& rotation)
    {
        _rotation = rotation;
        dirty(DIRTY_ROTATION);
    }


    void Transform::setRotation(float qx, float qy, float qz, float qw)
    {
        _rotation = { qx, qy, qz, qw };
        dirty(DIRTY_ROTATION);
    }


    void Transform::setRotation(const glm::mat4& rotation)
    {
        glm::quat rotationQuat = glm::quat_cast(rotation);
        _rotation = rotationQuat;
        dirty(DIRTY_ROTATION);
    }


    void Transform::setRotation(const glm::vec3& axis, float angle)
    {
        _rotation = glm::angleAxis(angle, axis);
        dirty(DIRTY_ROTATION);
    }


    void Transform::setTranslation(const glm::vec3& translation)
    {
        _translation = translation;
        dirty(DIRTY_TRANSLATION);
    }


    void Transform::setTranslation(float tx, float ty, float tz)
    {
        _translation = { tx, ty, tz };
        dirty(DIRTY_TRANSLATION);
    }


    void Transform::setTranslationX(float tx)
    {
        _translation.x = tx;
        dirty(DIRTY_TRANSLATION);
    }


    void Transform::setTranslationY(float ty)
    {
        _translation.y = ty;
        dirty(DIRTY_TRANSLATION);
    }


    void Transform::setTranslationZ(float tz)
    {
        _translation.z = tz;
        dirty(DIRTY_TRANSLATION);
    }


    void Transform::translate(float tx, float ty, float tz)
    {
        _translation.x += tx;
        _translation.y += ty;
        _translation.z += tz;
        dirty(DIRTY_TRANSLATION);
    }


    void Transform::translate(const glm::vec3& translation)
    {
        _translation += translation;
        dirty(DIRTY_TRANSLATION);
    }


    void Transform::translateX(float tx)
    {
        _translation.x += tx;
        dirty(DIRTY_TRANSLATION);
    }


    void Transform::translateY(float ty)
    {
        _translation.y += ty;
        dirty(DIRTY_TRANSLATION);
    }


    void Transform::translateZ(float tz)
    {
        _translation.z += tz;
        dirty(DIRTY_TRANSLATION);
    }


    void Transform::translateSmooth(const glm::vec3& target, float elapsedTime, float responseTime)
    {
        if( elapsedTime > 0 )
        {
            _translation += (target - _translation) * (elapsedTime / (elapsedTime + responseTime));
            dirty(DIRTY_TRANSLATION);
        }
    }


    void Transform::transformVector(glm::vec3* normal) const
    {
        getMatrix();
        *normal = glm::vec3(_matrix * glm::vec4(*normal, 1.0f));
    }


    void Transform::transformVector(const glm::vec3& normal, glm::vec3* dst) const
    {
        getMatrix();
        *dst = glm::vec3(_matrix * glm::vec4(normal, 1.0f));
    }


    void Transform::dirty(char matrixDirtyBits)
    {
        _matrixDirtyBits |= matrixDirtyBits;
        if( isTransformChangedSuspended() )
        {
            if( !isDirty(DIRTY_NOTIFY) )
            {
                suspendTransformChange(shared_from_this());
            }
        }
        else
        {
            transformChanged();
        }
    }


    bool Transform::isDirty(char matrixDirtyBits) const
    {
        return (_matrixDirtyBits & matrixDirtyBits) == matrixDirtyBits;
    }


    void Transform::suspendTransformChange(const std::shared_ptr<Transform>& transform)
    {
        BOOST_ASSERT(transform);
        transform->_matrixDirtyBits |= DIRTY_NOTIFY;
        _transformsChanged.push_back(transform);
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
