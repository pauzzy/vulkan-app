#version 460

layout (location = 0) out vec4 fragColor;

layout (location = 0) in vec4 inColor;

void main() {
    fragColor = vec4(inColor);
}