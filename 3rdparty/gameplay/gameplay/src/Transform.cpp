#include "Base.h"
#include "Transform.h"
#include "Node.h"


namespace gameplay
{
    int Transform::_suspendTransformChanged(0);
    std::vector<std::shared_ptr<Transform>> Transform::_transformsChanged;


    Transform::Transform()
        : _matrixDirtyBits(0)
        , _listeners()
    {
        _scale.set(Vector3::one());
    }


    Transform::Transform(const Vector3& scale, const Quaternion& rotation, const Vector3& translation)
        : _matrixDirtyBits(0)
        , _listeners()
    {
        set(scale, rotation, translation);
    }


    Transform::Transform(const Vector3& scale, const Matrix& rotation, const Vector3& translation)
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


    const Matrix& Transform::getMatrix() const
    {
        if( _matrixDirtyBits )
        {
            _matrix.setIdentity();

            // Compose the matrix in TRS order since we use column-major matrices with column vectors and
            // multiply M*v (as opposed to XNA and DirectX that use row-major matrices with row vectors and multiply v*M).
            _matrix.translate(_translation);
            _matrix.rotate(_rotation);
            _matrix.scale(_scale);

            _matrixDirtyBits &= ~DIRTY_TRANSLATION & ~DIRTY_ROTATION & ~DIRTY_SCALE;
        }

        return _matrix;
    }


    const Vector3& Transform::getScale() const
    {
        return _scale;
    }


    void Transform::getScale(Vector3* scale) const
    {
        BOOST_ASSERT(scale);
        scale->set(_scale);
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


    const Quaternion& Transform::getRotation() const
    {
        return _rotation;
    }


    void Transform::getRotation(Quaternion* rotation) const
    {
        BOOST_ASSERT(rotation);
        rotation->set(_rotation);
    }


    void Transform::getRotation(Matrix* rotation) const
    {
        BOOST_ASSERT(rotation);
        Matrix::createRotation(_rotation, rotation);
    }


    float Transform::getRotation(Vector3* axis) const
    {
        BOOST_ASSERT(axis);
        return _rotation.toAxisAngle(axis);
    }


    const Vector3& Transform::getTranslation() const
    {
        return _translation;
    }


    void Transform::getTranslation(Vector3* translation) const
    {
        BOOST_ASSERT(translation);
        translation->set(_translation);
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


    Vector3 Transform::getForwardVector() const
    {
        Vector3 v;
        getForwardVector(&v);
        return v;
    }


    void Transform::getForwardVector(Vector3* dst) const
    {
        getMatrix().getForwardVector(dst);
    }


    Vector3 Transform::getBackVector() const
    {
        Vector3 v;
        getBackVector(&v);
        return v;
    }


    void Transform::getBackVector(Vector3* dst) const
    {
        getMatrix().getBackVector(dst);
    }


    Vector3 Transform::getUpVector() const
    {
        Vector3 v;
        getUpVector(&v);
        return v;
    }


    void Transform::getUpVector(Vector3* dst) const
    {
        getMatrix().getUpVector(dst);
    }


    Vector3 Transform::getDownVector() const
    {
        Vector3 v;
        getDownVector(&v);
        return v;
    }


    void Transform::getDownVector(Vector3* dst) const
    {
        getMatrix().getDownVector(dst);
    }


    Vector3 Transform::getLeftVector() const
    {
        Vector3 v;
        getLeftVector(&v);
        return v;
    }


    void Transform::getLeftVector(Vector3* dst) const
    {
        getMatrix().getLeftVector(dst);
    }


    Vector3 Transform::getRightVector() const
    {
        Vector3 v;
        getRightVector(&v);
        return v;
    }


    void Transform::getRightVector(Vector3* dst) const
    {
        getMatrix().getRightVector(dst);
    }


    void Transform::rotate(float qx, float qy, float qz, float qw)
    {
        Quaternion q(qx, qy, qz, qw);
        _rotation.multiply(q);
        dirty(DIRTY_ROTATION);
    }


    void Transform::rotate(const Quaternion& rotation)
    {
        _rotation.multiply(rotation);
        dirty(DIRTY_ROTATION);
    }


    void Transform::rotate(const Vector3& axis, float angle)
    {
        Quaternion rotationQuat;
        Quaternion::createFromAxisAngle(axis, angle, &rotationQuat);
        _rotation.multiply(rotationQuat);
        _rotation.normalize();
        dirty(DIRTY_ROTATION);
    }


    void Transform::rotate(const Matrix& rotation)
    {
        Quaternion rotationQuat;
        Quaternion::createFromRotationMatrix(rotation, &rotationQuat);
        _rotation.multiply(rotationQuat);
        dirty(DIRTY_ROTATION);
    }


    void Transform::rotateX(float angle)
    {
        Quaternion rotationQuat;
        Quaternion::createFromAxisAngle(Vector3::unitX(), angle, &rotationQuat);
        _rotation.multiply(rotationQuat);
        dirty(DIRTY_ROTATION);
    }


    void Transform::rotateY(float angle)
    {
        Quaternion rotationQuat;
        Quaternion::createFromAxisAngle(Vector3::unitY(), angle, &rotationQuat);
        _rotation.multiply(rotationQuat);
        dirty(DIRTY_ROTATION);
    }


    void Transform::rotateZ(float angle)
    {
        Quaternion rotationQuat;
        Quaternion::createFromAxisAngle(Vector3::unitZ(), angle, &rotationQuat);
        _rotation.multiply(rotationQuat);
        dirty(DIRTY_ROTATION);
    }


    void Transform::scale(float scale)
    {
        _scale.scale(scale);
        dirty(DIRTY_SCALE);
    }


    void Transform::scale(float sx, float sy, float sz)
    {
        _scale.x *= sx;
        _scale.y *= sy;
        _scale.z *= sz;
        dirty(DIRTY_SCALE);
    }


    void Transform::scale(const Vector3& scale)
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


    void Transform::set(const Vector3& scale, const Quaternion& rotation, const Vector3& translation)
    {
        _scale.set(scale);
        _rotation.set(rotation);
        _translation.set(translation);
        dirty(DIRTY_TRANSLATION | DIRTY_ROTATION | DIRTY_SCALE);
    }


    void Transform::set(const Vector3& scale, const Matrix& rotation, const Vector3& translation)
    {
        _scale.set(scale);
        Quaternion rotationQuat;
        Quaternion::createFromRotationMatrix(rotation, &rotationQuat);
        _rotation.set(rotationQuat);
        _translation.set(translation);
        dirty(DIRTY_TRANSLATION | DIRTY_ROTATION | DIRTY_SCALE);
    }


    void Transform::set(const Vector3& scale, const Vector3& axis, float angle, const Vector3& translation)
    {
        _scale.set(scale);
        _rotation.set(axis, angle);
        _translation.set(translation);
        dirty(DIRTY_TRANSLATION | DIRTY_ROTATION | DIRTY_SCALE);
    }


    void Transform::set(const Transform& transform)
    {
        _scale.set(transform._scale);
        _rotation.set(transform._rotation);
        _translation.set(transform._translation);
        dirty(DIRTY_TRANSLATION | DIRTY_ROTATION | DIRTY_SCALE);
    }


    void Transform::setIdentity()
    {
        _scale.set(1.0f, 1.0f, 1.0f);
        _rotation.setIdentity();
        _translation.set(0.0f, 0.0f, 0.0f);
        dirty(DIRTY_TRANSLATION | DIRTY_ROTATION | DIRTY_SCALE);
    }


    void Transform::setScale(float scale)
    {
        _scale.set(scale, scale, scale);
        dirty(DIRTY_SCALE);
    }


    void Transform::setScale(float sx, float sy, float sz)
    {
        _scale.set(sx, sy, sz);
        dirty(DIRTY_SCALE);
    }


    void Transform::setScale(const Vector3& scale)
    {
        _scale.set(scale);
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


    void Transform::setRotation(const Quaternion& rotation)
    {
        _rotation.set(rotation);
        dirty(DIRTY_ROTATION);
    }


    void Transform::setRotation(float qx, float qy, float qz, float qw)
    {
        _rotation.set(qx, qy, qz, qw);
        dirty(DIRTY_ROTATION);
    }


    void Transform::setRotation(const Matrix& rotation)
    {
        Quaternion rotationQuat;
        Quaternion::createFromRotationMatrix(rotation, &rotationQuat);
        _rotation.set(rotationQuat);
        dirty(DIRTY_ROTATION);
    }


    void Transform::setRotation(const Vector3& axis, float angle)
    {
        _rotation.set(axis, angle);
        dirty(DIRTY_ROTATION);
    }


    void Transform::setTranslation(const Vector3& translation)
    {
        _translation.set(translation);
        dirty(DIRTY_TRANSLATION);
    }


    void Transform::setTranslation(float tx, float ty, float tz)
    {
        _translation.set(tx, ty, tz);
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


    void Transform::translate(const Vector3& translation)
    {
        _translation.x += translation.x;
        _translation.y += translation.y;
        _translation.z += translation.z;
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


    void Transform::translateLeft(float amount)
    {
        // Force the current transform matrix to be updated.
        getMatrix();

        Vector3 left;
        _matrix.getLeftVector(&left);
        left.normalize();
        left.scale(amount);

        translate(left);
    }


    void Transform::translateUp(float amount)
    {
        // Force the current transform matrix to be updated.
        getMatrix();

        Vector3 up;
        _matrix.getUpVector(&up);
        up.normalize();
        up.scale(amount);

        translate(up);
    }


    void Transform::translateForward(float amount)
    {
        // Force the current transform matrix to be updated.
        getMatrix();

        Vector3 forward;
        _matrix.getForwardVector(&forward);
        forward.normalize();
        forward.scale(amount);

        translate(forward);
    }


    void Transform::translateSmooth(const Vector3& target, float elapsedTime, float responseTime)
    {
        if( elapsedTime > 0 )
        {
            _translation += (target - _translation) * (elapsedTime / (elapsedTime + responseTime));
            dirty(DIRTY_TRANSLATION);
        }
    }


    void Transform::transformPoint(Vector3* point) const
    {
        getMatrix();
        _matrix.transformPoint(point);
    }


    void Transform::transformPoint(const Vector3& point, Vector3* dst) const
    {
        getMatrix();
        _matrix.transformPoint(point, dst);
    }


    void Transform::transformVector(Vector3* normal) const
    {
        getMatrix();
        _matrix.transformVector(normal);
    }


    void Transform::transformVector(const Vector3& normal, Vector3* dst) const
    {
        getMatrix();
        _matrix.transformVector(normal, dst);
    }


    void Transform::transformVector(float x, float y, float z, float w, Vector3* dst) const
    {
        getMatrix();
        _matrix.transformVector(x, y, z, w, dst);
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
