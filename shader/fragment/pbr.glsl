#version 450
#pragma shader_stage(fragment)

layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec4 fragTangent;
layout(location = 3) in vec2 fragTexCoord1;
layout(location = 2) in vec2 fragTexCoord0;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D texSampler;


void main() {
    outColor = 0.5 + 0.5*vec4(fragNormal, 1.0);
}
