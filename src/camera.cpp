#include "camera.h"

Camera::Camera(Window& window, float aspectRatio) : window(window)
{
    eye = glm::vec3(0.0f, 0.0f, -2.0f);
    front = glm::vec3(0.0f, 0.0f, 1.0f);
    view = glm::lookAt(eye, eye + front, WORLD_UP);
    setProjection(aspectRatio);
}

void Camera::update()
{
    Input& input = window.getInput();
    glm::vec3 inputVec = glm::vec3(0.0f);
    
    if (input.getKeyPressed(GLFW_KEY_W)) {
        inputVec.z += 1.f;
    }
    if (input.getKeyPressed(GLFW_KEY_A)) {
        inputVec.x += 1.f;
    }
    if (input.getKeyPressed(GLFW_KEY_S)) {
        inputVec.z -= 1.f;
    }
    if (input.getKeyPressed(GLFW_KEY_D)) {
        inputVec.x -= 1.f;
    }
    if (input.getKeyPressed(GLFW_KEY_SPACE)) {
        inputVec.y -= 1.f;
    }
    if (input.getKeyPressed(GLFW_KEY_LEFT_CONTROL)) {
        inputVec.y += 1.f;
    }

    yaw += input.getCursorDelta().x * mouseSens;
    yaw = glm::mod(yaw, 360.f); 
    pitch += input.getCursorDelta().y * mouseSens;
    pitch = glm::clamp(pitch, -60.f, 60.f);

    front = glm::normalize(
        glm::vec3(
            glm::cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
            glm::sin(glm::radians(pitch)),
            glm::sin(glm::radians(yaw)) * cos(glm::radians(pitch))
        )
    );

    glm::vec3 right = glm::cross(WORLD_UP, front);

    if (glm::length(inputVec) > 0) {
        inputVec = glm::normalize(inputVec);

        float speed = input.getKeyPressed(GLFW_KEY_LEFT_SHIFT) ? 
            moveSpeed * moveRunSpeedMultiplier :
            moveSpeed;

        eye += 
            speed * 
            static_cast<float>(window.getDeltaTime()) * 
            (front * inputVec.z + right * inputVec.x + WORLD_UP * inputVec.y);  
    }

    view = glm::lookAt(eye, eye + front, WORLD_UP);
}

void Camera::setProjection(float aspectRatio)
{
    projection = glm::perspective(
        glm::radians(FOV), aspectRatio, Z_NEAR, Z_FAR
    );
}
