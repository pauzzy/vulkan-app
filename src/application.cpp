#include "application.h"

Application::Application(const uint32_t width, const uint32_t height, std::string_view title)
{
    window = std::make_unique<Window>(width, height, title);
    initVulkan(width, height);
}

Application::~Application()
{
    vkDeviceWaitIdle(device->getLogicalDeviceHandle());
    renderer.reset();
    graphicsPipeline.reset();
    swapchain.reset();
    device.reset();
    window->destroyWindowSurface(instance->getHandle());
    instance.reset();
    window.reset();
}

void Application::initVulkan(const uint32_t width, const uint32_t height) {
    instance = std::make_unique<Instance>(
        window.get()
    );
    window->createWindowSurface(
        instance->getHandle()
    );
    device = std::make_unique<Device>(
        instance->getHandle(), 
        window->getSurfaceHandle()
    );
    swapchain = std::make_unique<Swapchain>(
        *device,
        window->getSurfaceHandle(),
        width, height
    );
    graphicsPipeline = std::make_unique<GraphicsPipeline>(
        *device,
        *swapchain
    );
    renderer = std::make_unique<Renderer>(
        *window,
        *device,
        *swapchain,
        *graphicsPipeline
    );
}

void Application::run()
{
    window->run(
        [this]() {renderer->update();},
        [this]() {renderer->render();}
    );
}