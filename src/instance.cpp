#include "instance.h"

Instance::Instance(const Window* window) {
    if (volkInitialize() != VK_SUCCESS) {
        throw std::runtime_error("failed initializing volk!");
    }

    VkApplicationInfo appInfo {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "app",
        .apiVersion = VULKAN_VERSION
    };

    std::vector<const char*> requestedLayer {
        "VK_LAYER_KHRONOS_validation"
    };


    std::vector<const char*> requiredInstanceExtensions = window->getRequiredInstanceExtensions(); 
    requiredInstanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    VkDebugUtilsMessengerCreateInfoEXT debugInfo {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = debugCallback
    };

    VkInstanceCreateInfo instanceCreateInfo {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO, 
        .pNext = &debugInfo,
        .pApplicationInfo = &appInfo,
        .enabledLayerCount = static_cast<uint32_t>(requestedLayer.size()),
        .ppEnabledLayerNames = requestedLayer.data(),
        .enabledExtensionCount = static_cast<uint32_t>(requiredInstanceExtensions.size()),
        .ppEnabledExtensionNames = requiredInstanceExtensions.data(),
    };

    if (vkCreateInstance(&instanceCreateInfo, nullptr, &handle) != VK_SUCCESS) {
        throw std::runtime_error("failed creating vulkan instance!");
    }

    volkLoadInstance(handle);
}

Instance::~Instance()
{
    if (handle) vkDestroyInstance(handle, nullptr);
    volkFinalize();
}

VKAPI_ATTR VkBool32 VKAPI_CALL Instance::debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT      severity, 
    VkDebugUtilsMessageTypeFlagsEXT             type, 
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, 
    void*                                       pUserData
)
{
    if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        std::println("validation layer: {}", pCallbackData->pMessage);
    }

    return VK_FALSE;
}