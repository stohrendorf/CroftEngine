layout(std140, binding=1) uniform Camera {
    mat4 u_projection;
    mat4 u_inverseProjection;
    mat4 u_view;
    mat4 u_inverseView;
    mat4 u_viewProjection;
    mat4 u_inverseViewProjection;
    float aspect_ratio;
    float near_plane;
    float far_plane;
};
