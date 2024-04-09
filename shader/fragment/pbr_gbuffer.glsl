#version 450
#pragma shader_stage(fragment)

layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec4 fragTangent;
layout(location = 2) in vec2 fragTexCoord0;
#ifndef DISABLE_IN_TEX_COORD_1
layout(location = 3) in vec2 fragTexCoord1;
#endif

layout(location = 0) out vec4 outBaseColor;
layout(location = 1) out vec4 outNormal;

#ifndef DISABLE_BASE_COLOR
layout(set = 2, binding = 0) uniform sampler2D baseColorTexture;
#endif
#ifndef DISABLE_METALLIC_ROUGHNESS
layout(set = 2, binding = 1) uniform sampler2D metallicRoughnessTexture;
#endif
#ifndef DISABLE_NORMAL
layout(set = 2, binding = 2) uniform sampler2D normalTexture;
#endif


void main() {
    #ifndef DISABLE_BASE_COLOR
    vec3 baseColor = texture(baseColorTexture, fragTexCoord0).rgb;
    #endif
    #ifndef DISABLE_METALLIC_ROUGHNESS
    float metallicity = texture(metallicRoughnessTexture, fragTexCoord0).b;
    float roughness = texture(metallicRoughnessTexture, fragTexCoord0).g;
    #endif
    #ifndef DISABLE_NORMAL
    vec3 normal = texture(normalTexture, fragTexCoord0).rgb;
    #endif

    #ifndef DISABLE_BASE_COLOR
    outBaseColor = vec4(baseColor, 1.0);
    #else
    outBaseColor = vec4(1.0, 1.0, 1.0, 1.0);
    #endif

    vec3 bitangent = cross(fragNormal, fragTangent.xyz) * -fragTangent.w; // TODO sponza bitangents oriented the wrong way

    mat3 tangentSpaceBase = mat3(fragTangent.xyz, bitangent, fragNormal);

    #ifndef DISABLE_NORMAL
    outNormal = vec4(tangentSpaceBase * normal, 1.0);
    #else
    outNormal = vec4(fragNormal, 1.0);
    #endif
}
