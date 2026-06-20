#pragma once
#include <glm/glm.hpp>

struct Vertex {
    glm::vec3 position;
    glm::vec4 color;
};

struct Matrices {
	glm::mat4 view;
	glm::mat4 projection;
};