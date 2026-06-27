#include "texture.h"  

Texture::Texture(Device &device, std::string_view filepath) :
device(device)
{
    // -- load image properties and pixel data
    unsigned char* dataBuffer = nullptr;
    int imageWidth, imageHeight, nrImageChannels;
    if (dataBuffer = stbi_load(
        filepath.data(),
        &imageWidth,
        &imageHeight,
        &nrImageChannels,
        4
    ); !dataBuffer) {
        throw std::runtime_error(std::format("failed reading image: [{}]!", filepath.data()));
    }

    uint32_t dataBufferByteSize = imageWidth * imageHeight * 4;

    // -- create staging buffer

    uint32_t graphicsQueueFamilyIndex = device.getGraphicsQueueFamilyIndex();

    VkBufferCreateInfo stagingBufferCreateInfo {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = dataBufferByteSize,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 1,
        .pQueueFamilyIndices = &graphicsQueueFamilyIndex
    };

    VmaAllocationCreateInfo stagingBufferAllocationCreateInfo {
        .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO
    };

    VmaAllocationInfo stagingBufferAllocationInfo = {}; 

    VkBuffer stagingBufferHandle = nullptr;
    VmaAllocation stagingBufferAllocation;

    vmaCreateBuffer(
        device.getVmaAllocator(),
        &stagingBufferCreateInfo,
        &stagingBufferAllocationCreateInfo,
        &stagingBufferHandle,
        &stagingBufferAllocation,
        &stagingBufferAllocationInfo
    );

    vmaMapMemory(device.getVmaAllocator(), stagingBufferAllocation, &stagingBufferAllocationInfo.pMappedData);
    memcpy(stagingBufferAllocationInfo.pMappedData, dataBuffer, dataBufferByteSize);
    vmaUnmapMemory(device.getVmaAllocator(), stagingBufferAllocation);
    
    stbi_image_free(dataBuffer);

    VkImageLayout imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VkImageCreateInfo imageCreateInfo {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = VK_FORMAT_R8G8B8A8_SRGB,
        .extent = VkExtent3D {
            .width = static_cast<uint32_t>(imageWidth),
            .height = static_cast<uint32_t>(imageHeight),
            .depth = 1
        },
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | 
            VK_IMAGE_USAGE_SAMPLED_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 1,
        .pQueueFamilyIndices = &graphicsQueueFamilyIndex,
        .initialLayout = imageLayout
    };

    VmaAllocationCreateInfo allocationCreateInfo {
        .usage = VMA_MEMORY_USAGE_AUTO
    };

    VmaAllocationInfo imageAllocationInfo = {};

    vmaCreateImage(
        device.getVmaAllocator(),
        &imageCreateInfo,
        &allocationCreateInfo,
        &imageHandle,
        &imageAllocation,
        &imageAllocationInfo
    );

    VkCommandPoolCreateInfo tempCommandPoolCreateInfo {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .queueFamilyIndex = device.getGraphicsQueueFamilyIndex()
    };

    VkCommandPool tempCommandPool = nullptr;
    vkCreateCommandPool(
        device.getLogicalDeviceHandle(), 
        &tempCommandPoolCreateInfo,
        nullptr,
        &tempCommandPool
    );

    VkCommandBufferAllocateInfo tempCommandBufferAllocateInfo {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = tempCommandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1
    };

    VkCommandBuffer tempCommandBuffer = nullptr;
    vkAllocateCommandBuffers(
        device.getLogicalDeviceHandle(),
        &tempCommandBufferAllocateInfo,
        &tempCommandBuffer
    );

    VkCommandBufferBeginInfo tempCommandBufferBeginInfo {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };

    VkBufferImageCopy regions {
        .bufferOffset = 0,
        .imageSubresource = VkImageSubresourceLayers {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .mipLevel = 0,
            .baseArrayLayer = 0,
            .layerCount = 1
        },
        .imageOffset = VkOffset3D {
            .x = 0,
            .y = 0,
            .z = 0
        },
        .imageExtent = VkExtent3D {
            .width = static_cast<uint32_t>(imageWidth), 
            .height = static_cast<uint32_t>(imageHeight),
            .depth = 1
        }
    };

    vkBeginCommandBuffer(tempCommandBuffer, &tempCommandBufferBeginInfo);

    VkImageMemoryBarrier toTransferDst {
        .sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcAccessMask       = 0,
        .dstAccessMask       = VK_ACCESS_TRANSFER_WRITE_BIT,
        .oldLayout           = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout           = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image               = imageHandle,
        .subresourceRange    = {
            .aspectMask      = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel    = 0,
            .levelCount      = 1,
            .baseArrayLayer  = 0,
            .layerCount      = 1
        }
    };
    vkCmdPipelineBarrier(
        tempCommandBuffer,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        0, 0, nullptr, 0, nullptr,
        1, &toTransferDst
    );

    vkCmdCopyBufferToImage(
        tempCommandBuffer, 
        stagingBufferHandle, 
        imageHandle, 
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
        1, 
        &regions
    );

    VkImageMemoryBarrier toShaderRead {
        .sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcAccessMask       = VK_ACCESS_TRANSFER_WRITE_BIT,
        .dstAccessMask       = VK_ACCESS_SHADER_READ_BIT,
        .oldLayout           = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        .newLayout           = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image               = imageHandle,
        .subresourceRange    = {
            .aspectMask      = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel    = 0,
            .levelCount      = 1,
            .baseArrayLayer  = 0,
            .layerCount      = 1
        }
    };
    vkCmdPipelineBarrier(
        tempCommandBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0, 0, nullptr, 0, nullptr,
        1, &toShaderRead
    );

    vkEndCommandBuffer(tempCommandBuffer);

    VkSubmitInfo queueSubmitInfo {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &tempCommandBuffer
    };

    vkQueueSubmit(device.getGraphicsQueue(), 1, &queueSubmitInfo, nullptr);

    vkQueueWaitIdle(device.getGraphicsQueue());

    vkDestroyCommandPool(device.getLogicalDeviceHandle(), tempCommandPool, nullptr);
    vmaDestroyBuffer(device.getVmaAllocator(), stagingBufferHandle, stagingBufferAllocation);

    VkImageViewCreateInfo imageViewCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = imageHandle,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = VK_FORMAT_R8G8B8A8_SRGB,
        .components = VkComponentMapping {
            .r = VK_COMPONENT_SWIZZLE_IDENTITY,
            .g = VK_COMPONENT_SWIZZLE_IDENTITY,
            .b = VK_COMPONENT_SWIZZLE_IDENTITY,
            .a = VK_COMPONENT_SWIZZLE_IDENTITY
        },
        .subresourceRange = VkImageSubresourceRange {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        }
    };

    vkCreateImageView(
        device.getLogicalDeviceHandle(),
        &imageViewCreateInfo,
        nullptr,
        &imageView
    );

    VkSamplerCreateInfo imageSamplerCreateInfo {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter = VK_FILTER_LINEAR,
        .minFilter = VK_FILTER_LINEAR,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE
    };

    vkCreateSampler(
        device.getLogicalDeviceHandle(),
        &imageSamplerCreateInfo,
        nullptr,
        &imageSampler
    );

    
}

Texture::~Texture() {
    vkDestroySampler(
        device.getLogicalDeviceHandle(),
        imageSampler,
        nullptr
    );

    vkDestroyImageView(
        device.getLogicalDeviceHandle(),
        imageView,
        nullptr
    );

    vmaDestroyImage(
        device.getVmaAllocator(),
        imageHandle,
        imageAllocation
    );
}