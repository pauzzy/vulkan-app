#include "device.h"

Device::Device(const VkInstance instance, const VkSurfaceKHR surface)
{
    // -- find physical device
    uint32_t physicalDeviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);
    std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
    vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.data());

    if (!physicalDeviceCount) {
        throw std::runtime_error("failed finding physical device!");
    }
        
    // TODO: search for best fitting GPU (not doing it right now :>)
    physicalDeviceHandle = physicalDevices[0];
    VkPhysicalDeviceProperties physicalDeviceProperties {};
    vkGetPhysicalDeviceProperties(physicalDeviceHandle, &physicalDeviceProperties);
    std::println("selected [{}] as physical device.", physicalDeviceProperties.deviceName);

    // -- find queue family
    uint32_t queueFamiliesCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties2(physicalDeviceHandle, &queueFamiliesCount, nullptr);
    std::vector<VkQueueFamilyProperties2> queueFamiliesProperties(
        queueFamiliesCount, { .sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2 }
    );
    vkGetPhysicalDeviceQueueFamilyProperties2(physicalDeviceHandle, &queueFamiliesCount, queueFamiliesProperties.data());
    
    bool foundQueueFamilyIndex = false;
    for (size_t queueFamilyIndex = 0; queueFamilyIndex < queueFamiliesProperties.size(); ++queueFamilyIndex) {
        VkBool32 hasPresentSupport = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(physicalDeviceHandle, queueFamilyIndex, surface, &hasPresentSupport);

        const VkQueueFamilyProperties2& familyProperties = queueFamiliesProperties[queueFamilyIndex];
        if (familyProperties.queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT && hasPresentSupport) {
            graphicsQueueFamilyIndex = static_cast<uint32_t>(queueFamilyIndex);
            foundQueueFamilyIndex = true;
            break;
        }
    }

    if (!foundQueueFamilyIndex) {
        throw std::runtime_error("failed finding queue family index");
    }

    // -- create logical device

    VkPhysicalDeviceVulkan14Features supportedFeatures14 {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES, .pNext = nullptr
    };
    VkPhysicalDeviceVulkan13Features supportedFeatures13 {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES, .pNext = &supportedFeatures14
    };
    VkPhysicalDeviceVulkan12Features supportedFeatures12 {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES, .pNext = &supportedFeatures13
    };
    VkPhysicalDeviceFeatures2 supportedFeatures {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, .pNext = &supportedFeatures12
    };
    vkGetPhysicalDeviceFeatures2(physicalDeviceHandle, &supportedFeatures);
    
    if (!supportedFeatures13.synchronization2 || !supportedFeatures13.dynamicRendering ||
        !supportedFeatures12.timelineSemaphore) {
        throw std::runtime_error("physical device doesn't meet the feature requirements!");
    }

    VkPhysicalDeviceVulkan14Features features14 {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES, 
        .pNext = nullptr
    };
    VkPhysicalDeviceVulkan13Features features13 {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES, 
        .pNext = &features14,
        .synchronization2 = VK_TRUE,
        .dynamicRendering = VK_TRUE
    };
    VkPhysicalDeviceVulkan12Features features12 {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES, 
        .pNext = &features13,
        .timelineSemaphore = VK_TRUE
    };
    VkPhysicalDeviceFeatures2 features {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, 
        .pNext = &features12
    };
    
    std::vector<float> queuePriorities { 1.0f };
    VkDeviceQueueCreateInfo graphicsQueueCreateInfo {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = graphicsQueueFamilyIndex,
        .queueCount = 1,
        .pQueuePriorities = queuePriorities.data()
    };

    const std::vector<const char*> logicalDeviceExtensions { 
        VK_KHR_SWAPCHAIN_EXTENSION_NAME 
    };

    VkDeviceCreateInfo logicalDeviceCreateInfo {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = &features,
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &graphicsQueueCreateInfo,
        .enabledExtensionCount = static_cast<uint32_t>(logicalDeviceExtensions.size()),
        .ppEnabledExtensionNames = logicalDeviceExtensions.data(),
        .pEnabledFeatures = nullptr
    };

    if (vkCreateDevice(physicalDeviceHandle, &logicalDeviceCreateInfo, nullptr, &logicalDeviceHandle) != VK_SUCCESS) {
        throw std::runtime_error("failed creating logical device!");
    }

    // -- get graphics queue
    vkGetDeviceQueue(logicalDeviceHandle, graphicsQueueFamilyIndex, 0, &graphicsQueue);

    if (!graphicsQueue) {
        throw std::runtime_error("failed getting graphics queue from logical device!");
    }

    // -- create vulkan memory allocator

    VmaVulkanFunctions vmaFuncInfo {};
    VmaAllocatorCreateInfo vmaAllocatorCreateInfo {
        .physicalDevice = physicalDeviceHandle,
        .device = logicalDeviceHandle,
        .pVulkanFunctions = &vmaFuncInfo,
        .instance = instance,
        .vulkanApiVersion = VULKAN_VERSION
    };

    vmaImportVulkanFunctionsFromVolk(&vmaAllocatorCreateInfo, &vmaFuncInfo);
    
    if (vmaCreateAllocator(&vmaAllocatorCreateInfo, &vmaAllocator) != VK_SUCCESS) {
        throw std::runtime_error("failed initializing vulkan memory allocator!");
    }
}

Device::~Device()
{
    if (vmaAllocator) vmaDestroyAllocator(vmaAllocator);
    if (logicalDeviceHandle) vkDestroyDevice(logicalDeviceHandle, nullptr);
}