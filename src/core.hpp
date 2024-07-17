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

struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 texCoord;

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

struct UniformBufferObject {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

const std::vector<Vertex> vertices = {
    {{-0.5f, -0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},
    {{0.5f, -0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
    {{0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
    {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},

    {{-0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},
    {{0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
    {{0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
    {{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
};

const std::vector<uint16_t> indices = {
    0, 1, 2, 2, 3, 0,
    4, 5, 6, 6, 7, 4
};

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

        VkRenderPass renderPass;
        VkDescriptorSetLayout descriptorSetLayout;
        VkPipelineLayout pipelineLayout;
        VkPipeline graphicsPipeline;

        VkCommandPool commandPool;
        std::vector<VkCommandBuffer> commandBuffers;

    public:
        void run();

        static Engine* getSingleton() { return singleton; }
        Engine() { singleton = this; }


    private:
        void initialize();
        void mainLoop();
        void cleanup();

        // vulkan stuff, indents are used to show the hierarchy
        // not all functions are here, just the ones that need access to private members

        // defined in src/init/instance.cpp
        void createInstance();
            bool checkValidationLayerSupport();
        void setupDebugMessenger();
        void createSurface();

        // defined in src/init/device.cpp
        void pickPhysicalDevice();
            QueueFamilyIndices findQueueFamilies(VkPhysicalDevice p_device);
            u32 rateDeviceSuitability(VkPhysicalDevice p_device);
            bool checkDeviceExtensionSupport(VkPhysicalDevice p_device);
            SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice p_device);
        void createLogicalDevice();

        // defined in src/init/swap_chain.cpp
        void createSwapChain();
            VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& p_formats);
            VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& p_modes);
            VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& p_capabilities);
            void recreateSwapChain();
        
        // defined in src/init/image.cpp
        void createImageViews();
            VkImageView createImageView(VkImage p_image, VkFormat p_format, VkImageAspectFlags p_aspectFlags);
        
        // defined in src/init/pipeline.cpp
        void createRenderPass();
            VkFormat findDepthFormat();
            VkFormat findSupportedFormat(const std::vector<VkFormat>& p_candidates, VkImageTiling p_tiling, VkFormatFeatureFlags p_features);
        void createDescriptorSetLayout();
        void createGraphicsPipeline();
            VkShaderModule createShaderModule(const std::vector<char>& p_code);
        void createCommandPool();
        void createDepthResources();

};
}