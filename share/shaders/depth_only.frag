#include "geometry_pipeline_interface.glsl"

layout(bindless_sampler) uniform sampler2DArray u_diffuseTextures;

void main()
{
    if (gpi.texCoord.z >= 0) {
        vec4 baseColor = texture(u_diffuseTextures, gpi.texCoord);
        if (baseColor.a != 1) {
            discard;
        }
    }
}
