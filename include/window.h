#pragma once
#include <string_view>
#include <print>
#include <vector>
#include <functional>
#include <Volk/volk.h>
#include <GLFW/glfw3.h>

#include "input.h"

class Window {
public:
    Window(
        const uint32_t      width, 
        const uint32_t      height, 
        std::string_view    title
    );
    void destroyWindow();

    void run(
        std::function<void()> updateFunc, 
        std::function<void()> renderFunc
    );

    void createWindowSurface(VkInstance instance);
    void destroyWindowSurface(VkInstance instance);
    void getDimensions(int* width, int* height);

    std::vector<const char*> getRequiredInstanceExtensions() const;

    GLFWwindow* getHandle() const { return handle; }
    VkSurfaceKHR getSurfaceHandle() const { return surfaceHandle; } 
    double getDeltaTime() const { return deltaTime; }
    Input& getInput() const { return *input; }

private:
    uint32_t width = 0;
    uint32_t height = 0;
    GLFWwindow* handle = nullptr;
    VkSurfaceKHR surfaceHandle = nullptr;
    double deltaTime = 0;

    std::unique_ptr<Input> input;
};