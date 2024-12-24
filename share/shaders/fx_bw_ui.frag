#include "flat_pipeline_interface.glsl"
#include "fx_input.glsl"
#include "util.glsl"
#include "time_uniform.glsl"

/* just make the health bar black & white without applying the film 
   effect to it because you can't see your health otherwise */ 
void main() {
     vec2 uv = fpi.texCoord;
     vec4 image = texture(u_input,uv);
     out_color = vec3(luminanceRgb(image));
}