#include "core.hpp"

using namespace wmac;

void Engine::createVertexBuffer() {
    vertexBuffer.size = sizeof(vertices[0]) * MAX_VERTICES;

    createBuffer(vertexBuffer.size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vertexBuffer.stagingOpaque, vertexBuffer.stagingMemory);

    vkMapMemory(device, vertexBuffer.stagingMemory, 0, vertexBuffer.size, 0, &vertexBuffer.mapped);
    memcpy(vertexBuffer.mapped, vertices.data(), (size_t) vertexBuffer.size);

    createBuffer(vertexBuffer.size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer.opaque, vertexBuffer.memory);

    copyBuffer(vertexBuffer.stagingOpaque, vertexBuffer.opaque, vertexBuffer.size);
}

void Engine::updateVertexBuffer(const std::vector<Vertex>& p_newVertices) {
    memcpy(vertexBuffer.mapped, p_newVertices.data(), (size_t) vertexBuffer.size);
    copyBuffer(vertexBuffer.stagingOpaque, vertexBuffer.opaque, vertexBuffer.size);
}

void Engine::createIndexBuffer() {
    indexBuffer.size = sizeof(indices[0]) * MAX_INDICES;

    createBuffer(indexBuffer.size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, indexBuffer.stagingOpaque, indexBuffer.stagingMemory);

    vkMapMemory(device, indexBuffer.stagingMemory, 0, indexBuffer.size, 0, &indexBuffer.mapped);
    memcpy(indexBuffer.mapped, indices.data(), (size_t) vertexBuffer.size);

    createBuffer(vertexBuffer.size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer.opaque, indexBuffer.memory);

    copyBuffer(indexBuffer.stagingOpaque, indexBuffer.opaque, indexBuffer.size);
}

void Engine::updateIndexBuffer(const std::vector<u32>& p_newIndices) {
    memcpy(indexBuffer.mapped, p_newIndices.data(), (size_t) indexBuffer.size);
    copyBuffer(indexBuffer.stagingOpaque, indexBuffer.opaque, indexBuffer.size);
}

void Engine::createUniformBuffers() {
    VkDeviceSize bufferSize = sizeof(mat4);

    uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
    uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i], uniformBuffersMemory[i]);

        vkMapMemory(device, uniformBuffersMemory[i], 0, bufferSize, 0, &uniformBuffersMapped[i]);
    }
}

void Engine::createBuffer(VkDeviceSize p_size, VkBufferUsageFlags p_usage, VkMemoryPropertyFlags p_properties, VkBuffer& p_buffer, VkDeviceMemory& p_bufferMemory) {
    VkBufferCreateInfo bufferInfo {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = p_size,
        .usage = p_usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    VkResult result = vkCreateBuffer(device, &bufferInfo, nullptr, &p_buffer);
    ASSERT_FATAL(result == VK_SUCCESS, "failed to create vertex buffer!");

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, p_buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, p_properties),
    };

    result = vkAllocateMemory(device, &allocInfo, nullptr, &p_bufferMemory);
    ASSERT_FATAL(result == VK_SUCCESS, "failed to allocate vertex buffer memory!");
    

    vkBindBufferMemory(device, p_buffer, p_bufferMemory, 0);
}

void Engine::copyBuffer(VkBuffer p_srcBuffer, VkBuffer p_dstBuffer, VkDeviceSize p_size) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkBufferCopy copyRegion{
        .size = p_size,
    };
    vkCmdCopyBuffer(commandBuffer, p_srcBuffer, p_dstBuffer, 1, &copyRegion);

    endSingleTimeCommands(commandBuffer);
}

void Engine::copyBufferToImage(VkBuffer p_buffer, VkImage p_image, u32 p_width, u32 p_height) {
    VkBufferImageCopy region {
        .bufferOffset = 0,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .imageSubresource = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .mipLevel = 0,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
        .imageOffset = {0, 0, 0},
        .imageExtent = {p_width, p_height, 1},
    };

    VkCommandBuffer commandBuffer = beginSingleTimeCommands();
        vkCmdCopyBufferToImage(
            commandBuffer,
            p_buffer,
            p_image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &region
        );
    endSingleTimeCommands(commandBuffer);
}

VkCommandBuffer Engine::beginSingleTimeCommands() {
    VkCommandBufferAllocateInfo allocInfo {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void Engine::endSingleTimeCommands(VkCommandBuffer p_commandBuffer) {
    vkEndCommandBuffer(p_commandBuffer);

    VkSubmitInfo submitInfo {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &p_commandBuffer,
    };

    vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphicsQueue);

    vkFreeCommandBuffers(device, commandPool, 1, &p_commandBuffer);
}

void Engine::createCommandBuffers() {
    commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = MAX_FRAMES_IN_FLIGHT,
    };

    VkResult result = vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data());
    ASSERT_FATAL(result == VK_SUCCESS, "failed to allocate command buffers!");
}

void Engine::recordCommandBuffer(VkCommandBuffer p_commandBuffer, u32 p_imageIndex) {
    VkCommandBufferBeginInfo beginInfo {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = 0, // optional, useless for us
        .pInheritanceInfo = nullptr, // for secondary command buffers
    };

    std::array<VkClearValue, 2> clearValues {
        VkClearValue {0.0f, 0.0f, 0.0f, 1.0f},
        VkClearValue {1.0f, 0},
    };

    VkRenderPassBeginInfo renderPassInfo {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = renderPass,
        .framebuffer = swapChainFramebuffers[p_imageIndex],
        .renderArea = {
            .offset = {0, 0},
            .extent = swapChainExtent,
        },
        .clearValueCount = scast<u32>(clearValues.size()),
        .pClearValues = clearValues.data(),
    };

    VkViewport viewport {
        .width = (float) swapChainExtent.width,
        .height = (float) swapChainExtent.height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };

    VkRect2D scissor {
        .offset = {0, 0},
        .extent = swapChainExtent,
    };

    VkBuffer vertexBuffers[] = {vertexBuffer.opaque};
    VkDeviceSize offsets[] = {0};

    VkResult result = vkBeginCommandBuffer(p_commandBuffer, &beginInfo);
    ASSERT_FATAL(result == VK_SUCCESS, "failed to begin recording command buffer!");

        vkCmdBeginRenderPass(p_commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

            vkCmdBindPipeline(p_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

            vkCmdSetViewport(p_commandBuffer, 0, 1, &viewport);
            vkCmdSetScissor(p_commandBuffer, 0, 1, &scissor);

            vkCmdBindVertexBuffers(p_commandBuffer, 0, 1, vertexBuffers, offsets);
            vkCmdBindIndexBuffer(p_commandBuffer, indexBuffer.opaque, 0, VK_INDEX_TYPE_UINT32);

            vkCmdBindDescriptorSets(p_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[currentFrame], 0, nullptr);

            vkCmdDrawIndexed(p_commandBuffer, scast<u32>(indices.size()), 1, 0, 0, 0);

        vkCmdEndRenderPass(p_commandBuffer);

    result = vkEndCommandBuffer(p_commandBuffer);
    ASSERT_FATAL(result == VK_SUCCESS, "failed to record command buffer!");
}

void Engine::createSyncObjects() {
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };

    VkFenceCreateInfo fenceInfo {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT,
    };

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        bool result =
            vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) == VK_SUCCESS &&
            vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) == VK_SUCCESS &&
            vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) == VK_SUCCESS;
        
        ASSERT_FATAL(result, "failed to create synchronization objects for a frame!");
    }
}

