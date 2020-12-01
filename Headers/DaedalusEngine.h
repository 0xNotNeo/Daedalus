#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <soloud.h>
#include <soloud_wav.h>
#include <soloud_wavstream.h>

#include <iostream>
#include <cstring>
#include <vector>

#include "Instance.h"
#include "ValidationLayers.h"
#include "PhysicalDevice.h"
#include "QueueFamily.h"
#include "LogicalDevice.h"

#include "WindowSurface.h"
#include "SwapChain.h"
#include "ImageViews.h"

#include "RenderPass.h"
#include "GraphicsPipeline.h"

#include "FrameBuffers.h"
#include "CommandBuffers.h"
#include "CommandPool.h"
#include "SyncObjects.h"

#include "TextureImage.h"
#include "DepthBuffers.h"
#include "ModelLoader.h"

namespace ddls
{
class DaedalusEngine
{
public:
    
    void run();
    
    DaedalusEngine();
    DaedalusEngine(uint32_t windowWidth_p,
                   uint32_t windowHeight_p,
                   std::string applicationName_p);
    
private:
    
    // Application Infos
    std::string applicationName;
    const std::string engineName = "Daedalus";
    
    // GLFW Window Creation
    GLFWwindow* window;
    uint32_t windowWidth;
    uint32_t windowHeight;
    void initSystem();

    // SoLoud Sound Engine
    SoLoud::Soloud soundEngine;
    SoLoud::WavStream backgroundMusic;
    
    // Resize Handling
    bool framebufferResized;
    static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
        auto app = reinterpret_cast<DaedalusEngine*>(glfwGetWindowUserPointer(window));
        app->framebufferResized = true;
    }
    
    // Main Functions
    void initVulkan();
    void mainLoop();
    void cleanup();
    
    // Drawing
    void drawFrame();
    
    // Vulkan Instance
    VkInstance instance;
    
    // Vulkan Validation Layers
    VkDebugUtilsMessengerEXT debugMessenger;
    
    // Vulkan Physical Device & Queue Families
    VkPhysicalDevice physicalDevice;
    
    // Vulkan Logical Device
    VkDevice device;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    
    // Vulkan Window Surface
    VkSurfaceKHR surface;
    
    // Vulkan Swap Chain 
    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    void cleanupSwapChain();
    void recreateSwapChain();
    
    // Vulkan Image Views
    std::vector<VkImageView> swapChainImageViews;
    
    // Vulkan Render Passes
    VkRenderPass renderPass;
    
    // Vulkan Graphics Pipeline
    VkDescriptorSetLayout descriptorSetLayout;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;
    
    // Vulkan Frame Buffers
    std::vector<VkFramebuffer> swapChainFrameBuffers;
    
    // Vulkan Command Pool
    VkCommandPool commandPool;
    
    // Vulkan Command Buffers
    std::vector<VkCommandBuffer> commandBuffers;
    
    // Vulkan Semaphores & Fences
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    std::vector<VkFence> imagesInFlight;
    size_t currentFrame;
    
    // Vulkan Buffers
    std::vector<Vertex> vertices;
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    std::vector<uint32_t> indices;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;
    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;
    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;
    
    // Vulkan Textures
    VkImage textureImage;
    VkDeviceMemory textureImageMemory;
    VkImageView textureImageView;
    VkSampler textureSampler;
    
    // Vulkan Depth
    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;
};
}
