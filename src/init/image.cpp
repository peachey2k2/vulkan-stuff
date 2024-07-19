#include "core.hpp"

using namespace wmac;

void Engine::createImageViews() {
    swapChainImageViews.resize(swapChainImages.size());

    for (u32 i = 0; i < swapChainImages.size(); i++) {
        swapChainImageViews[i] = createImageView(swapChainImages[i], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
    }
}

VkImageView Engine::createImageView(VkImage p_image, VkFormat p_format, VkImageAspectFlags p_aspectFlags) {
    VkImageViewCreateInfo viewInfo {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = p_image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = p_format,
        .subresourceRange = {
            .aspectMask = p_aspectFlags,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
    };

    VkImageView imageView;
    VkResult result = vkCreateImageView(device, &viewInfo, nullptr, &imageView);
    ASSERT_FATAL(result == VK_SUCCESS, "failed to create image view!");

    return imageView;
}

void Engine::createDepthResources() {
    VkFormat depthFormat = findDepthFormat();
    createImage(
        swapChainExtent.width,
        swapChainExtent.height,
        depthFormat,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        depthImage,
        depthImageMemory);
    depthImageView = createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
}

void Engine::createImage(
    u32 p_width,
    u32 p_height,
    VkFormat p_format,
    VkImageTiling p_tiling,
    VkImageUsageFlags p_usage,
    VkMemoryPropertyFlags p_properties,
    VkImage& p_image,
    VkDeviceMemory& p_imageMemory
) {
    VkImageCreateInfo imageInfo {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .flags = 0,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = p_format,
        .extent = {p_width, p_height, 1},
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = p_tiling,
        .usage = p_usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };

    VkResult result = vkCreateImage(device, &imageInfo, nullptr, &p_image);
    ASSERT_FATAL(result == VK_SUCCESS, "failed to create image!");

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, p_image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, p_properties),
    };

    result = vkAllocateMemory(device, &allocInfo, nullptr, &p_imageMemory);
    ASSERT_FATAL(result == VK_SUCCESS, "failed to allocate image memory!");

    vkBindImageMemory(device, p_image, p_imageMemory, 0);
}

void Engine::createTextureImage() {
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load("texture.png", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;

    ASSERT_FATAL(pixels, "failed to load texture image!");

    createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, imageStagingBuffer, imageStagingBufferMemory);

    void* data;
    vkMapMemory(device, imageStagingBufferMemory, 0, imageSize, 0, &data);
        memcpy(data, pixels, scast<size_t>(imageSize));
    vkUnmapMemory(device, imageStagingBufferMemory);

    stbi_image_free(pixels);

    createImage(
        texWidth,
        texHeight,
        VK_FORMAT_R8G8B8A8_SRGB,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        textureImage,
        textureImageMemory
    );

    transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        copyBufferToImage(imageStagingBuffer, textureImage, scast<u32>(texWidth), scast<u32>(texHeight));
    transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vkDestroyBuffer(device, imageStagingBuffer, nullptr);
    vkFreeMemory(device, imageStagingBufferMemory, nullptr);
}
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter" // TODO: look into this
void Engine::transitionImageLayout(VkImage p_image, VkFormat p_format, VkImageLayout p_oldLayout, VkImageLayout p_newLayout) {
#pragma GCC diagnostic pop
    VkImageMemoryBarrier barrier {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .oldLayout = p_oldLayout,
        .newLayout = p_newLayout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = p_image,
        .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
    };

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (p_oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && p_newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (p_oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && p_newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else {
        throw engine_fatal_exception("unsupported layout transition!");
    }

    VkCommandBuffer commandBuffer = beginSingleTimeCommands();
        vkCmdPipelineBarrier(
            commandBuffer,
            sourceStage, destinationStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );
    endSingleTimeCommands(commandBuffer);
}

void Engine::createTextureImageView() {
    textureImageView = createImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
}

void Engine::createTextureSampler() {
    VkSamplerCreateInfo samplerInfo {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter = VK_FILTER_LINEAR, // VK_FILTER_NEAREST for pixelated
        .minFilter = VK_FILTER_LINEAR, // VK_FILTER_NEAREST for pixelated
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR, // VK_SAMPLER_MIPMAP_MODE_NEAREST for pixelated
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .mipLodBias = 0.0f,
        .anisotropyEnable = VK_TRUE,
        .maxAnisotropy = 16,
        .compareEnable = VK_FALSE,
        .compareOp = VK_COMPARE_OP_ALWAYS,
        .minLod = 0.0f,
        .maxLod = 0.0f,
        .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
        .unnormalizedCoordinates = VK_FALSE,
    };

    VkResult result = vkCreateSampler(device, &samplerInfo, nullptr, &textureSampler);
    ASSERT_FATAL(result == VK_SUCCESS, "failed to create texture sampler!");
}
