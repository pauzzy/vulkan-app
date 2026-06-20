#pragma once
#include <stdexcept>
#include <vector>
#include <Volk/volk.h>
#include <vma/vk_mem_alloc.h>

#include "device.h"

class Swapchain {
public:
    Swapchain(
        Device& device,
        const VkSurfaceKHR      surface, 
        const uint32_t          width, 
        const uint32_t          height
    );
    ~Swapchain();

    void create(
        const VkSurfaceKHR      surface, 
        const uint32_t          width, 
        const uint32_t          height
    );
    void destroy();

    VkSwapchainKHR getSwapchainHandle() const { return swapchainHandle; }
    std::vector<VkImage>& getSwapchainImages() { return swapchainImages; }
    std::vector<VkImageView>& getSwapchainImageViews() { return swapchainImageViews; }
    VkImage getSwapchainDepthImage() { return swapchainDepthImage; }
    VkImageView getSwapchainDepthImageView() { return swapchainDepthImageView; }
    uint32_t getSwapchainWidth() const { return swapchainWidth; }
    uint32_t getSwapchainHeight() const { return swapchainHeight; }
    std::vector<VkSemaphore>& getSwapchainRenderCompleteSemaphores() { return swapchainRenderCompleteSemaphores; }
    VkFormat getSwapchainFormat() const { return swapchainFormat; }
    VkFormat getSwapchainDepthFormat() const { return depthFormat; }
    
private:
    VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;
    VkFormat swapchainFormat = VK_FORMAT_B8G8R8A8_SRGB;

    Device& device;

    VkSwapchainKHR swapchainHandle = nullptr;
    std::vector<VkImage> swapchainImages;
    std::vector<VkImageView> swapchainImageViews;
    std::vector<VkSemaphore> swapchainRenderCompleteSemaphores;
    uint32_t swapchainWidth = 0;
    uint32_t swapchainHeight = 0;
    VkImage swapchainDepthImage = nullptr;
    VmaAllocation swapchainDepthImageAllocation = nullptr;
    VkImageView swapchainDepthImageView = nullptr;
};