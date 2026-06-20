#version 460

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec4 inColor;

layout (location = 0) out vec4 outColor;

layout (push_constant) uniform Push {
    mat4 model;
} push;

layout (set = 0, binding = 0) uniform Matrices {
    mat4 view;
    mat4 projection;
} matrices;

void main() {
    outColor = inColor;
    gl_Position = matrices.projection * matrices.view * push.model * vec4(inPosition, 1.0);
}