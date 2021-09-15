layout(location=6) uniform float u_time;

float time_seconds()
{
    return u_time / 1000.0;
}
