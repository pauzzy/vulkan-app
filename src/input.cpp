#include "input.h"

Input::Input(GLFWwindow* window, int cursorMode)
{
    glfwSetInputMode(window, GLFW_CURSOR, cursorMode);
    glfwSetWindowUserPointer(window, this);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetCursorPosCallback(window, cursorCallback);
}

void Input::resetState()
{
    keysClicked.clear();
    cursorDelta = glm::vec2(0.f);
}

void Input::keyCallback(
    GLFWwindow *window,
    int key, 
    int scancode,
    int action, 
    int mods
) {
    Input* input = static_cast<Input*>(
        glfwGetWindowUserPointer(window)
    );

    if (action == GLFW_PRESS) {
        input->setKeyPressed(key, true);
    }

    if (action == GLFW_RELEASE) {
        input->setKeyPressed(key, false);
    }
}

void Input::cursorCallback(
    GLFWwindow *window, 
    double xpos, 
    double ypos
) {
    Input* input = static_cast<Input*>(
        glfwGetWindowUserPointer(window)
    );

    static bool firstCursor = true;
    static double lastCursorX;
    static double lastCursorY;

    if (firstCursor) {
        lastCursorX = xpos;
        lastCursorY = ypos;
        firstCursor = false;
        return;
    }

    input->setCursorDelta(
        xpos - lastCursorX, 
        ypos - lastCursorY
    );
    
    lastCursorX = xpos;
    lastCursorY = ypos;
}
