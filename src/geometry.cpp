#include "geometry.h"

void createPlane(
    std::vector<Vertex> &vertices, 
    std::vector<uint32_t> &indices
) {
    vertices.insert(vertices.end(), {
        Vertex {
            .position = glm::vec3( 0.0f, -0.5f, 0.0f),
            .color = glm::vec4( 1.0f, 0.0f, 0.0f, 1.0f )
        },
        Vertex {
            .position = glm::vec3(-0.5f,  0.5f, 0.0f),
            .color = glm::vec4( 0.0f, 1.0f, 0.0f, 1.0f )
        },
        Vertex {
            .position = glm::vec3( 0.5f,  0.5f, 0.0f),
            .color = glm::vec4( 0.0f, 0.0f, 1.0f, 0.0f )
        }
    });

    indices.insert(indices.end(), {
        0, 1, 2
    });
}
