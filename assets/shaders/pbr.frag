#version 300 es

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


float DotProd(vec3 a, vec3 b)
{
    return abs(dot(a, b)) + 0.000001;
}

float DistributionGGXuabc(float alpha1, float alpha2, float NdotM)
{
    float distribution =  1.0 / pow(1.0 + alpha1 * (1.0 - NdotM), alpha2);

    return distribution;
}

float GeometricS1(float dp, float alpha)
{
    float k = (alpha + 1.0) * (alpha + 1.0) / 8.0;
    float denom = dp * (1.0 - k) + k;

    return dp / denom;
}

float GeometricSmith(float HdotL, float HdotV, float alpha)
{
    float ret = GeometricS1(HdotL, alpha) * GeometricS1(HdotV, alpha);

    return ret;
}

vec3 SchlickFresnel(vec3 F0, float HdotL)
{
    vec3 ret = F0 + (1.0 - F0) * pow(1.0 - HdotL, 5.0);

    return ret;
}

vec3 DiffuseBRDF(vec3 surface_albedo)
{
    vec3 diffuse = (surface_albedo / PI);

    return diffuse;
}

vec3 SpecularBRDF(vec3 F0, float alpha1, float alpha2, float NdotV, float NdotL, float HdotL, float HdotV, float NdotH)
{
    vec3 F = SchlickFresnel(F0, HdotL);
    float G = GeometricSmith(HdotL, HdotV, alpha1);
    float D = DistributionGGXuabc(alpha1, alpha2, NdotH);

    vec3 specular = (F * G * D) / (4.0 * NdotL * NdotV);

    return specular;
}

vec3 Shade(vec3 l, vec3 v, vec3 n, vec3 diffuse_color, float roughness, float metalness)
{
    vec3 F0 = vec3(0.045); // TBD

    float alpha1 = roughness;
    float alpha2 = metalness;
    bool is_microfacet = true;
    vec3 specular_color = uMaterial.specular_color;

    bool is_point_light = true;
    vec3 irradiance;
    if( !is_point_light ) {
        // attenuate
        float attenuation_amount = length(l);
        float attenuation_term = 1.0 / (attenuation_amount * attenuation_amount);
        irradiance = uLightColor * attenuation_term;
    } else {
        irradiance = uLightColor;
    }

    l = normalize(l);
    v = normalize(v);
    vec3 h = normalize(v + l);

    const float delta = 0.000001;
    float NdotV = abs(dot(n, v)) + delta;
    float NdotL = abs(dot(n, l)) + delta;
    float HdotL = abs(dot(h, l)) + delta;
    float HdotV = abs(dot(h, v)) + delta;
    float NdotH = abs(dot(n, h)) + delta;

    vec3 F = SchlickFresnel(F0, HdotL);

    vec3 specular_brdf = SpecularBRDF(F0, alpha1, alpha2, NdotV, NdotL, HdotL, HdotV, NdotH);
    vec3 diffuse_brdf = DiffuseBRDF(uMaterial.surface_albedo);

    vec3 kS = specular_brdf;
    vec3 kD = diffuse_brdf;

    vec3 final_color = (kD * diffuse_color + kS * specular_color) * irradiance * NdotL;

    return final_color;
}

vec3 FetchObjectNormal(vec2 uv, vec3 normal, vec3 tangent, vec3 bitangent)
{
    vec3 bump_map_normal = texture(uNormalTexture, uv).rgb * 2.0 - 1.0;
    bump_map_normal = normalize(bump_map_normal);

    mat3 TBN = mat3( tangent,
                     bitangent,
                     normal );

    vec3 new_normal = TBN * bump_map_normal;

    return new_normal;
}

void main()
{
    vec3 normal = normalize( vNormal );
    vec3 tangent = normalize( vTangent );
    vec3 bitangent = normalize( vBiTangent );

    float roughness = 4.0; // TBD
    float metalness = 0.0; // TBD

    normal = FetchObjectNormal(vUV, normal, tangent, bitangent);

    vec4 diffuse_color = texture(uColorTexture, vUV).rgba;

    vec3 light_direction = uLightPosition - vPosition;
    vec3 view_direction = uCameraPosition - vPosition;

    vec3 final_color = Shade(light_direction, view_direction, normal,
                             diffuse_color.rgb, roughness, metalness);

    fragColor = vec4(final_color, diffuse_color.a);
}
