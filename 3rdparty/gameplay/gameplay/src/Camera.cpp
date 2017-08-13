#include "Base.h"
#include "Camera.h"
#include "Game.h"
#include "Node.h"

#include <glm/gtc/matrix_transform.hpp>

// Camera dirty bits
#define CAMERA_DIRTY_PROJ 2
#define CAMERA_DIRTY_VIEW_PROJ 4
#define CAMERA_DIRTY_INV_VIEW 8
#define CAMERA_DIRTY_INV_VIEW_PROJ 16
#define CAMERA_DIRTY_BOUNDS 32
#define CAMERA_DIRTY_ALL (CAMERA_DIRTY_PROJ | CAMERA_DIRTY_VIEW_PROJ | CAMERA_DIRTY_INV_VIEW | CAMERA_DIRTY_INV_VIEW_PROJ | CAMERA_DIRTY_BOUNDS)

// Other misc camera bits
#define CAMERA_CUSTOM_PROJECTION 64


namespace gameplay
{
    Camera::Camera(float fieldOfView, float aspectRatio, float nearPlane, float farPlane)
        : _type(PERSPECTIVE)
        , _fieldOfView(fieldOfView)
        , _aspectRatio(aspectRatio)
        , _nearPlane(nearPlane)
        , _farPlane(farPlane)
        , _bits(CAMERA_DIRTY_ALL)
        , _listeners()
    {
    }


    Camera::Camera(float zoomX, float zoomY, float aspectRatio, float nearPlane, float farPlane)
        : _type(ORTHOGRAPHIC)
        , _fieldOfView(0)
        , _aspectRatio(aspectRatio)
        , _nearPlane(nearPlane)
        , _farPlane(farPlane)
        , _bits(CAMERA_DIRTY_ALL)
        , _listeners()
    {
        // Orthographic camera.
        _zoom[0] = zoomX;
        _zoom[1] = zoomY;
    }


    Camera::~Camera() = default;


    Camera::Type Camera::getCameraType() const
    {
        return _type;
    }


    float Camera::getFieldOfView() const
    {
        BOOST_ASSERT(_type == Camera::PERSPECTIVE);

        return _fieldOfView;
    }


    void Camera::setFieldOfView(float fieldOfView)
    {
        BOOST_ASSERT(_type == Camera::PERSPECTIVE);

        _fieldOfView = fieldOfView;
        _bits |= CAMERA_DIRTY_PROJ | CAMERA_DIRTY_VIEW_PROJ | CAMERA_DIRTY_INV_VIEW_PROJ | CAMERA_DIRTY_BOUNDS;
        cameraChanged();
    }


    float Camera::getZoomX() const
    {
        BOOST_ASSERT(_type == Camera::ORTHOGRAPHIC);

        return _zoom[0];
    }


    void Camera::setZoomX(float zoomX)
    {
        BOOST_ASSERT(_type == Camera::ORTHOGRAPHIC);

        _zoom[0] = zoomX;
        _bits |= CAMERA_DIRTY_PROJ | CAMERA_DIRTY_VIEW_PROJ | CAMERA_DIRTY_INV_VIEW_PROJ | CAMERA_DIRTY_BOUNDS;
        cameraChanged();
    }


    float Camera::getZoomY() const
    {
        BOOST_ASSERT(_type == Camera::ORTHOGRAPHIC);

        return _zoom[1];
    }


    void Camera::setZoomY(float zoomY)
    {
        BOOST_ASSERT(_type == Camera::ORTHOGRAPHIC);

        _zoom[1] = zoomY;
        _bits |= CAMERA_DIRTY_PROJ | CAMERA_DIRTY_VIEW_PROJ | CAMERA_DIRTY_INV_VIEW_PROJ | CAMERA_DIRTY_BOUNDS;
        cameraChanged();
    }


    float Camera::getAspectRatio() const
    {
        return _aspectRatio;
    }


    void Camera::setAspectRatio(float aspectRatio)
    {
        _aspectRatio = aspectRatio;
        _bits |= CAMERA_DIRTY_PROJ | CAMERA_DIRTY_VIEW_PROJ | CAMERA_DIRTY_INV_VIEW_PROJ | CAMERA_DIRTY_BOUNDS;
        cameraChanged();
    }


    float Camera::getNearPlane() const
    {
        return _nearPlane;
    }


    void Camera::setNearPlane(float nearPlane)
    {
        _nearPlane = nearPlane;
        _bits |= CAMERA_DIRTY_PROJ | CAMERA_DIRTY_VIEW_PROJ | CAMERA_DIRTY_INV_VIEW_PROJ | CAMERA_DIRTY_BOUNDS;
        cameraChanged();
    }


    float Camera::getFarPlane() const
    {
        return _farPlane;
    }


    void Camera::setFarPlane(float farPlane)
    {
        _farPlane = farPlane;
        _bits |= CAMERA_DIRTY_PROJ | CAMERA_DIRTY_VIEW_PROJ | CAMERA_DIRTY_INV_VIEW_PROJ | CAMERA_DIRTY_BOUNDS;
        cameraChanged();
    }


    const glm::mat4& Camera::getViewMatrix() const
    {
        return _view;
    }


    const glm::mat4& Camera::getInverseViewMatrix() const
    {
        if( _bits & CAMERA_DIRTY_INV_VIEW )
        {
            _inverseView = glm::inverse(_view);

            _bits &= ~CAMERA_DIRTY_INV_VIEW;
        }

        return _inverseView;
    }


    const glm::mat4& Camera::getProjectionMatrix() const
    {
        if( !(_bits & CAMERA_CUSTOM_PROJECTION) && (_bits & CAMERA_DIRTY_PROJ) )
        {
            if( _type == PERSPECTIVE )
            {
                _projection = glm::perspective(_fieldOfView, _aspectRatio, _nearPlane, _farPlane);
            }
            else
            {
                // Create an ortho projection with the origin at the bottom left of the viewport, +X to the right and +Y up.
                _projection = glm::ortho(-_zoom[0]/2, _zoom[0]/2, -_zoom[1]/2, _zoom[1]/2, _nearPlane, _farPlane);
            }

            _bits &= ~CAMERA_DIRTY_PROJ;
        }

        return _projection;
    }


    void Camera::setProjectionMatrix(const glm::mat4& matrix)
    {
        _projection = matrix;
        _bits |= CAMERA_CUSTOM_PROJECTION;
        _bits |= CAMERA_DIRTY_PROJ | CAMERA_DIRTY_VIEW_PROJ | CAMERA_DIRTY_INV_VIEW_PROJ | CAMERA_DIRTY_BOUNDS;

        cameraChanged();
    }


    void Camera::resetProjectionMatrix()
    {
        if( _bits & CAMERA_CUSTOM_PROJECTION )
        {
            _bits &= ~CAMERA_CUSTOM_PROJECTION;
            _bits |= CAMERA_DIRTY_PROJ | CAMERA_DIRTY_VIEW_PROJ | CAMERA_DIRTY_INV_VIEW_PROJ | CAMERA_DIRTY_BOUNDS;

            cameraChanged();
        }
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
            _inverseViewProjection = glm::inverse(getViewProjectionMatrix());

            _bits &= ~CAMERA_DIRTY_INV_VIEW_PROJ;
        }

        return _inverseViewProjection;
    }


    void Camera::project(const Rectangle& viewport, const glm::vec3& position, float* x, float* y, float* depth) const
    {
        BOOST_ASSERT(x);
        BOOST_ASSERT(y);

        // Transform the point to clip-space.
        glm::vec4 clipPos = getViewProjectionMatrix() * glm::vec4{ position.x, position.y, position.z, 1.0f };

        // Compute normalized device coordinates.
        BOOST_ASSERT(clipPos.w != 0.0f);
        float ndcX = clipPos.x / clipPos.w;
        float ndcY = clipPos.y / clipPos.w;

        // Compute screen coordinates by applying our viewport transformation.
        *x = viewport.x + (ndcX + 1.0f) * 0.5f * viewport.width;
        *y = viewport.y + (1.0f - (ndcY + 1.0f) * 0.5f) * viewport.height;
        if( depth )
        {
            float ndcZ = clipPos.z / clipPos.w;
            *depth = (ndcZ + 1.0f) / 2.0f;
        }
    }


    void Camera::project(const Rectangle& viewport, const glm::vec3& position, glm::vec2* out) const
    {
        BOOST_ASSERT(out);
        float x, y;
        project(viewport, position, &x, &y);
        *out = { x,y };
    }


    void Camera::project(const Rectangle& viewport, const glm::vec3& position, glm::vec3* out) const
    {
        BOOST_ASSERT(out);
        float x, y, depth;
        project(viewport, position, &x, &y, &depth);
        *out = { x, y, depth };
    }


    void Camera::unproject(const Rectangle& viewport, float x, float y, float depth, glm::vec3* dst) const
    {
        BOOST_ASSERT(dst);

        // Create our screen space position in NDC.
        BOOST_ASSERT(viewport.width != 0.0f && viewport.height != 0.0f);
        glm::vec4 screen((x - viewport.x) / viewport.width, ((viewport.height - y) - viewport.y) / viewport.height, depth, 1.0f);

        // Map to range -1 to 1.
        screen.x = screen.x * 2.0f - 1.0f;
        screen.y = screen.y * 2.0f - 1.0f;
        screen.z = screen.z * 2.0f - 1.0f;

        // Transform the screen-space NDC by our inverse view projection matrix.
        screen = getInverseViewProjectionMatrix() * screen;

        // Divide by our W coordinate.
        if( screen.w != 0.0f )
        {
            screen.x /= screen.w;
            screen.y /= screen.w;
            screen.z /= screen.w;
        }

        *dst = glm::vec3{ screen };
    }


    void Camera::transformChanged()
    {
        _bits |= CAMERA_DIRTY_INV_VIEW | CAMERA_DIRTY_INV_VIEW_PROJ | CAMERA_DIRTY_VIEW_PROJ | CAMERA_DIRTY_BOUNDS;

        cameraChanged();
    }


    void Camera::cameraChanged()
    {
        if( _listeners.empty() )
            return;

        for( auto listener : _listeners )
        {
            listener->cameraChanged(this);
        }
    }


    void Camera::addListener(Camera::Listener* listener)
    {
        _listeners.push_back(listener);
    }


    void Camera::removeListener(Camera::Listener* listener)
    {
        auto it = std::find(_listeners.begin(), _listeners.end(), listener);
        if(it != _listeners.end())
            _listeners.erase(it);
    }


    void Camera::setViewMatrix(const glm::mat4& m)
    {
        _view = m;
        _bits |= CAMERA_DIRTY_PROJ | CAMERA_DIRTY_VIEW_PROJ | CAMERA_DIRTY_INV_VIEW_PROJ | CAMERA_DIRTY_BOUNDS | CAMERA_DIRTY_INV_VIEW;
        cameraChanged();
    }
}
