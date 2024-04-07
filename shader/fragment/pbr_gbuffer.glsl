#version 450
#pragma shader_stage(fragment)

layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec4 fragTangent;
layout(location = 2) in vec2 fragTexCoord0;
#ifndef DISABLE_IN_TEX_COORD_1
layout(location = 3) in vec2 fragTexCoord1;
#endif

layout(location = 0) out vec4 outColor;

#ifndef DISABLE_BASE_COLOR
layout(set = 2, binding = 0) uniform sampler2D baseColorTexture;
#endif
#ifndef DISABLE_METALLIC_ROUGHNESS
layout(set = 2, binding = 1) uniform sampler2D metallicRoughnessTexture;
#endif
#ifndef DISABLE_USE_NORMAL_TEXTURE
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
    #ifndef DISABLE_USE_NORMAL_TEXTURE
    vec3 normal = texture(normalTexture, fragTexCoord0).rgb;
    #endif

    // TODO proper pbr shading

    #ifndef DISABLE_BASE_COLOR
    outColor = vec4(baseColor, 1.0);
    #else
    outColor = vec4(1.0, 1.0, 1.0, 1.0);
    #endif
}
