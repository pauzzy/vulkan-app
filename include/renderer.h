#pragma once
#include <stdexcept>
#include <array>
#include <vector>
#include <Volk/volk.h>
#include <glm/gtc/matrix_transform.hpp>

#include "window.h"
#include "swapchain.h"
#include "graphics_pipeline.h"
#include "buffer.h"
#include "camera.h"


class Renderer {
public:
    Renderer(
        Window&             window,
        Device&             device,
        Swapchain&          swapchain,
        GraphicsPipeline&   graphicsPipeline
    );
    ~Renderer();

    void update();
    void render();

private:
    void createSyncResources();
    void createCommandBuffers();
    void createUniformBuffers();
    void handleSwapchainRecreation();

    struct FrameResources {
        VkCommandPool commandPool = nullptr;
        VkCommandBuffer commandBuffer = nullptr;
        VkSemaphore imageAcquiredSemaphore = nullptr;
        std::unique_ptr<Buffer<Matrices>> uniformBuffer;
        VkDescriptorSet descriptorSet = nullptr;
    };

    Window& window;
    Device& device;
    Swapchain& swapchain;
    GraphicsPipeline& graphicsPipeline;

    static const size_t MaxFramesInFlight = 2;

    std::array<FrameResources, MaxFramesInFlight> frameResources;
    VkSemaphore timelineSemaphore = nullptr;
    uint64_t frameIndex = 0;
    uint64_t nextSignalValue = MaxFramesInFlight + 1;
    bool requireSwapchainRecreate = false;

    std::unique_ptr<Buffer<Vertex>> vertexBuffer;
    std::unique_ptr<Buffer<uint32_t>> indexBuffer;

    VkDescriptorPool descriptorPool = nullptr;

    // -- logic
    std::unique_ptr<Camera> camera;
    Matrices matrices;
};