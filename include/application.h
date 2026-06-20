#pragma once
#include <memory>

#include "window.h"
#include "instance.h"
#include "renderer.h"

class Application {
public:
    Application(
        const uint32_t      width, 
        const uint32_t      height, 
        std::string_view    title
    );
    ~Application();

    void run();

private:
    void initVulkan(
        const uint32_t width, 
        const uint32_t height
    );

    std::unique_ptr<Window> window;

    std::unique_ptr<Instance> instance;
    std::unique_ptr<Device> device;
    std::unique_ptr<Swapchain> swapchain;
    std::unique_ptr<GraphicsPipeline> graphicsPipeline;
    std::unique_ptr<Renderer> renderer;
};