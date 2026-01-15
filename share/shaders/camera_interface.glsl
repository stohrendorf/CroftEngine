#include "bindings.glsl"

layout(std140, binding=BINDING_UNIFORM_CAMERA) uniform Camera {
    mat4 projection;
    mat4 view;
    mat4 viewProjection;
    vec4 viewport;
    float aspectRatio;
    float nearPlane;
    float farPlane;
} camera;


float InvFarPlane = 1.0 / camera.farPlane;
