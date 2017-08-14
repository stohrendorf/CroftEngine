#pragma once

#include <glm/glm.hpp>

namespace gameplay
{
    class Node;


    class Camera final
    {
        friend class Node;

    public:
        Camera(float fieldOfView, float aspectRatio, float nearPlane, float farPlane);

        ~Camera() = default;

        float getFieldOfView() const;

        void setFieldOfView(float fieldOfView);

        float getAspectRatio() const;

        float getNearPlane() const;

        float getFarPlane() const;

        const glm::mat4& getViewMatrix() const;

        void setViewMatrix(const glm::mat4& m);

        const glm::mat4& getInverseViewMatrix() const;

        const glm::mat4& getProjectionMatrix() const;

        const glm::mat4& getViewProjectionMatrix() const;

        const glm::mat4& getInverseViewProjectionMatrix() const;

    private:

        Camera& operator=(const Camera&) = delete;

        void transformChanged();

        float _fieldOfView;
        float _aspectRatio;
        float _nearPlane;
        float _farPlane;
        mutable glm::mat4 _view;
        mutable glm::mat4 _projection;
        mutable glm::mat4 _viewProjection;
        mutable glm::mat4 _inverseView;
        mutable glm::mat4 _inverseViewProjection;
        mutable int _bits;
    };
}
