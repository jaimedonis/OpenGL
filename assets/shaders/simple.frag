precision mediump float;

#define PI 3.1415

in vec3 vPosition;
in vec3 vNormal;
in vec3 vTangent;
in vec3 vBiTangent;
in vec2 vUV;

uniform mat4 uModel;
uniform mat4 uCameraView;
uniform mat4 uProjection;

uniform vec3 uCameraPosition;
uniform vec3 uLightPosition;
uniform vec3 uLightColor;

uniform sampler2D uColorTexture;
uniform sampler2D uMetallicRoughnessTexture;
uniform sampler2D uNormalTexture;

struct Material {
    vec3 surface_albedo;
    float specular_power;
    vec3 specular_color;
    vec3 ambient_color;
    vec3 diffuse_color;
    bool is_metal;
};
layout(std140) uniform uMaterialBlock
{
    Material uMaterial;
};

out vec4 fragColor;



float DistributionGGXp(float alpha, vec3 n, vec3 m)
{
    float distribution = ((alpha + 2.0) / (2.0*PI)) * pow(clamp(dot(n, m), 0.0, 1.0), alpha);

    return distribution;
}

float DistributionGGXuabc(float alpha1, float alpha2, vec3 n, vec3 m)
{
    float distribution =  1.0 / ((1.0 + alpha1) * (1.0 - pow(clamp(dot(n, m), 0.0, 1.0), alpha2)));

    return distribution;
}

float GeometricS1(float dp, float alpha)
{
    float k = (alpha + 1.0) * (alpha + 1.0) / 8.0;
    float denom = dp * (1.0 - k) + k;
    return dp / denom;
}

float GeometricSmith(vec3 l, vec3 v, vec3 h, float alpha)
{
    float dot1 = clamp(dot(l, h), 0.0, 1.0);
    float dot2 = clamp(dot(v, h), 0.0, 1.0);

    float ret = GeometricS1(dot1, alpha) * GeometricS1(dot1, alpha);

    return ret;
}

vec3 SchlickFresnel(vec3 F0, vec3 l, vec3 direction)
{
    vec3 ret = F0 + (1.0 - F0) * pow((1.0 - clamp(dot(l, direction), 0.0, 1.0)), 5.0);

    return ret;
}

vec3 SpecularBRDF(vec3 F0, vec3 l, vec3 v, vec3 n, float alpha1, float alpha2, bool is_microfacet)
{
    vec3 h = normalize(v + l);

    vec3 direction;
    if( is_microfacet ) {
        direction = h;
    } else {
        direction = n;
    }
    vec3 F = SchlickFresnel(F0, l, direction);
    float G = GeometricSmith(l, v, h, alpha1);
    float D = DistributionGGXuabc(alpha1, alpha2, n, h);

    vec3 brdf = (F * G * D) / (4.0 * dot(n, l) * dot(n, v) + 0.001);

    return brdf;
}

vec3 Shade(vec3 l, vec3 v, vec3 n, vec3 diffuse_color)
{
    vec3 F0 = vec3(0.045);
    float alpha1 = 1.0;
    float alpha2 = 1.0;
    bool is_microfacet = true;
    vec3 specular_color = uMaterial.specular_color;

    vec3 kS = SpecularBRDF(F0, l, v, n, alpha1, alpha2, is_microfacet);
    vec3 kD = 1.0 - kS;

    vec3 color = (kD * diffuse_color) + (kS * specular_color);

    return color;
}

void main()
{
    vec3 normal = normalize( vNormal );
    vec3 tangent = normalize( vTangent );
    vec3 bitangent = normalize( vBiTangent );

    vec4 diffuse_color = texture(uColorTexture, vUV).rgba;

    vec3 light_direction = normalize(uLightPosition - vPosition);
    vec3 view_direction = normalize(uCameraPosition - vPosition);

    vec3 final_color = Shade(light_direction, view_direction, normal, diffuse_color.rgb);

    fragColor = vec4(final_color, diffuse_color.a);
}

