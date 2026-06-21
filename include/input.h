#pragma once
#include <print>
#include <array>
#include <vector>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class Input {
public:
    Input(GLFWwindow* window, int cursorMode);

    void resetState();

    void setKeyPressed(int key, bool toggle) { 
        keysPressed[key] = toggle; 
        if (toggle) {
            keysClicked.push_back(key);
        } 
    }

    bool getKeyPressed(int key) const { return keysPressed[key]; }

    void setCursorDelta(double deltaX, double deltaY) { cursorDelta.x += deltaX; cursorDelta.y += deltaY; }
    glm::vec2 getCursorDelta() const { return cursorDelta; }

private:
    static void keyCallback(
        GLFWwindow* window, 
        int key, 
        int scancode, 
        int action, 
        int mods
    );

    static void cursorCallback(
        GLFWwindow* window, 
        double xpos, 
        double ypos
    );

    std::array<bool, static_cast<size_t>(GLFW_KEY_LAST + 1)> keysPressed = {};
    std::vector<int> keysClicked = {};

    glm::vec2 cursorDelta;
};