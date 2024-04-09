#version 450
#pragma shader_stage(vertex)


layout(location = 0) in vec2 inPosition;

layout(location = 0) out vec2 fragTexCoord0;


void main() {
    gl_Position = vec4(inPosition, 0.0, 1.0);
    fragTexCoord0 = inPosition*0.5 + 0.5;
}
