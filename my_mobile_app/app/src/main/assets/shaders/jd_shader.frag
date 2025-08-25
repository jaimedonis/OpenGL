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
uniform sampler2D uNormalTexture;

struct Material {
    vec3 surface_albedo;
    float specular_power;
    vec3 specular_color;
    vec3 ambient_color;
    vec3 diffuse_color;
};
layout(std140) uniform uMaterialBlock
{
    Material uMaterial;
};

out vec4 fragColor;


vec3 FetchObjectNormal(vec2 uv, vec3 normal, vec3 tangent, vec3 bitangent)
{
    vec3 bump_map_normal = texture(uNormalTexture, uv).rgb;
    bump_map_normal = normalize(bump_map_normal) * 2.0 - 1.0;

    mat3 TBN = mat3( tangent,
                     bitangent,
                     normal );

    vec3 new_normal = TBN * bump_map_normal;

    return new_normal;
}

vec3 ComputeDiffuseReflection(vec3 normal, vec3 light_direction, vec3 light_color, vec3 diffuse_color)
{
    vec3 direct_color = light_color * dot(normal, light_direction);
    return (uMaterial.ambient_color + direct_color) * diffuse_color;
}

vec3 ComputeSpecularReflection(vec3 normal, vec3 half_vector, float nl, vec3 light_color)
{
    float highlight = pow(clamp(dot(normal, half_vector),0.0, 1.0), uMaterial.specular_power) * float(nl > 0.0);
    return light_color * uMaterial.specular_color * highlight;
}

vec3 Shade(vec3 world_position, vec3 normal, vec3 camera_position, vec3 diffuse_color)
{
    vec3 light_direction = normalize(uLightPosition - world_position);
    vec3 diffuse = ComputeDiffuseReflection(normal, light_direction, uLightColor, diffuse_color);

    vec3 eye_direction = normalize(camera_position - world_position);
    vec3 half_vector = normalize(light_direction + eye_direction);
    float nl = clamp(dot(normal, light_direction), 0.0, 1.0);
    vec3 specular = ComputeSpecularReflection(normal, half_vector, nl, uLightColor);

    float nh = clamp(dot(normal, half_vector), 0.0, 1.0);
    vec3 direct_color = (((uMaterial.surface_albedo / PI) * (diffuse * nl)) + (specular * pow(nh, uMaterial.specular_power))) * uLightColor;

    return direct_color;
}

void main()
{
    // re-normalize, after interpolation.
    vec3 normal = normalize( vNormal );
    vec3 tangent = normalize( vTangent );
    vec3 bitangent = normalize( vBiTangent );

    // color texture value
    vec4 diffuse_color = texture(uColorTexture, vUV).rgba;

    normal = FetchObjectNormal(vUV, normal, tangent, bitangent);

    vec3 material_color = Shade(vPosition, normal, uCameraPosition, diffuse_color.rgb);

    fragColor = vec4(material_color, diffuse_color.a);
}
