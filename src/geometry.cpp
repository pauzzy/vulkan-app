#include "geometry.h"

void createPlane(
    std::vector<Vertex> &vertices, 
    std::vector<uint32_t> &indices
) {

    const uint32_t SIZE = 20;
    const uint32_t HALF_SIZE = SIZE / 2;
    const float DENSITY = 2.0f;
    const float STEP = 1.0f/DENSITY;

    for (float z = 0; z <= SIZE; z += STEP) {
        for (float x = 0; x <= SIZE; x += STEP) {
            vertices.push_back(
                Vertex {
                    .position = glm::vec3(
                        x - HALF_SIZE, 
                        0, 
                        z - HALF_SIZE
                    ),
                    .color = glm::vec4(
                        .2f, .2f, .8f, 1.0f
                    ),
                    .uv = glm::vec2(
                        x/SIZE, z/SIZE
                    )
                }
            );
        }
    }

    const uint32_t ROW = (uint32_t)(SIZE * DENSITY) + 1;
    uint32_t i = 0;
    for (float z = 0; z < SIZE; z += STEP) {
        for (float x = 0; x < SIZE; x += STEP) {
            indices.push_back(i);
            indices.push_back(i + 1);
            indices.push_back(i + ROW);

            indices.push_back(i + 1);
            indices.push_back(i + ROW);
            indices.push_back(i + ROW + 1);
            
            i++;
        }
        i++;
    }
}
