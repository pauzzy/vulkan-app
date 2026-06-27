#pragma once
#include <stdexcept>
#include <string_view>
#include <Volk/volk.h>
#include <vma/vk_mem_alloc.h>
#include <stb_image.h>

#include "device.h"

class Texture {
public:
    Texture(Device& device, std::string_view filepath);
    ~Texture();

    VkImage getImageHandle() const { return imageHandle; }
    VkImageView getImageView() const { return imageView; }
    VkSampler getImageSampler() const { return imageSampler; }

private:
    Device& device;

    VkImage imageHandle = nullptr;
    VkImageView imageView = nullptr;
    VmaAllocation imageAllocation = nullptr;
    VkSampler imageSampler = nullptr;
};