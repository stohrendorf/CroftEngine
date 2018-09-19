uniform sampler2D u_depth;

float depth_at(in vec2 uv)
{
    vec4 clipSpaceLocation;
    clipSpaceLocation.xy = uv * 2 - vec2(1); // normalized device coordinates
    clipSpaceLocation.z = texture2D(u_depth, uv).r * 2.0 - 1.0;
    clipSpaceLocation.w = 1;
    vec4 camSpaceLocation = inverse(u_projection) * clipSpaceLocation;
    float d = length(camSpaceLocation.xyz / camSpaceLocation.w);
    d /= Z_max;
    return clamp(d, 0.0, 1.0);
}
