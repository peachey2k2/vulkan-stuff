#define STB_IMAGE_IMPLEMENTATION // include stb_image implementation, rather than just the header
#include "core.hpp"

const u32 WIDTH = 800;
const u32 HEIGHT = 600;

namespace wmac {
    Engine* Engine::singleton = nullptr;

    void Engine::run() {
        initialize();
        mainLoop();
        cleanup();
    }

    void Engine::initialize() {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // don't create an opengl context
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // don't allow resizing (for now)

        // initialize the glfw window
        window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
        glfwSetFramebufferSizeCallback(window, [](GLFWwindow* p_window, int p_width, int p_height) {
#pragma GCC diagnostic pop
            Engine::getSingleton()->framebufferResized = true;
        });

        // initialize vulkan. this is where the "fun" begins
        // this part is WAY TOO LONG, so it's been splited up into multiple files
        createInstance();
        setupDebugMessenger();
        createSurface();

        pickPhysicalDevice();
        createLogicalDevice();

        createSwapChain();

        createImageViews();

        createRenderPass();
        createDescriptorSetLayout();
        createGraphicsPipeline();
        createCommandPool();
        createDepthResources();

        createFramebuffers();
        createTextureImage();
        createTextureImageView();
        createTextureSampler();

        createVertexBuffer();
        createIndexBuffer();
        createUniformBuffers();

        createDescriptorPool();
        createDescriptorSets();

        createCommandBuffers();
        createSyncObjects();
    }

    void Engine::mainLoop() {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            drawFrame();
        }
        vkDeviceWaitIdle(device);
    }

    void Engine::drawFrame() {
        vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

        if (imageIndex >= MAX_FRAMES_IN_FLIGHT) {
            // for some fucking reason, imageIndex is sometimes equal to MAX_FRAMES_IN_FLIGHT
            // idk how nor why, i couldn't even find anything about that on internet.
            // it doesn't majorly ruin things unless i set MAX_FRAMES_IN_FLIGHT higher.
            // also unnsecessary console spam. TODO: fix, somehow...
            std::cout << "--dw about the following error, it's fine. it's always fine--" << '\n';
            return;
        }

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreateSwapChain();
            return;
        } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw engine_fatal_exception("failed to acquire swap chain image!");
        }

        vkResetFences(device, 1, &inFlightFences[currentFrame]);

        vkResetCommandBuffer(commandBuffers[currentFrame], 0);
        recordCommandBuffer(commandBuffers[currentFrame], imageIndex);

        updateUniformBuffer(imageIndex);
        updateVertexBuffer(vertices);
        updateIndexBuffer(indices);

        VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};

        VkSubmitInfo submitInfo {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = waitSemaphores,
            .pWaitDstStageMask = waitStages,
            .commandBufferCount = 1,
            .pCommandBuffers = &commandBuffers[currentFrame],
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = signalSemaphores,
        };

        result = vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]);
        ASSERT_FATAL(result == VK_SUCCESS, "failed to submit draw command buffer!");

        VkPresentInfoKHR presentInfo {
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = signalSemaphores,
            .swapchainCount = 1,
            .pSwapchains = &swapChain,
            .pImageIndices = &imageIndex,
        };

        result = vkQueuePresentKHR(presentQueue, &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
            framebufferResized = false;
            recreateSwapChain();
        } else if (result != VK_SUCCESS) {
            throw engine_fatal_exception("failed to present swap chain image!");
        }

        ++currentFrame %= MAX_FRAMES_IN_FLIGHT;
    }

    #define FREE_ARRAY(m_array, m_func) \
    for (auto& __e : m_array) {         \
        m_func;                         \
    }                                   \

    void Engine::cleanup() {
        cleanupVulkan();
    }

    void Engine::cleanupVulkan() {
        cleanupSwapChain();

        vkDestroySampler(device, textureSampler, nullptr);
        vkDestroyImageView(device, textureImageView, nullptr);

        vkDestroyImage(device, textureImage, nullptr);
        vkFreeMemory(device, textureImageMemory, nullptr);

        FREE_ARRAY(uniformBuffers, vkDestroyBuffer(device, __e, nullptr));
        FREE_ARRAY(uniformBuffersMemory, vkFreeMemory(device, __e, nullptr));

        vkDestroyDescriptorPool(device, descriptorPool, nullptr);

        vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

        vkUnmapMemory(device, vertexBuffer.stagingMemory);
        vkDestroyBuffer(device, vertexBuffer.opaque, nullptr);
        vkDestroyBuffer(device, vertexBuffer.stagingOpaque, nullptr);
        vkFreeMemory(device, vertexBuffer.memory, nullptr);
        vkFreeMemory(device, vertexBuffer.stagingMemory, nullptr);

        vkUnmapMemory(device, indexBuffer.stagingMemory);
        vkDestroyBuffer(device, indexBuffer.opaque, nullptr);
        vkDestroyBuffer(device, indexBuffer.stagingOpaque, nullptr);
        vkFreeMemory(device, indexBuffer.memory, nullptr);
        vkFreeMemory(device, indexBuffer.stagingMemory, nullptr);
        
        FREE_ARRAY(imageAvailableSemaphores, vkDestroySemaphore(device, __e, nullptr));
        FREE_ARRAY(renderFinishedSemaphores, vkDestroySemaphore(device, __e, nullptr));
        FREE_ARRAY(inFlightFences, vkDestroyFence(device, __e, nullptr));

        vkDestroyCommandPool(device, commandPool, nullptr);

        vkDestroyPipeline(device, graphicsPipeline, nullptr);
        vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
        vkDestroyRenderPass(device, renderPass, nullptr);

        vkDestroyDevice(device, nullptr);

        if (enableValidationLayers) {
            DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
        }

        vkDestroySurfaceKHR(instance, surface, nullptr);
        vkDestroyInstance(instance, nullptr);

        glfwDestroyWindow(window);
        glfwTerminate();
    }

    void Engine::cleanupSwapChain() {
        vkDestroyImageView(device, depthImageView, nullptr);
        vkDestroyImage(device, depthImage, nullptr);
        vkFreeMemory(device, depthImageMemory, nullptr);

        FREE_ARRAY(swapChainFramebuffers, vkDestroyFramebuffer(device, __e, nullptr));
        FREE_ARRAY(swapChainImageViews, vkDestroyImageView(device, __e, nullptr));
        vkDestroySwapchainKHR(device, swapChain, nullptr);
    }

    // these functions have to be bound manually
    VkResult Engine::CreateDebugUtilsMessengerEXT(VkInstance p_instance, const VkDebugUtilsMessengerCreateInfoEXT* p_pCreateInfo, const VkAllocationCallbacks* p_pAllocator, VkDebugUtilsMessengerEXT* p_pDebugMessenger) {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(p_instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr) {
            return func(p_instance, p_pCreateInfo, p_pAllocator, p_pDebugMessenger);
        } else {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

    void Engine::DestroyDebugUtilsMessengerEXT(VkInstance p_instance, VkDebugUtilsMessengerEXT p_debugMessenger, const VkAllocationCallbacks* p_pAllocator) {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(p_instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr) {
            func(p_instance, p_debugMessenger, p_pAllocator);
        }
    }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
    // debug callback for vulkan validation layers
    VKAPI_ATTR VkBool32 VKAPI_CALL Engine::debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT p_messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT p_messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* p_pCallbackData,
        void* p_pUserData
    ) {
#pragma GCC diagnostic pop
        static u32 errorCount = 0;

        std::cerr << (p_messageType & 0b1 ? "\x1b[36m[INFO] " : "\x1b[31m[ERROR("+std::to_string(++errorCount)+")] ") << "\x1b[0m" << p_pCallbackData->pMessage << '\n';

        ASSERT_FATAL(errorCount < 50, "too many errors!");

        return VK_FALSE;
    }
}