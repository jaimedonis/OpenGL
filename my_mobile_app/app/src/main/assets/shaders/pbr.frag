#version 300 es

///////////////////////////////////////////////////////////////////////////////
// Created by Jaime Donis
// Last modified date: Aug 1, 2025
//

precision mediump float;

#define PI 3.1415
#define MAX_NUM_LIGHTS 4
#define LIGHT_TYPE_POINT 0
#define LIGHT_TYPE_DIRECTIONAL 1
#define MATERIAL_TYPE_MICROFACET 0
#define MATERIAL_TYPE_FLAT_MIRROR 1

in vec3 vPosition;
in vec3 vNormal;
in vec3 vTangent;
in vec3 vBiTangent;
in vec2 vUV;

uniform mat4 uModel;
uniform mat4 uCameraView;
uniform mat4 uProjection;

uniform vec3 uCameraPosition;

uniform sampler2D uColorTexture;
uniform sampler2D uMetallicRoughnessTexture;
uniform sampler2D uNormalTexture;

struct SceneLight {
    int light_type;
    vec3 light_position;
    vec3 light_color;
};
layout(std140) uniform uPBRLightingBlock
{
    SceneLight uPBRLights[MAX_NUM_LIGHTS];
};
uniform int uNumberOfLights;

struct PBRMaterial {
    int material_type;
    vec3 subsurface_albedo;
    vec3 F0;
};
layout(std140) uniform uMaterialBlock
{
    PBRMaterial uPBRMaterial;
};

out vec4 fragColor;


float DistributionGGXp(float alpha, float NdotM)
{
    float distribution = ((alpha + 2.0) / (2.0*PI)) * pow(NdotM, alpha);

    return distribution;
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

vec3 DiffuseBRDF(vec3 subsurface_albedo, vec3 F)
{
    vec3 diffuse = (vec3(1.0) - F) * (subsurface_albedo / PI);

    return diffuse;
}

vec3 SpecularBRDF(vec3 F0, vec3 F, float alpha1, float alpha2, float NdotV, float NdotL, float HdotL, float HdotV, float NdotH)
{
    float G = GeometricSmith(HdotL, HdotV, alpha1);
    float D = DistributionGGXuabc(alpha1, alpha2, NdotH);
    //float D = DistributionGGXp(alpha1, NdotH);

    vec3 specular = (F * G * D) / (4.0 * NdotL * NdotV);

    return specular;
}

vec3 Shade(vec3 l, vec3 v, vec3 n, vec3 diffuse_color, int light_type, vec3 light_color, int material_type, float roughness, float metalness)
{
    float alpha1 = roughness;
    float alpha2 = metalness;

    l = normalize(l);
    v = normalize(v);
    vec3 h = normalize(v + l);

    const float delta = 0.001;
    float NdotV = abs(dot(n, v)) + delta;
    float NdotL = abs(dot(n, l)) + delta;
    float HdotL = abs(dot(h, l)) + delta;
    float HdotV = abs(dot(h, v)) + delta;
    float NdotH = abs(dot(n, h)) + delta;

    vec3 F;
    if(material_type == MATERIAL_TYPE_MICROFACET) {
        F = SchlickFresnel(uPBRMaterial.F0, HdotL);
    } else {
        F = SchlickFresnel(uPBRMaterial.F0, NdotL);
    }

    vec3 specular_brdf = SpecularBRDF(uPBRMaterial.F0, F, alpha1, alpha2, NdotV, NdotL, HdotL, HdotV, NdotH);
    vec3 diffuse_brdf = DiffuseBRDF(uPBRMaterial.subsurface_albedo, F);

    vec3 kS = specular_brdf;
    vec3 kD = diffuse_brdf;

    vec3 final_color = ((kD * diffuse_color + kS) * light_color) * NdotL;

    return final_color;
}

vec3 FetchObjectNormal(vec2 uv, vec3 normal, vec3 tangent, vec3 bitangent)
{
    vec3 bump_map_normal = texture(uNormalTexture, uv).rgb * 2.0 - 1.0;

    mat3 TBN = mat3( tangent,
                     bitangent,
                     normal );

    vec3 new_normal = TBN * bump_map_normal;

    return normalize(new_normal);
}

void main()
{
    vec3 normal = normalize( vNormal );
    vec3 tangent = normalize( vTangent );
    vec3 bitangent = normalize( vBiTangent );

    vec4 alpha = texture(uMetallicRoughnessTexture, vUV);
    float roughness = alpha.g;
    float metalness = alpha.b;

    normal = FetchObjectNormal(vUV, normal, tangent, bitangent);

    mat3 TBN = mat3( tangent,
                     bitangent,
                     normal );

    vec4 diffuse_color = texture(uColorTexture, vUV).rgba;

    vec3 final_color = vec3(0.0);
    for(int i = 0; i < uNumberOfLights; ++i ) {
        vec3 light_pos = uPBRLights[i].light_position;
        vec3 light_color = uPBRLights[i].light_color;
        int light_type = uPBRLights[i].light_type;
        vec3 light_direction;
        if(light_type == LIGHT_TYPE_POINT) {
            light_direction = light_pos - vPosition;
        } else {
            light_direction = light_pos;
        }
        vec3 view_direction = uCameraPosition - vPosition;
        light_direction = TBN * light_direction;
        view_direction = TBN * view_direction;

        final_color += Shade(light_direction, view_direction, normal,
                             diffuse_color.rgb, light_type, light_color,
                             uPBRMaterial.material_type, roughness, metalness);
    }

    fragColor = vec4(final_color, diffuse_color.a);
}
