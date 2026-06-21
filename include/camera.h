#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "window.h"

class Camera {
public:
    Camera(Window& window, float aspectRatio);
    
    void update();

    void setProjection(float aspectRatio);
    glm::mat4 getProjection() const { return projection; }
    glm::mat4 getView() const { return view; }

private:
    Window& window;

    const glm::vec3 WORLD_UP = glm::vec3(0.0f, 1.0f, 0.0f);
    const float FOV = 60.0f;
    const float Z_NEAR = 0.1f;
    const float Z_FAR = 100.0f;

    float moveSpeed = 10.f;
    float mouseSens = .1f;

    float yaw = 90.0f;
    float pitch = 0.0f;

    glm::mat4 projection;
    glm::mat4 view;

    glm::vec3 eye;
    glm::vec3 front;
};