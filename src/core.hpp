#pragma once

#define GLFW_INCLUDE_VULKAN // include vulkan headers with glfw
#define GLM_FORCE_DEPTH_ZERO_TO_ONE // use 0 to 1 depth range instead of -1 to 1 (opengl)
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "stb_image.h"

#include "helpers.hpp"
#include "debug.hpp"

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <cstring>
#include <optional>
#include <set>
#include <limits>
#include <algorithm>
#include <fstream>
#include <array>
#include <chrono>
#include <cstddef>
// #include <cstddef>

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif


namespace wmac {

struct QueueFamilyIndices {
    std::optional<u32> graphicsFamily;
    std::optional<u32> presentFamily;

    bool isComplete() { return graphicsFamily.has_value(); };
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

struct Buffer {
    VkBuffer opaque;
    VkDeviceMemory memory;
    VkBuffer stagingOpaque;
    VkDeviceMemory stagingMemory;
    VkDeviceSize size;
    void* mapped;
};

struct Vertex {
    vec3 pos;
    vec3 color;
    vec2 texCoord;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription {
            .binding = 0,
            .stride = sizeof(Vertex),
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
        };
        
        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions {
            VkVertexInputAttributeDescription {
                .location = 0,
                .binding = 0,
                .format = VK_FORMAT_R32G32B32_SFLOAT,
                // if you're seeing errors here, just ignore them, they're not real
                .offset = offsetof(Vertex, pos),
            },
            VkVertexInputAttributeDescription {
                .location = 1,
                .binding = 0,
                .format = VK_FORMAT_R32G32B32_SFLOAT,
                .offset = offsetof(Vertex, color),
            },
            VkVertexInputAttributeDescription {
                .location = 2,
                .binding = 0,
                .format = VK_FORMAT_R32G32_SFLOAT,
                .offset = offsetof(Vertex, texCoord),
            },
        };

        return attributeDescriptions;
    }
};

extern const std::vector<Vertex> vertices;
extern const std::vector<u32> indices;

extern const u32 WIDTH;
extern const u32 HEIGHT;

extern const u32 MAX_FRAMES_IN_FLIGHT;
extern const u32 MAX_VERTICES;
extern const u32 MAX_INDICES;

class Engine {
    public:
        bool framebufferResized = false;

    private:
        static Engine* singleton;

        const std::vector<const char*> validationLayers = {
            "VK_LAYER_KHRONOS_validation"
        };

        GLFWwindow* window;
        VkInstance instance;
        VkDebugUtilsMessengerEXT debugMessenger;
        VkSurfaceKHR surface;

        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        VkDevice device;

        VkQueue graphicsQueue;
        VkQueue presentQueue;

        VkSwapchainKHR swapChain;
        std::vector<VkImage> swapChainImages;
        VkFormat swapChainImageFormat;
        VkExtent2D swapChainExtent;
        std::vector<VkImageView> swapChainImageViews;
        std::vector<VkFramebuffer> swapChainFramebuffers;

        VkRenderPass renderPass;
        VkDescriptorSetLayout descriptorSetLayout;
        VkPipelineLayout pipelineLayout;
        VkPipeline graphicsPipeline;

        VkDescriptorPool descriptorPool;
        std::vector<VkDescriptorSet> descriptorSets;

        VkCommandPool commandPool;
        std::vector<VkCommandBuffer> commandBuffers;

        std::vector<VkSemaphore> imageAvailableSemaphores;
        std::vector<VkSemaphore> renderFinishedSemaphores;
        std::vector<VkFence> inFlightFences;

        // VkBuffer vertexBuffer;
        // VkDeviceMemory vertexBufferMemory;

        // VkBuffer indexBuffer;
        // VkDeviceMemory indexBufferMemory;
        Buffer vertexBuffer;
        Buffer indexBuffer;

        std::vector<VkBuffer> uniformBuffers;
        std::vector<VkDeviceMemory> uniformBuffersMemory;
        std::vector<void*> uniformBuffersMapped;

        // VkBuffer vertexStagingBuffer;
        // VkDeviceMemory vertexStagingBufferMemory;
        // void* vertexStagingBufferData;

        // VkBuffer indexStagingBuffer;
        // VkDeviceMemory indexStagingBufferMemory;
        // void* indexStagingBufferData;

        VkBuffer imageStagingBuffer;
        VkDeviceMemory imageStagingBufferMemory;

        VkImage textureImage;
        VkDeviceMemory textureImageMemory;

        VkImageView textureImageView;
        VkSampler textureSampler;

        VkImage depthImage;
        VkDeviceMemory depthImageMemory;
        VkImageView depthImageView;

        u32 currentFrame = 0;

    public:
        void run();

        static Engine* getSingleton() { return singleton; }
        Engine() { singleton = this; }


    private:
        // indents are used to show hierarchy
        void initialize();
        void mainLoop();
            void drawFrame();
            void recordCommandBuffer(VkCommandBuffer p_commandBuffer, uint32_t p_imageIndex);
            void updateUniformBuffer(uint32_t p_currentImage);

        void cleanup();
            void cleanupVulkan();
            void cleanupSwapChain();

        VkResult CreateDebugUtilsMessengerEXT(
            VkInstance p_instance,
            const VkDebugUtilsMessengerCreateInfoEXT* p_createInfo,
            const VkAllocationCallbacks* p_allocator,
            VkDebugUtilsMessengerEXT* p_debugMessenger
        );
        void DestroyDebugUtilsMessengerEXT(
            VkInstance p_instance,
            VkDebugUtilsMessengerEXT p_debugMessenger,
            const VkAllocationCallbacks* p_allocator
        );
        static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT p_messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT p_messageType,
            const VkDebugUtilsMessengerCallbackDataEXT* p_pCallbackData,
            void* p_pUserData
        );

        // vulkan stuff below

        // src/init/instance.cpp
        void createInstance();
            bool checkValidationLayerSupport();
            std::vector<const char*> getRequiredExtensions();
        void setupDebugMessenger();
            void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& p_createInfo);
        void createSurface();

        // src/init/device.cpp
        void pickPhysicalDevice();
            QueueFamilyIndices findQueueFamilies(VkPhysicalDevice p_device);
            u32 rateDeviceSuitability(VkPhysicalDevice p_device);
            bool checkDeviceExtensionSupport(VkPhysicalDevice p_device);
            SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice p_device);
        void createLogicalDevice();
            u32 findMemoryType(u32 p_typeFilter, VkMemoryPropertyFlags p_properties);

        // src/init/swap_chain.cpp
        void createSwapChain();
            VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& p_formats);
            VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& p_modes);
            VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& p_capabilities);
            void recreateSwapChain();
        
        // src/init/image.cpp
        void createImageViews();
            VkImageView createImageView(VkImage p_image, VkFormat p_format, VkImageAspectFlags p_aspectFlags);
        
        // src/init/pipeline.cpp
        void createRenderPass();
            VkFormat findDepthFormat();
            VkFormat findSupportedFormat(const std::vector<VkFormat>& p_candidates, VkImageTiling p_tiling, VkFormatFeatureFlags p_features);
        void createDescriptorSetLayout();
        void createGraphicsPipeline();
            VkShaderModule createShaderModule(const std::vector<char>& p_code);
            static std::vector<char> readFile(const std::string& p_filename);
        void createCommandPool();
        void createDepthResources();
            void createImage(u32 p_width, u32 p_height, VkFormat p_format, VkImageTiling p_tiling, VkImageUsageFlags p_usage, VkMemoryPropertyFlags p_properties, VkImage& p_image, VkDeviceMemory& p_imageMemory);
       
        // src/init/swap_chain.cpp
        void createFramebuffers();

        // src/init/image.cpp
        void createTextureImage();
            void transitionImageLayout(VkImage p_image, VkFormat p_format, VkImageLayout p_oldLayout, VkImageLayout p_newLayout);
        void createTextureImageView();
        void createTextureSampler();

        // src/init/buffers.cpp
        void createVertexBuffer();
            void updateVertexBuffer(const std::vector<Vertex>& p_newVertices);
        void createIndexBuffer();
            void updateIndexBuffer(const std::vector<u32>& p_newIndices);
        void createUniformBuffers();
            void createBuffer(VkDeviceSize p_size, VkBufferUsageFlags p_usage, VkMemoryPropertyFlags p_properties, VkBuffer& p_buffer, VkDeviceMemory& p_bufferMemory);
            void copyBuffer(VkBuffer p_srcBuffer, VkBuffer p_dstBuffer, VkDeviceSize p_size);
            void copyBufferToImage(VkBuffer p_buffer, VkImage p_image, u32 p_width, u32 p_height);
            VkCommandBuffer beginSingleTimeCommands();
            void endSingleTimeCommands(VkCommandBuffer p_commandBuffer);

        // src/init/descriptor.cpp
        void createDescriptorPool();
        void createDescriptorSets();

        // src/init/buffers.cpp
        void createCommandBuffers();
        void createSyncObjects();


};
}