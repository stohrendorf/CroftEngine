layout(early_fragment_tests) in;

layout(location=0) out vec4 out_color;
layout(location=1) out vec3 out_normal;
layout(location=2) out vec3 out_position;
layout(location=3) out vec4 out_reflective;

void main() {
    out_color = vec4(0.8, 0.8, 1.0, 1.0);
    out_normal = vec3(0.0, 0.0, 0.0);
    out_position = vec3(0.0, 0.0, 0.0);
    out_reflective = vec4(0.0, 0.0, 0.0, 0.0);
}
