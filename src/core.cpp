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
        glfwSetFramebufferSizeCallback(window, [](GLFWwindow* p_window, int p_width, int p_height) {
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
    }

    void Engine::mainLoop() {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
        }
    }

    void Engine::cleanup() {
        glfwDestroyWindow(window);
        glfwTerminate();
    }
}