#version 450
#pragma shader_stage(fragment)

layout(location = 0) in vec3 fragNormal;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D texSampler;


void main() {
    outColor = 0.5 + 0.5*vec4(fragNormal, 1.0);
}
