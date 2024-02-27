#version 450
#pragma shader_stage(fragment)

layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec4 fragTangent;
layout(location = 2) in vec2 fragTexCoord0;
layout(location = 3) in vec2 fragTexCoord1;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform sampler2D baseColorTexture; // TODO 0 is the material descriptor set id for now (will be 2)
layout(set = 0, binding = 1) uniform sampler2D metallicRoughnessTexture; // TODO 0 is the material descriptor set id for now (will be 2)
layout(set = 0, binding = 2) uniform sampler2D normalTexture; // TODO 0 is the material descriptor set id for now (will be 2)


void main() {
    vec3 baseColor = texture(baseColorTexture, fragTexCoord0).rgb;
    float metallicity = texture(metallicRoughnessTexture, fragTexCoord0).b;
    float roughness = texture(metallicRoughnessTexture, fragTexCoord0).g;
    vec3 normal = texture(normalTexture, fragTexCoord0).rgb;

    // TODO proper pbr shading

    outColor = vec4(baseColor, 1.0);
}
