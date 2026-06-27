#version 460

layout (location = 0) out vec4 fragColor;

layout (location = 0) in vec4 inColor;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;

layout (set = 1, binding = 0) uniform sampler2D texSampler;

void main() {
    vec3 lightDirection = vec3(0.0f, -1.0f, 0.0f);
    float light = max(dot(inNormal, -lightDirection), 0.0f);
    vec4 texColor = texture(texSampler, inUV);
    fragColor = vec4(light * texColor.rgb, 1.0f);
}