#include "buffer.h"
template <typename T>
Buffer<T>::Buffer(Device& device, const std::vector<T>& data, VkBufferUsageFlags usage) : device(device) {
    size_t dataByteSize = sizeof(data[0]) * data.size();

    VkBufferCreateInfo stagingBufferCreateInfo {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = dataByteSize,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };

    VmaAllocationCreateInfo stagingBufferAllocationCreateInfo {
        .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO,
    };

    VkBuffer stagingBuffer = nullptr;
    VmaAllocation stagingBufferAllocation = nullptr;
    VmaAllocationInfo stagingBufferAllocationInfo = {};
    if (vmaCreateBuffer(
        device.getVmaAllocator(),
        &stagingBufferCreateInfo,
        &stagingBufferAllocationCreateInfo,
        &stagingBuffer,
        &stagingBufferAllocation,
        &stagingBufferAllocationInfo
    ) != VK_SUCCESS) {
        throw std::runtime_error("failed creating staging buffer!");
    }

    void* memAddr = nullptr;
    if (vmaMapMemory(device.getVmaAllocator(), stagingBufferAllocation, &memAddr) != VK_SUCCESS) {
        throw std::runtime_error("failed mapping staging buffer memory!");
    }
    memcpy(memAddr, data.data(), dataByteSize);
    vmaUnmapMemory(device.getVmaAllocator(), stagingBufferAllocation);

    VkBufferCreateInfo bufferCreateInfo {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = dataByteSize,
        .usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT |
            usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };
    
    VmaAllocationCreateInfo bufferAllocationCreateInfo {
        .usage = VMA_MEMORY_USAGE_AUTO,
    };

    VmaAllocationInfo bufferAllocationInfo = {};
    if (vmaCreateBuffer(
        device.getVmaAllocator(),
        &bufferCreateInfo,
        &bufferAllocationCreateInfo,
        &bufferHandle,
        &bufferAllocation,
        &bufferAllocationInfo
    ) != VK_SUCCESS) {
        throw std::runtime_error("failed creating buffer!");
    }

    VkCommandPoolCreateInfo commandPoolCreateInfo {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
        .queueFamilyIndex = device.getGraphicsQueueFamilyIndex()
    };

    VkCommandPool commandPool = nullptr;
    if (vkCreateCommandPool(device.getLogicalDeviceHandle(), &commandPoolCreateInfo, nullptr, &commandPool) != VK_SUCCESS) {
        throw std::runtime_error("failed creating command pool!");
    }

    VkCommandBufferAllocateInfo commandBufferAllocateInfo {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1
    };

    VkCommandBuffer commandBuffer = nullptr;
    if (vkAllocateCommandBuffers(device.getLogicalDeviceHandle(), &commandBufferAllocateInfo, &commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed allocating command buffer!");
    }

    VkCommandBufferBeginInfo commandBufferBeginInfo {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, 
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };
    vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);

    VkBufferCopy copyRegion = {
        .srcOffset = 0,
        .dstOffset = 0,
        .size = dataByteSize
    };
    vkCmdCopyBuffer(commandBuffer, stagingBuffer, bufferHandle, 1, &copyRegion);

    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &commandBuffer
    };
    vkQueueSubmit(device.getGraphicsQueue(), 1, &submitInfo, nullptr);

    vkQueueWaitIdle(device.getGraphicsQueue());

    vkDestroyCommandPool(device.getLogicalDeviceHandle(), commandPool, nullptr);    
    vmaDestroyBuffer(device.getVmaAllocator(), stagingBuffer, stagingBufferAllocation);
}

template <typename T>
Buffer<T>::~Buffer() {        
    if (bufferHandle) vmaDestroyBuffer(device.getVmaAllocator(), bufferHandle, bufferAllocation);
}

template <typename T>
uint32_t Buffer<T>::getMemoryTypeIndex(
    VkMemoryRequirements                    memoryRequirements, 
    std::vector<VkMemoryPropertyFlagBits>&  memoryPropertyFlags
) {
    VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties = {};
    vkGetPhysicalDeviceMemoryProperties(device.getPhysicalDeviceHandle(), &physicalDeviceMemoryProperties);

    uint32_t memTypeIndex = 0; 
    for (uint32_t i = 0; i < physicalDeviceMemoryProperties.memoryTypeCount; ++i) {
        if (memoryRequirements.memoryTypeBits & (1 << i)) {
            bool _memTypeSuitable = true;
            for (size_t j = 0; j < memoryPropertyFlags.size(); ++j) {
                if (
                    !(physicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & 
                    memoryPropertyFlags[j])
                ) {
                    _memTypeSuitable = false;
                    break;
                }
            }
            
            if (_memTypeSuitable) {
                memTypeIndex = i;
            }
        }         
    }

    return memTypeIndex;
}

//only for learning reasons
#ifdef NEVER_USED

template <typename T>
void Buffer<T>::createBufferWithoutVMA(std::vector<T>& data)
{
    //  1. create staging-buffer with cpu accessibility

    size_t bufferByteSize = data.size() * sizeof(data[0]);

    VkBufferCreateInfo stagingBufferCreateInfo {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = bufferByteSize,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    VkBuffer stagingBuffer = nullptr;    
    vkCreateBuffer(device.getLogicalDeviceHandle(), &stagingBufferCreateInfo, nullptr, &stagingBuffer);
    
    VkMemoryRequirements stagingBufferMemoryRequirements = {};
    vkGetBufferMemoryRequirements(device.getLogicalDeviceHandle(), stagingBuffer, &stagingBufferMemoryRequirements);
    
    std::vector<VkMemoryPropertyFlagBits> memFlagsStagingBuffer {
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    };
    uint32_t stagingBufferMemTypeIndex = getMemoryTypeIndex(
        stagingBufferMemoryRequirements,
        memFlagsStagingBuffer
    );

    VkMemoryAllocateInfo stagingBufferMemoryAllocateInfo {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = stagingBufferMemoryRequirements.size,
        .memoryTypeIndex = stagingBufferMemTypeIndex
    };

    VkDeviceMemory stagingBufferMemory = nullptr;
    vkAllocateMemory(device.getLogicalDeviceHandle(), &stagingBufferMemoryAllocateInfo, nullptr, &stagingBufferMemory);

    vkBindBufferMemory(device.getLogicalDeviceHandle(), stagingBuffer, stagingBufferMemory, 0);

    //  2. transfer data to staging-buffer

    void* memAddr = nullptr;
    vkMapMemory(
        device.getLogicalDeviceHandle(),
        stagingBufferMemory,
        0, 
        stagingBufferMemoryRequirements.size,
        0,
        &memAddr
    );

    memcpy(memAddr, data.data(), bufferByteSize);
    
    vkUnmapMemory(device.getLogicalDeviceHandle(), stagingBufferMemory);

    //  3. create real-buffer with gpu accessibility
    
    VkBufferCreateInfo bufferCreateInfo {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = bufferByteSize,
        .usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | 
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };

    vkCreateBuffer(device.getLogicalDeviceHandle(), &bufferCreateInfo, nullptr, &bufferHandle);

    VkMemoryRequirements bufferMemoryRequirements = {};
    vkGetBufferMemoryRequirements(device.getLogicalDeviceHandle(), bufferHandle, &bufferMemoryRequirements);

    std::vector<VkMemoryPropertyFlagBits> memFlagsBuffer {
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    };

    uint32_t bufferMemTypeIndex = getMemoryTypeIndex(
        bufferMemoryRequirements,
        memFlagsBuffer
    );

    VkMemoryAllocateInfo bufferMemoryAllocateInfo {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = bufferMemoryRequirements.size,
        .memoryTypeIndex = bufferMemTypeIndex
    };

    vkAllocateMemory(device.getLogicalDeviceHandle(), &bufferMemoryAllocateInfo, nullptr, &bufferMemoryHandle);

    vkBindBufferMemory(device.getLogicalDeviceHandle(), bufferHandle, bufferMemoryHandle, 0);

    //  4. transfer data from staging-buffer to real-buffer
    
    VkCommandPoolCreateInfo commandPoolCreateInfo {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
        .queueFamilyIndex = device.getGraphicsQueueFamilyIndex()
    };

    VkCommandPool commandPool = nullptr;
    vkCreateCommandPool(device.getLogicalDeviceHandle(), &commandPoolCreateInfo, nullptr, &commandPool);

    VkCommandBufferAllocateInfo commandBufferAllocateInfo {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1
    };

    VkCommandBuffer commandBuffer = nullptr;
    vkAllocateCommandBuffers(device.getLogicalDeviceHandle(), &commandBufferAllocateInfo, &commandBuffer);

    VkCommandBufferBeginInfo commandBufferBeginInfo {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, 
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };
    vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);

    VkBufferCopy copyRegion = {
        .srcOffset = 0,
        .dstOffset = 0,
        .size = bufferByteSize
    };
    vkCmdCopyBuffer(commandBuffer, stagingBuffer, bufferHandle, 1, &copyRegion);

    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &commandBuffer
    };
    vkQueueSubmit(device.getGraphicsQueue(), 1, &submitInfo, nullptr);

    vkQueueWaitIdle(device.getGraphicsQueue());

    // 5. cleanup temporary resources
    vkDestroyCommandPool(device.getLogicalDeviceHandle(), commandPool, nullptr);
    vkFreeMemory(device.getLogicalDeviceHandle(), stagingBufferMemory, nullptr);
    vkDestroyBuffer(device.getLogicalDeviceHandle(), stagingBuffer, nullptr);
}

#endif