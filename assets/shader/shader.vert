#version 460

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec4 inColor;
layout (location = 2) in vec2 inUV;

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec3 outNormal;
layout (location = 2) out vec2 outUV;

layout (push_constant) uniform Push {
    mat4 model;
} push;

layout (set = 0, binding = 0) uniform RendererData {
    mat4 view;
    mat4 projection;
    float time;
} rendererData;

void main() {
    outColor = inColor;
    outUV = inUV;

    vec3 position = inPosition;

    position.y = sin(rendererData.time + position.x);

    mat3 normalMatrix = transpose(inverse(mat3(push.model)));
    
    outNormal = normalize(
        normalMatrix * 
        vec3(
            -(cos(rendererData.time + position.x)), 
            1.0f, 
            0
        )
    );
        
    gl_Position = 
        rendererData.projection * 
        rendererData.view * 
        push.model * 
        vec4(position, 1.0f);
}