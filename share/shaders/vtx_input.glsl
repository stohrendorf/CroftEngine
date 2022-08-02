layout(location=0) in vec3 a_position;

layout(location=1) in vec3 a_normal;

layout(location=2) in vec4 a_color;
layout(location=3) in vec3 a_texCoord;

layout(location=4) in float a_boneIndex;

layout(location=5) in float a_isQuad;
layout(location=6) in vec3 a_quadVert1;
layout(location=7) in vec3 a_quadVert2;
layout(location=8) in vec3 a_quadVert3;
layout(location=9) in vec3 a_quadVert4;
layout(location=10) in vec4 a_quadUv12;
layout(location=11) in vec4 a_quadUv34;

// warning: locations are re-used for "reflective" and "modelMatrix"
layout(location=12) in vec4 a_colorTopLeft;
layout(location=13) in vec4 a_colorTopRight;
layout(location=14) in vec4 a_colorBottomLeft;
layout(location=15) in vec4 a_colorBottomRight;

// warning: re-uses location
layout(location=12) in vec4 a_reflective;

// warning: re-uses location
layout(location=12) in mat4 a_modelMatrix;
