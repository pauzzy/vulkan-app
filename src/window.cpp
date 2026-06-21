#include "window.h"

Window::Window(
    const uint32_t width, 
    const uint32_t height, 
    std::string_view title
) : width(width), height(height)
{
     if (!glfwInit()) {
        std::println("failed initializing glfw!");
        return;
    }
    
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    handle = glfwCreateWindow(
        width, height, 
        title.data(), 
        nullptr, nullptr
    );

    if (!handle) { 
        throw std::runtime_error("failed creating window!");
    }

    input = std::make_unique<Input>(
        handle,
        GLFW_CURSOR_DISABLED
    );
}

void Window::destroyWindow()
{
    glfwDestroyWindow(handle);
    glfwTerminate();
}

void Window::run(
    std::function<void()> updateFunc, 
    std::function<void()> renderFunc
) {
    while (!glfwWindowShouldClose(handle)) {
        double startTime = glfwGetTime();
        glfwPollEvents();
        updateFunc();
        renderFunc();
        deltaTime = glfwGetTime() - startTime;
    }
}

void Window::createWindowSurface(VkInstance instance)
{
    if (glfwCreateWindowSurface(instance, handle, nullptr, &surfaceHandle) != VK_SUCCESS) {
        throw std::runtime_error("failed creating window surface!");
    }
}

void Window::destroyWindowSurface(VkInstance instance)
{
    if (surfaceHandle) vkDestroySurfaceKHR(instance, surfaceHandle, nullptr);
}

void Window::getDimensions(int* width, int* height)
{
    glfwGetWindowSize(handle, width, height);
}

std::vector<const char*> Window::getRequiredInstanceExtensions() const
{    
    uint32_t instanceExtensionsCount = 0;
    const char** glfwInstanceExtensions = glfwGetRequiredInstanceExtensions(&instanceExtensionsCount);

    std::vector<const char*> requiredInstanceExtensions;
    for (size_t i = 0; i < instanceExtensionsCount; ++i) {
        requiredInstanceExtensions.push_back(glfwInstanceExtensions[i]);
    }

    return requiredInstanceExtensions;
}