#include "camera_interface.glsl"
#include "flat_pipeline_interface.glsl"

uniform sampler2D u_depth;

layout(location=0) out float out_depth;

void main()
{
    vec4 clipSpaceLocation = vec4(
    fpi.texCoord,
    texture(u_depth, fpi.texCoord).r,
    1
    ) * 2 - 1;
    vec4 camSpaceLocation = u_inverseProjection * clipSpaceLocation;
    float d = length(camSpaceLocation.xyz / camSpaceLocation.w) / u_farPlane;
    out_depth = clamp(d, 0.0, 1.0);
}
