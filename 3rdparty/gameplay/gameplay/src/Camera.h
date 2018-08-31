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

    void setAspectRatio(float aspectRatio);

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

    float m_fieldOfView;
    float m_aspectRatio;
    float m_nearPlane;
    float m_farPlane;
    mutable glm::mat4 m_view;
    mutable glm::mat4 m_projection;
    mutable glm::mat4 m_viewProjection;
    mutable glm::mat4 m_inverseView;
    mutable glm::mat4 m_inverseViewProjection;
    mutable int m_bits;
};
}
