vec3 reinhard_tonemapping(vec3 color, float exposure)
{
    color *= exposure;
    return color / (color + vec3(1.0));
}

vec3 u2_filmic_tonemapping(vec3 color, float exposure)
{
    color *= exposure;

    const float A = 0.15;
    const float B = 0.50;
    const float C = 0.10;
    const float D = 0.20;
    const float E = 0.02;
    const float F = 0.30;

    return ((color * (A * color + vec3(C * B)) + vec3(D * E)) / (color * (A * color + vec3(B)) + vec3(D * F))) - vec3(E / F);
}

vec3 aces_filmic_tonemapping(vec3 x)
{
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return (x * (a * x + b)) / (x * (c * x + d) + e);
}

vec3 aces_tonemapping(vec3 color, float exposure)
{
    color *= exposure;
    color = aces_filmic_tonemapping(color);
    return color;
}
