#include "swapchain.h"

Swapchain::Swapchain(
    Device& device,
    const VkSurfaceKHR      surface, 
    const uint32_t          width, 
    const uint32_t          height
) : device(device)
{
    create(
        surface, 
        width, height
    );
}

Swapchain::~Swapchain()
{
    destroy();
}

void Swapchain::create(
    const VkSurfaceKHR  surface, 
    const uint32_t      width, 
    const uint32_t      height
)
{
    swapchainWidth = width;
    swapchainHeight = height;
    
    VkSurfaceCapabilitiesKHR surfaceCapabilities {};
    if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.getPhysicalDeviceHandle(), surface, &surfaceCapabilities) 
        != VK_SUCCESS) {
        throw std::runtime_error("failed getting physical device surface capabilities!");
    }

    constexpr uint32_t NO_LIMIT = 0;

    uint32_t requestedImageCount = std::max(2u, surfaceCapabilities.minImageCount);
    if (surfaceCapabilities.maxImageCount > NO_LIMIT) {
        requestedImageCount = std::min(requestedImageCount, surfaceCapabilities.maxImageCount);
    }

    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device.getPhysicalDeviceHandle(), surface, &formatCount, nullptr);
    std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device.getPhysicalDeviceHandle(), surface, &formatCount, surfaceFormats.data());
    bool desiredFormatSupported = false;

    for (const VkSurfaceFormatKHR& surfaceFormat : surfaceFormats) {
        if (surfaceFormat.format == swapchainFormat) {
            desiredFormatSupported = true;
            break;
        }
    }

    if (!desiredFormatSupported) {
        throw std::runtime_error("failed due to desired swapchain format not being supported!");
    }

    VkSwapchainCreateInfoKHR swapchainCreateInfo {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = surface,
        .minImageCount = requestedImageCount,
        .imageFormat = swapchainFormat,
        .imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR,
        .imageExtent = VkExtent2D {.width = swapchainWidth, .height = swapchainHeight},
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .preTransform = surfaceCapabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = VK_PRESENT_MODE_FIFO_KHR
    };

    if (vkCreateSwapchainKHR(device.getLogicalDeviceHandle(), &swapchainCreateInfo, nullptr, &swapchainHandle) != VK_SUCCESS) {
        throw std::runtime_error("failed creating swapchain!");
    }

    uint32_t imageCount = 0;
    vkGetSwapchainImagesKHR(device.getLogicalDeviceHandle(), swapchainHandle, &imageCount, nullptr);
    swapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device.getLogicalDeviceHandle(), swapchainHandle, &imageCount, swapchainImages.data());
    swapchainImageViews.resize(imageCount);

    for (size_t i = 0; i < swapchainImageViews.size(); ++i) {
        VkImageViewCreateInfo imageViewCreateInfo {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = swapchainImages[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = swapchainFormat,
            .subresourceRange = VkImageSubresourceRange {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1
            }
        };

        if (vkCreateImageView(device.getLogicalDeviceHandle(), &imageViewCreateInfo, nullptr, &swapchainImageViews[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed creating image view!");
        }
    }

    swapchainRenderCompleteSemaphores.resize(swapchainImages.size());

    for (size_t i = 0; i < swapchainImages.size(); ++i) {
        VkSemaphoreCreateInfo semaphoreCreateInfo {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
        };

        if (vkCreateSemaphore(device.getLogicalDeviceHandle(), &semaphoreCreateInfo, nullptr, &swapchainRenderCompleteSemaphores[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed creating semaphore!");
        }
    }


    VkImageCreateInfo depthImageCreateInfo {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D, 
        .format = depthFormat,
        .extent = VkExtent3D { .width = swapchainWidth, .height = swapchainHeight, .depth = 1 },
        .mipLevels = 1, 
        .arrayLayers = 1, 
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL, 
        .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
    };

    VmaAllocationCreateInfo allocationCreateInfo {
        .flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO
    };

    if (vmaCreateImage(device.getVmaAllocator(), &depthImageCreateInfo, &allocationCreateInfo, &swapchainDepthImage, &swapchainDepthImageAllocation, nullptr) != VK_SUCCESS) {
        throw std::runtime_error("failed creating image!");
    }

    VkImageViewCreateInfo swapchainDepthImageViewCreateInfo {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = swapchainDepthImage,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = depthFormat,
        .subresourceRange = VkImageSubresourceRange {
            .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
            .levelCount = 1,
            .layerCount = 1
        }
    };

    if (vkCreateImageView(device.getLogicalDeviceHandle(), &swapchainDepthImageViewCreateInfo, nullptr, &swapchainDepthImageView) != VK_SUCCESS) {
        throw std::runtime_error("failed creating image view!");
    }
}

void Swapchain::destroy()
{
    for (VkImageView swapchainImageView : swapchainImageViews) {
        vkDestroyImageView(device.getLogicalDeviceHandle(), swapchainImageView, nullptr);
    }
    swapchainImageViews.clear();

    for (VkSemaphore swapchainRenderCompleteSemaphore : swapchainRenderCompleteSemaphores) {
        vkDestroySemaphore(device.getLogicalDeviceHandle(), swapchainRenderCompleteSemaphore, nullptr);
    }
    swapchainRenderCompleteSemaphores.clear();

    if (swapchainHandle) {
        vkDestroySwapchainKHR(device.getLogicalDeviceHandle(), swapchainHandle, nullptr);
        swapchainHandle = nullptr;
    }

    if (swapchainDepthImageView) {
        vkDestroyImageView(device.getLogicalDeviceHandle(), swapchainDepthImageView, nullptr);
        vmaDestroyImage(device.getVmaAllocator(), swapchainDepthImage, swapchainDepthImageAllocation);
        swapchainDepthImageView = nullptr;
    }
}
