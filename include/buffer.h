#pragma once
#include <stdexcept>
#include <Volk/volk.h>
#include <vma/vk_mem_alloc.h>
#include <glm/glm.hpp>

#include "geometry.h"
#include "device.h"


template <typename T>
class Buffer {
public:
    Buffer(
        Device& device, 
        const std::vector<T>& data, 
        VkBufferUsageFlags usage, 
        bool persistentMapping
    );
    
    ~Buffer();

    VkBuffer getBufferHandle() const { return bufferHandle; }
    void* getMappedMemAddr() const { return mappedMemAddr; }

private:
    uint32_t getMemoryTypeIndex(
        VkMemoryRequirements                    memoryRequirements, 
        std::vector<VkMemoryPropertyFlagBits>&  memoryPropertyFlags
    );

    void createDeviceLocalBuffer(const std::vector<T>& data, VkBufferUsageFlags usage);
    void createPersistentMappedBuffer(const std::vector<T>& data, VkBufferUsageFlags usage);

    void createBufferWithoutVMA(std::vector<T>& data);

    
    Device& device;    

    VkBuffer bufferHandle = nullptr;
    VmaAllocation bufferAllocation = nullptr;
    void* mappedMemAddr = nullptr;
};

#include "buffer.tpp"