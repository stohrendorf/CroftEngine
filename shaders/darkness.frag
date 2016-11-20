varying float v_depth;

// simulate a quadratic falloff from 12 to 20 sectors
float calcBrightness()
{
    float start = 12 * 1024;
    float end = 20 * 1024;

    if(v_depth < start)
        return 1;

    if(v_depth > end)
        return 0;

    float f = 1 - (v_depth - start) / (end - start);
    return f*f;
}
