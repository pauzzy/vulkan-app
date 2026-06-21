#include "renderer.h"

std::vector<Vertex> vertices;
std::vector<uint32_t> indices;

glm::mat4 model;

Renderer::Renderer(Window& window, Device &device, Swapchain &swapchain, GraphicsPipeline &graphicsPipeline) :
window(window), device(device), swapchain(swapchain), graphicsPipeline(graphicsPipeline)
{
	createPlane(vertices, indices);

	
	float aspectRatio = 
		static_cast<float>(swapchain.getSwapchainWidth()) / swapchain.getSwapchainHeight();

	camera = std::make_unique<Camera>(window, aspectRatio);

	matrices = Matrices {
		.view = camera->getView(),
		.projection = camera->getProjection()
	};

    createSyncResources();
    createCommandBuffers();
	createUniformBuffers();

	vertexBuffer = std::make_unique<Buffer<Vertex>>(
		device,
		vertices,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		false
	);

	indexBuffer = std::make_unique<Buffer<uint32_t>>(
		device,
		indices,
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		false
	);

	model = glm::mat4(1.0f);
}

Renderer::~Renderer()
{
	indexBuffer.reset();
	vertexBuffer.reset();
	
	if (descriptorPool) vkDestroyDescriptorPool(device.getLogicalDeviceHandle(), descriptorPool, nullptr);
	
	if (timelineSemaphore) vkDestroySemaphore(device.getLogicalDeviceHandle(), timelineSemaphore, nullptr);
    for (FrameResources& res : frameResources) {
        res.uniformBuffer.reset();
		vkDestroySemaphore(device.getLogicalDeviceHandle(), res.imageAcquiredSemaphore, nullptr);
        vkDestroyCommandPool(device.getLogicalDeviceHandle(), res.commandPool, nullptr);
    }
}

void Renderer::createSyncResources()
{
    VkSemaphoreTypeCreateInfo semaphoreTypeCreateInfo {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
        .semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
        .initialValue = static_cast<uint64_t>(MaxFramesInFlight)
    };

    VkSemaphoreCreateInfo semaphoreCreateInfo {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = &semaphoreTypeCreateInfo
    };

    if (vkCreateSemaphore(device.getLogicalDeviceHandle(), &semaphoreCreateInfo, nullptr, &timelineSemaphore) != VK_SUCCESS) {
        throw std::runtime_error("failed creating timeline semaphore!");
    }

    for (FrameResources &res : frameResources) {
        VkSemaphoreCreateInfo semaphoreCreateInfo {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
        };

        if (vkCreateSemaphore(device.getLogicalDeviceHandle(), &semaphoreCreateInfo, nullptr, &res.imageAcquiredSemaphore) != VK_SUCCESS) {
            throw std::runtime_error("failed creating per-frame image-acquire semaphore!");
        }


    }
}

void Renderer::createCommandBuffers()
{
    for (FrameResources &res : frameResources) { 
        VkCommandPoolCreateInfo commandPoolCreateInfo {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .queueFamilyIndex = device.getGraphicsQueueFamilyIndex()
        };
        
        if (vkCreateCommandPool(device.getLogicalDeviceHandle(), &commandPoolCreateInfo, nullptr, &res.commandPool) != VK_SUCCESS) {
            throw std::runtime_error("failed creating command pool!");
        } 

        VkCommandBufferAllocateInfo commandBufferAllocateInfo {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = res.commandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1
        };

        if (vkAllocateCommandBuffers(device.getLogicalDeviceHandle(), &commandBufferAllocateInfo, &res.commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed creating command buffers!");
        }
    }
}

void Renderer::createUniformBuffers()
{
 	std::vector<Matrices> matricesVec {
		matrices
	};

	std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings = {
		VkDescriptorSetLayoutBinding {
			.binding = 0,
			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_VERTEX_BIT
		}
	};
	
	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.bindingCount = 1, 
		.pBindings = descriptorSetLayoutBindings.data()
	};

	VkDescriptorSetLayout descriptorSetLayout = nullptr;
	vkCreateDescriptorSetLayout(
		device.getLogicalDeviceHandle(), 
		&descriptorSetLayoutCreateInfo, 
		nullptr,  
		&descriptorSetLayout
	);

	VkDescriptorPoolSize descriptorPoolSize {
		.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		.descriptorCount = MaxFramesInFlight
	};

	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.maxSets = MaxFramesInFlight, 
		.poolSizeCount = 1,
		.pPoolSizes = &descriptorPoolSize
	};

	vkCreateDescriptorPool(
		device.getLogicalDeviceHandle(),
		&descriptorPoolCreateInfo,
		nullptr,
		&descriptorPool
	);

	for (FrameResources &res : frameResources) { 
		res.uniformBuffer = std::make_unique<Buffer<Matrices>>(
			device,
			matricesVec,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			true
		);

		VkDescriptorSetAllocateInfo descriptorSetAllocateInfo {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.descriptorPool = descriptorPool,
			.descriptorSetCount = 1,
			.pSetLayouts = &descriptorSetLayout
		};

		vkAllocateDescriptorSets(
			device.getLogicalDeviceHandle(), 
			&descriptorSetAllocateInfo,
			&res.descriptorSet
		);

		VkDescriptorBufferInfo descriptorBufferInfo {
			.buffer = res.uniformBuffer->getBufferHandle(),
			.offset = 0,
			.range = sizeof(Matrices)
		};

		VkWriteDescriptorSet writeDescriptorSet {
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, 
			.dstSet = res.descriptorSet,
			.dstBinding = 0,
			.dstArrayElement = 0,
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.pBufferInfo = &descriptorBufferInfo
		};

		vkUpdateDescriptorSets(
			device.getLogicalDeviceHandle(),
			1,
			&writeDescriptorSet,
			0,
			nullptr
		);
	}

	vkDestroyDescriptorSetLayout(
		device.getLogicalDeviceHandle(),
		descriptorSetLayout,
		nullptr
	);	
}

void Renderer::handleSwapchainRecreation()
{
	if (requireSwapchainRecreate) {
        vkDeviceWaitIdle(device.getLogicalDeviceHandle());
		swapchain.destroy();

		int _width = 0, _height = 0;

		do {
			glfwWaitEvents();
			window.getDimensions(&_width, &_height);
		}
		while (_width == 0 || _height == 0);

		swapchain.create(window.getSurfaceHandle(), _width, _height);
			
		float aspectRatio = 
			static_cast<float>(swapchain.getSwapchainWidth()) / swapchain.getSwapchainHeight();
		camera->setProjection(aspectRatio);
		matrices.projection = camera->getProjection();

        requireSwapchainRecreate = false;
    }
}

void Renderer::update() {
	camera->update();
	matrices.view = camera->getView();

	window.getInput().resetState();
}

void Renderer::render()
{
	handleSwapchainRecreation();

    const uint32_t frameResIndex = frameIndex++ % MaxFramesInFlight;
    const uint64_t signalValue = nextSignalValue++;
    const uint64_t waitValue = signalValue - MaxFramesInFlight;

    VkSemaphoreWaitInfo waitInfo {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
        .semaphoreCount = 1,
        .pSemaphores = &timelineSemaphore,
        .pValues = &waitValue
    };
	// Host waits for the Device to signal the wait value
    vkWaitSemaphores(device.getLogicalDeviceHandle(), &waitInfo, UINT64_MAX);

    FrameResources& res = frameResources[frameResIndex];
    vkResetCommandPool(device.getLogicalDeviceHandle(), res.commandPool, 0);

    VkSemaphore imageAcquireSemaphore = frameResources[frameResIndex].imageAcquiredSemaphore;
    uint32_t imageIndex = 0;
    VkResult acquireResult = vkAcquireNextImageKHR(device.getLogicalDeviceHandle(), swapchain.getSwapchainHandle(), UINT64_MAX, imageAcquireSemaphore, VK_NULL_HANDLE, &imageIndex);
    
    if (acquireResult == VK_ERROR_OUT_OF_DATE_KHR) {
        requireSwapchainRecreate = true;
        return;
    }
    else if (acquireResult == VK_SUBOPTIMAL_KHR) {
		requireSwapchainRecreate = true;
    }

    VkCommandBufferBeginInfo commandBufferBeginInfo {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };
    vkBeginCommandBuffer(res.commandBuffer, &commandBufferBeginInfo);

    std::vector<VkImageMemoryBarrier2> layoutBarriers {
        VkImageMemoryBarrier2 {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            .srcAccessMask = 0,
            .dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .image = swapchain.getSwapchainImages()[imageIndex],
            .subresourceRange = VkImageSubresourceRange {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1
            }
        },
        VkImageMemoryBarrier2 {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
			.srcStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,
			.srcAccessMask = 0,
			.dstStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT, // both specified to control memory access at both stages (write)
			.dstAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.newLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
			.image = swapchain.getSwapchainDepthImage(),
			.subresourceRange
			{
				.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1,
			}
        }
    };

    VkDependencyInfo depInfo
	{
		.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
		.imageMemoryBarrierCount = static_cast<uint32_t>(layoutBarriers.size()),
		.pImageMemoryBarriers = layoutBarriers.data()
	};
    vkCmdPipelineBarrier2(res.commandBuffer, &depInfo);

    VkRenderingAttachmentInfo colorAttachInfo
	{
		.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
		.imageView = swapchain.getSwapchainImageViews()[imageIndex],
		.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.clearValue{.color{0.01f, 0.01f, 0.01f, 1}}
	};
	VkRenderingAttachmentInfo depthAttachInfo
	{
		.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
		.imageView = swapchain.getSwapchainDepthImageView(),
		.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.clearValue{.depthStencil{1.0f, 0}}
	};

    VkRenderingInfo renderingInfo
	{
		.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
		.renderArea
		{
			.offset{.x = 0, .y = 0},
			.extent{.width = swapchain.getSwapchainWidth(), .height = swapchain.getSwapchainHeight()}
		},
		.layerCount = 1,
		.colorAttachmentCount = 1,
		.pColorAttachments = &colorAttachInfo,
		.pDepthAttachment = &depthAttachInfo
	};

	vkCmdBeginRendering(res.commandBuffer, &renderingInfo);

    {
		VkViewport viewport
		{
			.x = 0, .y = 0,
			.width = static_cast<float>(swapchain.getSwapchainWidth()),
			.height = static_cast<float>(swapchain.getSwapchainHeight())
		};
		vkCmdSetViewport(res.commandBuffer, 0, 1, &viewport);

		VkRect2D scissor
		{
			.offset{.x = 0, .y = 0 },
			.extent{.width = swapchain.getSwapchainWidth(), .height = swapchain.getSwapchainHeight()}
		};
		vkCmdSetScissor(res.commandBuffer, 0, 1, &scissor);

		vkCmdBindPipeline(res.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline.getGraphicsPipelineHandle());

		memcpy(res.uniformBuffer->getMappedMemAddr(), &matrices, sizeof(Matrices));

		vkCmdBindDescriptorSets(res.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline.getGraphicsPipelineLayout(), 0, 1, &res.descriptorSet, 0, nullptr);
		VkBuffer vertexBufferHandle = vertexBuffer->getBufferHandle();
		VkDeviceSize offset = 0;
		vkCmdBindVertexBuffers(res.commandBuffer, 0, 1, &vertexBufferHandle, &offset);
		vkCmdBindIndexBuffer(res.commandBuffer, indexBuffer->getBufferHandle(), 0, VK_INDEX_TYPE_UINT32);

		vkCmdPushConstants(res.commandBuffer, graphicsPipeline.getGraphicsPipelineLayout(), VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &model);
		vkCmdDrawIndexed(res.commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
	}
	vkCmdEndRendering(res.commandBuffer);

	VkImageMemoryBarrier2 presentLayoutBarrier
	{
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
		.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
		.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
		.dstStageMask = VK_PIPELINE_STAGE_2_NONE, 
		.dstAccessMask = 0,
		.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		.image = swapchain.getSwapchainImages()[imageIndex],
		.subresourceRange
		{
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1,
		}
	};
	VkDependencyInfo presentDepInfo
	{
		.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
		.imageMemoryBarrierCount = 1,
		.pImageMemoryBarriers = &presentLayoutBarrier
	};
	vkCmdPipelineBarrier2(res.commandBuffer, &presentDepInfo);

	vkEndCommandBuffer(res.commandBuffer);

	VkSemaphoreSubmitInfo imageAcquireWaitInfo
	{
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
		.semaphore = imageAcquireSemaphore,
		.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT
	};

	std::vector<VkSemaphoreSubmitInfo> semaphoreSignals
	{
		{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
			.semaphore = swapchain.getSwapchainRenderCompleteSemaphores()[imageIndex],
			.stageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT
		},
		{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
			.semaphore = timelineSemaphore,
			.value = signalValue,
			.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT
		}
	};
	VkCommandBufferSubmitInfo cmdSubmitInfo
	{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
		.commandBuffer = res.commandBuffer,
	};
	VkSubmitInfo2 submitInfo
	{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
		.waitSemaphoreInfoCount = 1,
		.pWaitSemaphoreInfos = &imageAcquireWaitInfo,
		.commandBufferInfoCount = 1,
		.pCommandBufferInfos = &cmdSubmitInfo,
		.signalSemaphoreInfoCount = static_cast<uint32_t>(semaphoreSignals.size()),
		.pSignalSemaphoreInfos = semaphoreSignals.data()
	};
	vkQueueSubmit2(device.getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);

    VkSwapchainKHR swapchainHandle = swapchain.getSwapchainHandle();  
	VkPresentInfoKHR presentInfo{
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &swapchain.getSwapchainRenderCompleteSemaphores()[imageIndex],
		.swapchainCount = 1,
		.pSwapchains = &swapchainHandle,
		.pImageIndices = &imageIndex,
		.pResults = nullptr
	};

	vkQueuePresentKHR(device.getGraphicsQueue(), &presentInfo);
}