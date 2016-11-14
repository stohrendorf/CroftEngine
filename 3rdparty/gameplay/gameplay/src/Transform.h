#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>


namespace gameplay
{
    class BoundingBox;


    /**
     * Defines a 3-dimensional transformation.
     *
     * When using the scale, rotate, and translate methods, only the
     * transform's corresponding scale, rotation, or translation
     * component is updated (it is not as if the scale, rotate, or translate
     * is applied to the transform's matrix).
     *
     * Note: To construct a Transform from a transformation matrix stored as a glm::mat4,
     * first decompose the glm::mat4 into its separate translation, scale, and rotation
     * components using matrix.decompose(glm::vec3, glm::quat, glm::vec3) and then pass
     * those arguments to the appropriate constructor or set methods of Transform.
     */
    class Transform : public std::enable_shared_from_this<Transform>
    {
    public:

        /**
         * Globally suspends all transform changed events.
         */
        static void suspendTransformChanged();

        /**
         * Globally resumes all transform changed events.
         */
        static void resumeTransformChanged();

        /**
         * Gets whether all transform changed events are suspended.
         *
         * @return TRUE if transform changed events are suspended; FALSE if transform changed events are not suspended.
         */
        static bool isTransformChangedSuspended();


        /**
         * Listener interface for Transform events.
         */
        class Listener
        {
        public:

            virtual ~Listener() = default;


            /**
             * Handles when an transform has changed.
             *
             * @param transform The Transform object that was changed.
             * @param cookie Cookie value that was specified when the listener was registered.
             */
            virtual void transformChanged(Transform* transform, long cookie) = 0;
        };


        /**
         * Constructs the identity transform.
         */
        Transform();

        /**
         * Constructs a new transform from the specified values.
         *
         * @param scale The scale vector.
         * @param rotation The rotation quaternion.
         * @param translation The translation vector.
         */
        Transform(const glm::vec3& scale, const glm::quat& rotation, const glm::vec3& translation);

        /**
         * Constructs a new transform from the specified values.
         *
         * @param scale The scale vector.
         * @param rotation The rotation matrix.
         * @param translation The translation vector.
         */
        Transform(const glm::vec3& scale, const glm::mat4& rotation, const glm::vec3& translation);

        /**
         * Constructs a new transform from the given transform.
         *
         * @param copy The transform to copy.
         */
        Transform(const Transform& copy);

        /**
         * Destructor.
         */
        virtual ~Transform();

        /**
         * Gets the matrix corresponding to this transform.
         *
         * The matrix returned from this method is mathematically equivalent
         * to this transform only as long as this transform is not changed
         * (i.e. by calling set(), setScale(), translate(), rotateX(), etc.).
         * Once the transform has been changed, the user must call getMatrix()
         * again to get the updated matrix. Also note that changing the matrix
         * returned from this method does not change this transform.
         *
         * @return The matrix of this transform.
         */
        const glm::mat4& getMatrix() const;

        /**
         * Returns the scale for this transform.
         */
        const glm::vec3& getScale() const;

        /**
         * Gets the scale component of this transform in the specified vector.
         *
         * @param scale The vector to store the scale in.
         */
        void getScale(glm::vec3* scale) const;

        /**
         * Gets the scale factor along the x-axis of this transform.
         *
         * @return The scale factor along the x-axis.
         */
        float getScaleX() const;

        /**
         * Gets the scale factor along the y-axis of this transform.
         *
         * @return The scale factor along the y-axis.
         */
        float getScaleY() const;

        /**
         * Gets the scale factor along the z-axis of this transform.
         *
         * @return The scale factor along the z-axis.
         */
        float getScaleZ() const;

        /**
         * Returns the rotation for this transform.
         */
        const glm::quat& getRotation() const;

        /**
         * Gets the rotation component of this transform in the specified quaternion.
         *
         * @param rotation The quaternion to store the rotation in.
         */
        void getRotation(glm::quat* rotation) const;

        /**
         * Gets the rotation component of this transform in the specified matrix.
         *
         * @param rotation The matrix to store the rotation in.
         */
        void getRotation(glm::mat4* rotation) const;

        /**
         * Gets the angle of rotation, and stores the axis of rotation
         * of this transform in the specified glm::vec3.
         *
         * @param axis The vector to store the axis of rotation.
         *
         * @return The angle of rotation.
         */
        float getRotation(glm::vec3* axis) const;

        /**
         * Returns the translation for this transform.
         */
        const glm::vec3& getTranslation() const;

        /**
         * Gets the translation component of this transform in the specified vector.
         *
         * @param translation The vector to store the translation in.
         */
        void getTranslation(glm::vec3* translation) const;

        /**
         * Gets the translation factor along the x-axis of this transform.
         *
         * @return The translation factor along the x-axis.
         */
        float getTranslationX() const;

        /**
         * Gets the translation factor along the y-axis of this transform.
         *
         * @return The translation factor along the y-axis.
         */
        float getTranslationY() const;

        /**
         * Gets the translation factor along the z-axis of this transform.
         *
         * @return The translation factor along the z-axis.
         */
        float getTranslationZ() const;

        /**
         * Rotates this transform's rotation component by the given rotation.
         *
         * @param qx The quaternion x value.
         * @param qy The quaternion y value.
         * @param qz The quaternion z value.
         * @param qw The quaternion w value.
         */
        void rotate(float qx, float qy, float qz, float qw);

        /**
         * Rotates this transform's rotation component by the given rotation.
         *
         * @param rotation The rotation to rotate by (as a quaternion).
         */
        void rotate(const glm::quat& rotation);

        /**
         * Rotates this transform's rotation component by the given rotation
         * (defined as an axis angle rotation).
         *
         * @param axis The axis to rotate about.
         * @param angle The axis to rotate about (in radians).
         */
        void rotate(const glm::vec3& axis, float angle);

        /**
         * Rotates this transform's rotation component by the given rotation.
         *
         * @param rotation The rotation to rotate by (as a matrix).
         */
        void rotate(const glm::mat4& rotation);

        /**
         * Rotates this transform's rotation component by the given angle
         * about the x-axis.
         *
         * @param angle The angle to rotate by about the x-axis (in radians).
         */
        void rotateX(float angle);

        /**
         * Rotates this transform's rotation component by the given angle
         * about the y-axis.
         *
         * @param angle The angle to rotate by about the y-axis (in radians).
         */
        void rotateY(float angle);

        /**
         * Rotates this transform's rotation component by the given angle
         * about the z-axis.
         *
         * @param angle The angle to rotate by about the z-axis (in radians).
         */
        void rotateZ(float angle);

        /**
         * Scales this transform's scale component by the given factor along all axes.
         *
         * @param scale The factor to scale by along all axis.
         */
        void scale(float scale);

        /**
         * Scales this transform's scale component by the given factors along each axis.
         *
         * @param sx The factor to scale by in the x direction.
         * @param sy The factor to scale by in the y direction.
         * @param sz The factor to scale by in the z direction.
         */
        void scale(float sx, float sy, float sz);

        /**
         * Scales this transform's scale component by the given scale vector.
         *
         * @param scale The vector to scale by.
         */
        void scale(const glm::vec3& scale);

        /**
         * Scales this transform's scale component by the given scale
         * factor along the x axis.
         *
         * @param sx The scale factor along the x axis.
         */
        void scaleX(float sx);

        /**
         * Scales this transform's scale component by the given scale
         * factor along the y axis.
         *
         * @param sy The scale factor along the y axis.
         */
        void scaleY(float sy);

        /**
         * Scales this transform's scale component by the given scale
         * factor along the z axis.
         *
         * @param sz The scale factor along the z axis.
         */
        void scaleZ(float sz);

        /**
         * Sets the transform to the specified values.
         *
         * @param scale The scale vector.
         * @param rotation The rotation quaternion.
         * @param translation The translation vector.
         */
        void set(const glm::vec3& scale, const glm::quat& rotation, const glm::vec3& translation);

        /**
         * Sets the transform to the specified values.
         *
         * @param scale The scale vector.
         * @param rotation The rotation matrix.
         * @param translation The translation vector.
         */
        void set(const glm::vec3& scale, const glm::mat4& rotation, const glm::vec3& translation);

        /**
         * Sets the transform to the specified values.
         *
         * @param scale The scale vector.
         * @param axis The axis of rotation.
         * @param angle The angle of rotation (in radians).
         * @param translation The translation vector.
         */
        void set(const glm::vec3& scale, const glm::vec3& axis, float angle, const glm::vec3& translation);

        /**
         * Sets this transform to the specified transform.
         *
         * @param transform The transform to set this transform to.
         */
        void set(const Transform& transform);

        /**
         * Sets this transform to the identity transform.
         */
        void setIdentity();

        /**
         * Sets the scale factor along all axes for this transform
         * to the specified value.
         *
         * @param scale The scale factor along all axes.
         */
        void setScale(float scale);

        /**
         * Sets the scale component of this transform to the
         * specified values.
         *
         * @param sx The scale factor along the x axis.
         * @param sy The scale factor along the y axis.
         * @param sz The scale factor along the z axis.
         */
        void setScale(float sx, float sy, float sz);

        /**
         * Sets the scale component of this transform to the
         * specified scale vector.
         *
         * @param scale The scale vector.
         */
        void setScale(const glm::vec3& scale);

        /**
         * Sets the scale factor along the x-axis for this transform
         * to the specified value.
         *
         * @param sx The scale factor along the x-axis.
         */
        void setScaleX(float sx);

        /**
         * Sets the scale factor along the y-axis for this transform
         * to the specified value.
         *
         * @param sy The scale factor along the y-axis.
         */
        void setScaleY(float sy);

        /**
         * Sets the scale factor along the z-axis for this transform
         * to the specified value.
         *
         * @param sz The scale factor along the z-axis.
         */
        void setScaleZ(float sz);

        /**
         * Sets the rotation component for this transform to the
         * specified values.
         *
         * @param qx The quaternion x value.
         * @param qy The quaternion y value.
         * @param qz The quaternion z value.
         * @param qw The quaternion w value.
         */
        void setRotation(float qx, float qy, float qz, float qw);

        /**
         * Sets the rotation component for this transform to the
         * specified values.
         *
         * @param rotation The rotation as a quaternion.
         */
        void setRotation(const glm::quat& rotation);

        /**
         * Sets the rotation component for this transform to the
         * specified values.
         *
         * @param rotation The rotation as a matrix.
         */
        void setRotation(const glm::mat4& rotation);

        /**
         * Sets the rotation component for this transform to the rotation from the specified axis and angle.
         *
         * @param axis The axis of rotation.
         * @param angle The angle of rotation (in radians).
         */
        void setRotation(const glm::vec3& axis, float angle);

        /**
         * Sets the translation component for this transform to the
         * specified translation vector.
         *
         * @param translation The translation vector.
         */
        void setTranslation(const glm::vec3& translation);

        /**
         * Sets the translation component for this transform
         * to the specified values.
         *
         * @param tx The translation amount in the x direction.
         * @param ty The translation amount in the y direction.
         * @param tz The translation amount in the z direction.
         */
        void setTranslation(float tx, float ty, float tz);

        /**
         * Sets the translation factor along the x-axis for this
         * transform to the specified value.
         *
         * @param tx The translation factor along the x-axis.
         */
        void setTranslationX(float tx);

        /**
         * Sets the translation factor along the y-axis for this
         * transform to the specified value.
         *
         * @param ty The translation factor along the y-axis.
         */
        void setTranslationY(float ty);

        /**
         * Sets the translation factor along the z-axis for this
         * transform to the specified value.
         *
         * @param tz The translation factor along the z-axis.
         */
        void setTranslationZ(float tz);

        /**
         * Translates this transform's translation component by the
         * given values along each axis.
         *
         * @param tx The amount to translate along the x axis.
         * @param ty The amount to translate along the y axis.
         * @param tz The amount to translate along the z axis.
         */
        void translate(float tx, float ty, float tz);

        /**
         * Translates this transform's translation component by the
         * given translation vector.
         *
         * @param translation The amount to translate.
         */
        void translate(const glm::vec3& translation);

        /**
         * Translates this transform's translation component by the
         * given value along the x axis.
         *
         * @param tx The amount to translate along the x axis.
         */
        void translateX(float tx);

        /**
         * Translates this transform's translation component by the
         * given value along the y axis.
         *
         * @param ty The amount to translate along the y axis.
         */
        void translateY(float ty);

        /**
         * Translates this transform's translation component by the
         * given value along the z axis.
         *
         * @param tz The amount to translate along the z axis.
         */
        void translateZ(float tz);

        /**
         * Translates the camera towards the given target using a smoothing function.
         * The given response time determines the amount of smoothing (lag). A longer
         * response time yields a smoother result and more lag. To force the camera to
         * follow the target closely, provide a response time that is very small relative
         * to the given elapsed time.
         *
         * @param target target value.
         * @param elapsedTime elapsed time between calls.
         * @param responseTime response time (in the same units as elapsedTime).
         */
        void translateSmooth(const glm::vec3& target, float elapsedTime, float responseTime);

        /**
         * Transforms the specified vector and stores the
         * result in the original vector.
         *
         * @param vector The vector to transform.
         */
        void transformVector(glm::vec3* vector) const;

        /**
         * Transforms the specified vector and stores the result
         * in the original vector.
         *
         * @param transformVector The vector to transform.
         * @param dst The vector to store the result in.
         */
        void transformVector(const glm::vec3& transformVector, glm::vec3* dst) const;

        /**
         * Adds a transform listener.
         *
         * @param listener The listener to add.
         * @param cookie An optional long value that is passed to the specified listener when it is called.
         */
        void addListener(Transform::Listener* listener, long cookie = 0);

        /**
         * Removes a transform listener.
         *
         * @param listener The listener to remove.
         */
        void removeListener(Transform::Listener* listener);

        Transform mix(const Transform& other, float bias)
        {
            BOOST_ASSERT(bias >= 0 && bias <= 1);
            Transform res;
            res.setTranslation(glm::mix(_translation, other._translation, bias));
            res.setRotation(glm::slerp(_rotation, other._rotation, bias));
            res.setScale(glm::mix(_scale, other._scale, bias));
            return res;
        }

    protected:

        /**
         * Transform Listener.
         */
        struct TransformListener
        {
            /**
             * Listener for Transform events.
             */
            Listener* listener;

            /**
             * An optional long value that is specified to the Listener's callback.
             */
            long cookie;
        };


        /**
         * Defines the matrix dirty bits for marking the translation, scale and rotation
         * components of the Transform.
         */
        enum MatrixDirtyBits
        {
            DIRTY_TRANSLATION = 0x01,
            DIRTY_SCALE = 0x02,
            DIRTY_ROTATION = 0x04,
            DIRTY_NOTIFY = 0x08
        };


        /**
         * Marks this transform as dirty and fires transformChanged().
         */
        void dirty(char matrixDirtyBits);

        /**
         * Determines if the specified matrix dirty bit is set.
         *
         * @param matrixDirtyBits the matrix dirty bit to check for dirtiness.
         * @return TRUE if the specified matrix dirty bit is set; FALSE if the specified matrix dirty bit is unset.
         */
        bool isDirty(char matrixDirtyBits) const;

        /**
         * Adds the specified transform to the list of transforms waiting to be notified of a change.
         * Sets the DIRTY_NOTIFY bit on the transform.
         */
        static void suspendTransformChange(const std::shared_ptr<Transform>& transform);

        /**
         * Called when the transform changes.
         */
        virtual void transformChanged();

        /**
         * The scale component of the Transform.
         */
        glm::vec3 _scale;

        /**
         * The rotation component of the Transform.
         */
        glm::quat _rotation;

        /**
         * The translation component of the Transform.
         */
        glm::vec3 _translation;

        /**
         * The glm::mat4 representation of the Transform.
         */
        mutable glm::mat4 _matrix;

        /**
         * glm::mat4 dirty bits flag.
         */
        mutable char _matrixDirtyBits;

        /**
         * List of TransformListener's on the Transform.
         */
        std::list<TransformListener> _listeners;

    private:

        static int _suspendTransformChanged;
        static std::vector<std::shared_ptr<Transform>> _transformsChanged;
    };
}
