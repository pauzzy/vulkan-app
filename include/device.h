#pragma once
#include <print>
#include <vector>
#include <Volk/volk.h>
#include <vma/vk_mem_alloc.h>

class Device {
public:
    Device(const VkInstance instance, const VkSurfaceKHR surface);
    ~Device();

    VkPhysicalDevice getPhysicalDeviceHandle() const { return physicalDeviceHandle; }
    VkDevice getLogicalDeviceHandle() const { return logicalDeviceHandle; }
    VmaAllocator getVmaAllocator() const { return vmaAllocator; }
    uint32_t getGraphicsQueueFamilyIndex() const { return graphicsQueueFamilyIndex; }
    VkQueue getGraphicsQueue() const { return graphicsQueue; }
    
private:
    VkPhysicalDevice physicalDeviceHandle = nullptr;
    VkDevice logicalDeviceHandle = nullptr;
    uint32_t graphicsQueueFamilyIndex = 0;
    VkQueue graphicsQueue = nullptr;
    VmaAllocator vmaAllocator = nullptr;
};