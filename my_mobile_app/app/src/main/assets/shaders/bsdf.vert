#version 300 es

precision mediump float;

in vec3 inPosition;
in vec3 inNormal;
in vec4 inTangent;
in vec2 inUV;

uniform mat4 uModel;
uniform mat4 uCameraView;
uniform mat4 uProjection;

out vec3 vPosition;
out vec3 vNormal;
out vec3 vTangent;
out vec3 vBiTangent;
out vec2 vUV;

void main()
{
    mat4 modelViewMatrix = uCameraView * uModel;

    gl_Position = uProjection * modelViewMatrix * vec4(inPosition, 1.0);

    mat4 model_inverse = inverse(uModel);

    vec4 world_pos = uModel * vec4(inPosition, 1.0);
    vPosition = world_pos.xyz / world_pos.w;
    vNormal = normalize(mat3(model_inverse) * inNormal);
    vTangent = normalize(mat3(uModel) * inTangent.xyz);
    vBiTangent = cross(vNormal, vTangent) * inTangent.w;
    vUV = inUV;
}

