layout(std140, binding=1) uniform Camera {
    mat4 u_projection;
    mat4 u_inverseProjection;
    mat4 u_view;
    mat4 u_inverseView;
    mat4 u_viewProjection;
    mat4 u_inverseViewProjection;
    vec4 u_screenSize;
    float u_aspectRatio;
    float u_nearPlane;
    float u_farPlane;
};
