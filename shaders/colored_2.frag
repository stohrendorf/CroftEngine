uniform vec4 u_diffuseColor;

#include "darkness.frag"

void main()
{
    gl_FragColor = u_diffuseColor;

    gl_FragColor.rgb *= calcBrightness();
}
