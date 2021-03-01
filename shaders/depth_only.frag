#include "geometry_pipeline_interface.glsl"

layout(binding=4) uniform sampler2DArray u_diffuseTextures;

void main()
{
    if (gpi.texIndex >= 0) {
        vec4 baseColor = texture(u_diffuseTextures, vec3(gpi.texCoord, gpi.texIndex));
        if (baseColor.a < 0.5) {
            discard;
        }
    }
}
