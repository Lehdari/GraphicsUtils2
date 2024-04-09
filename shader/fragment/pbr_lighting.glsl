#version 450
#pragma shader_stage(fragment)


layout(location = 0) in vec2 fragTexCoord0;

layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform sampler2D gBufferBaseColor;
layout(set = 1, binding = 1) uniform sampler2D gBufferNormal;


void main() {
    vec3 baseColor = texture(gBufferBaseColor, fragTexCoord0).rgb;
    vec3 normal = texture(gBufferNormal, fragTexCoord0).rgb;

    float light = dot(normal, normalize(vec3(0.2, 0.7, 0.3)))*0.5 + 0.5;

    outColor = vec4(light*baseColor, 1.0);
}
