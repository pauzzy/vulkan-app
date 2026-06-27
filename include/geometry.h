#pragma once
#include <glm/glm.hpp>

struct Vertex {
    glm::vec3 position;
    glm::vec4 color;
    glm::vec2 uv;
};

struct RendererData {
	glm::mat4 view;
	glm::mat4 projection;
    float time;
};

void createPlane(
    std::vector<Vertex>& vertices, 
    std::vector<uint32_t>& indices
);