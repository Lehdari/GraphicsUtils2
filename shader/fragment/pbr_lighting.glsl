#version 450
#pragma shader_stage(fragment)


layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform sampler2D gBufferBaseColor;


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
    outColor = texture(baseColor, 1.0);
    #else
    outColor = vec4(1.0, 1.0, 1.0, 1.0);
    #endif
}
