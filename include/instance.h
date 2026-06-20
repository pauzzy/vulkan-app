#pragma once
#include <print>
#include <vector>
#include <vulkan/vulkan.h>
#include <Volk/volk.h>
#include "window.h"


class Instance {
public:
    Instance(const Window* window);
    ~Instance();

    VkInstance getHandle() const {return handle;}
private:
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT      severity,
        VkDebugUtilsMessageTypeFlagsEXT             type,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void *                                      pUserData
    );
    
    VkInstance handle;
};