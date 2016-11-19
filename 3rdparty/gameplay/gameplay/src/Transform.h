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
         * @param rotation The rotation quaternion.
         * @param translation The translation vector.
         */
        Transform(const glm::quat& rotation, const glm::vec3& translation);

        /**
         * Constructs a new transform from the specified values.
         *
         * @param rotation The rotation matrix.
         * @param translation The translation vector.
         */
        Transform(const glm::mat4& rotation, const glm::vec3& translation);

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
         * Returns the rotation for this transform.
         */
        const glm::quat& getRotation() const;

        /**
         * Returns the translation for this transform.
         */
        const glm::vec3& getTranslation() const;

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
         * Sets the transform to the specified values.
         *
         * @param rotation The rotation quaternion.
         * @param translation The translation vector.
         */
        void set(const glm::quat& rotation, const glm::vec3& translation);

        /**
         * Sets the transform to the specified values.
         *
         * @param rotation The rotation matrix.
         * @param translation The translation vector.
         */
        void set(const glm::mat4& rotation, const glm::vec3& translation);

        /**
         * Sets the transform to the specified values.
         *
         * @param axis The axis of rotation.
         * @param angle The angle of rotation (in radians).
         * @param translation The translation vector.
         */
        void set(const glm::vec3& axis, float angle, const glm::vec3& translation);

        /**
         * Sets this transform to the specified transform.
         *
         * @param transform The transform to set this transform to.
         */
        void set(const Transform& transform);

        void set(const glm::mat4& m)
        {
            _rotation = glm::quat_cast(m);
            _translation = glm::vec3(m[3]);
            dirty();
        }

        /**
         * Sets this transform to the identity transform.
         */
        void setIdentity();

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

        Transform mix(const Transform& other, float bias) const
        {
            BOOST_ASSERT(bias >= 0 && bias <= 1);
            Transform res;
            res.setTranslation(glm::mix(_translation, other._translation, bias));
            res.setRotation(glm::slerp(_rotation, other._rotation, bias));
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
         * Marks this transform as dirty and fires transformChanged().
         */
        void dirty();

        /**
         * Determines if the specified matrix dirty bit is set.
         *
         * @param matrixDirtyBits the matrix dirty bit to check for dirtiness.
         * @return TRUE if the specified matrix dirty bit is set; FALSE if the specified matrix dirty bit is unset.
         */
        bool isDirty() const;

        /**
         * Called when the transform changes.
         */
        virtual void transformChanged();

        /**
         * The rotation component of the Transform.
         */
        glm::quat _rotation;

        /**
         * The translation component of the Transform.
         */
        glm::vec3 _translation{0,0,0};

        /**
         * The glm::mat4 representation of the Transform.
         */
        mutable glm::mat4 _matrix{1.0f};

        /**
         * glm::mat4 dirty bits flag.
         */
        mutable bool _matrixDirty = false;

        /**
         * List of TransformListener's on the Transform.
         */
        std::list<TransformListener> _listeners{};
    };
}
