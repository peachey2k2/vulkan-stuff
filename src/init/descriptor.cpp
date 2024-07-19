#include "core.hpp"

using namespace wmac;

void Engine::createDescriptorPool() {
    std::array<VkDescriptorPoolSize, 2> poolSizes {
        {
            {
                .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .descriptorCount = scast<u32>(MAX_FRAMES_IN_FLIGHT),
            },
            {
                .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .descriptorCount = scast<u32>(MAX_FRAMES_IN_FLIGHT),
            },
        }
    };

    VkDescriptorPoolCreateInfo poolInfo {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .maxSets = scast<u32>(MAX_FRAMES_IN_FLIGHT),
        .poolSizeCount = scast<u32>(poolSizes.size()),
        .pPoolSizes = poolSizes.data(),
    };

    VkResult result = vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool);
    ASSERT_FATAL(result == VK_SUCCESS, "failed to create descriptor pool!");
}

void Engine::createDescriptorSets() {
    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = descriptorPool,
        .descriptorSetCount = scast<u32>(MAX_FRAMES_IN_FLIGHT),
        .pSetLayouts = layouts.data(),
    };

    descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
    VkResult result = vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data());
    ASSERT_FATAL(result == VK_SUCCESS, "failed to allocate descriptor sets!");

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorBufferInfo bufferInfo {
            .buffer = uniformBuffers[i],
            .offset = 0,
            .range = sizeof(mat4),
        };

        VkDescriptorImageInfo imageInfo {
            .sampler = textureSampler,
            .imageView = textureImageView,
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        };

        std::array<VkWriteDescriptorSet, 2> descriptorWrites {
            {
                {
                    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    .dstSet = descriptorSets[i],
                    .dstBinding = 0,
                    .dstArrayElement = 0,
                    .descriptorCount = 1,
                    .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                    .pBufferInfo = &bufferInfo,
                },
                {
                    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    .dstSet = descriptorSets[i],
                    .dstBinding = 1,
                    .dstArrayElement = 0,
                    .descriptorCount = 1,
                    .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                    .pImageInfo = &imageInfo,
                },
            }
        };

        vkUpdateDescriptorSets(device, scast<u32>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }
}

