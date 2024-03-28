#version 450
#pragma shader_stage(vertex)

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec4 inTangent;
layout(location = 3) in vec2 inTexCoord0;
#ifndef DISABLE_IN_TEX_COORD_1
layout(location = 4) in vec2 inTexCoord1;
#endif

layout(location = 0) out vec3 fragNormal;
layout(location = 1) out vec4 fragTangent;
layout(location = 2) out vec2 fragTexCoord0;
#ifndef DISABLE_IN_TEX_COORD_1
layout(location = 3) out vec2 fragTexCoord1;
#endif

layout(set = 1, binding = 0) uniform UniformBufferObject { // TODO 1 is the object descriptor set id for now (will be 3)
    mat4 model;
    mat4 view;
    mat4 projection;
} ubo;

void main() {
    gl_Position = ubo.projection * ubo.view * ubo.model * vec4(inPosition, 1.0);
    fragNormal = (ubo.model*vec4(inNormal, 0.0)).xyz;
    fragTangent = inTangent;
    fragTexCoord0 = inTexCoord0;
    #ifndef DISABLE_IN_TEX_COORD_1
    fragTexCoord1 = inTexCoord1;
    #endif
}
