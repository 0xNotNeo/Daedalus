#include "Engine.h"

#include "Types.h"
#include "Initializers.h"
#include "Bootstrap.h"

#include <algorithm>
#include <iostream>

// We want to immediately abort when there is an error. In normal engines this would give an error message to the user, or perform a dump of state
#define VK_CHECK(x)                                                      \
    do                                                                   \
    {                                                                    \
        VkResult error = x;                                              \
        if (error)                                                       \
        {                                                                \
            std::cout <<"Detected Vulkan error: " << error << std::endl; \
            abort();                                                     \
        }                                                                \
    } while (0)

Engine::Engine(const std::vector<std::string> &args, void *caMetalLayer) : caMetalLayer(caMetalLayer) {
    instanceExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
    
    if (std::find(args.begin(), args.end(), "-validate") != args.end()) { settings.validate = true; }
    if (std::find(args.begin(), args.end(), "-verbose") != args.end()) { settings.validate = true; settings.verbose = true; }
    
    instanceExtensions.push_back(VK_EXT_METAL_SURFACE_EXTENSION_NAME);
    
    initialize();
}

Engine::~Engine() {
    // Destroy context
    terminate();
}

void Engine::initialize() {
    initializeVulkan();
    
    initializeSwapchain();
    
    initializeCommands();
    
    initializeDefaultRenderPass();
    
    initializeFramebuffers();
    
    initializeSyncStructures();
    
    isInitialized = true;
}

void Engine::terminate() {
    if (isInitialized) {
        vkDestroyCommandPool(device, commandPool, VK_NULL_HANDLE);
        vkDestroySwapchainKHR(device, swapchain, VK_NULL_HANDLE);
        vkDestroyRenderPass(device, renderPass, VK_NULL_HANDLE);
        std::for_each(framebuffers.begin(), framebuffers.end(), [device = device](VkFramebuffer framebuffer) {
            vkDestroyFramebuffer(device, framebuffer, VK_NULL_HANDLE); });
        std::for_each(swapchainImageViews.begin(), swapchainImageViews.end(), [device = device](VkImageView imageView) {
            vkDestroyImageView(device, imageView, VK_NULL_HANDLE); });
        
        vkDestroyDevice(device, VK_NULL_HANDLE);
        vkDestroySurfaceKHR(instance, surface, VK_NULL_HANDLE);
        if (settings.validate) vkb::destroy_debug_utils_messenger(instance, debugMessenger);
        vkDestroyInstance(instance, VK_NULL_HANDLE);
    }
}

void Engine::run() {
    // Create context
    // Create/Resize swapchain
}

void Engine::update() {
    
}

void Engine::render() {
    
}

void Engine::onKey(Key key) {
    
}

void Engine::initializeVulkan() {
    // Instance and debug messenger creation
    vkb::InstanceBuilder builder;
    
    auto instanceBuilder = builder.set_engine_name(settings.engineName.c_str())
        .set_engine_version(0, 1)
        .set_app_name(settings.applicationName.c_str())
        .set_app_version(0, 1)
        .require_api_version(1, 1, 0);
    
    if (settings.validate) instanceBuilder.request_validation_layers(true).use_default_debug_messenger();
    if (settings.verbose) instanceBuilder.add_debug_messenger_severity(VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT);
    
    std::for_each(instanceLayers.begin(), instanceLayers.end(), [&instanceBuilder](const char* instanceLayer) {
        instanceBuilder.enable_layer(instanceLayer); });
    std::for_each(instanceExtensions.begin(), instanceExtensions.end(), [&instanceBuilder](const char* instanceLayer) {
        instanceBuilder.enable_extension(instanceLayer); });
    
    vkb::Instance vkbInstance = instanceBuilder.build().value();
    
    this->instance = vkbInstance.instance;
    this->debugMessenger = vkbInstance.debug_messenger;
    
    // Surface creation
    VkMetalSurfaceCreateInfoEXT surfaceInfo;
    surfaceInfo.sType = VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT;
    surfaceInfo.pNext = VK_NULL_HANDLE;
    surfaceInfo.flags = 0;
    surfaceInfo.pLayer = caMetalLayer;
    VK_CHECK(vkCreateMetalSurfaceEXT(this->instance, &surfaceInfo, VK_NULL_HANDLE, &this->surface));
    
    // Physical device creation
    vkb::PhysicalDeviceSelector physicalDeviceSelector{vkbInstance};
    vkb::PhysicalDevice physicalDevice = physicalDeviceSelector
        .set_minimum_version(1, 1)
        .set_surface(this->surface)
        .add_desired_extension("VK_KHR_portability_subset")
        .select()
        .value();
    
    // Device creation
    vkb::DeviceBuilder deviceBuilder{physicalDevice};
    vkb::Device vkbDevice = deviceBuilder.build().value();
    
    this->device = vkbDevice.device;
    this->physicalDevice = physicalDevice.physical_device;
    
    graphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
    graphicsQueueFamily = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();
    
}

void Engine::initializeSwapchain() {
    vkb::SwapchainBuilder swapchainBuilder{physicalDevice, device, surface};
    
    vkb::Swapchain vkbSwapchain = swapchainBuilder
        .use_default_format_selection()
        .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
        .set_desired_extent(settings.windowExtent.width, settings.windowExtent.height)
        .build()
        .value();
    
    swapchain = vkbSwapchain.swapchain;
    swapchainImages = vkbSwapchain.get_images().value();
    swapchainImageViews = vkbSwapchain.get_image_views().value();
    
    swapchainImageFormat = vkbSwapchain.image_format;
}

void Engine::initializeCommands() {
    VkCommandPoolCreateInfo commandPoolInfo = init::command_pool_create_info(graphicsQueueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    VK_CHECK(vkCreateCommandPool(device, &commandPoolInfo, VK_NULL_HANDLE, &commandPool));
    
    VkCommandBufferAllocateInfo commandBufferAllocateInfo = init::command_buffer_allocate_info(commandPool, 1);

    VK_CHECK(vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, &mainCommandBuffer));
}

void Engine::initializeDefaultRenderPass() {
    // the renderpass will use this color attachment.
    VkAttachmentDescription colorAttachment = {};
    //the attachment will have the format needed by the swapchain
    colorAttachment.format = swapchainImageFormat;
    //1 sample, we won't be doing MSAA
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    // we Clear when this attachment is loaded
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    // we keep the attachment stored when the renderpass ends
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    //we don't care about stencil
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    //we don't know or care about the starting layout of the attachment
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    //after the renderpass ends, the image has to be on a layout ready for display
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    
    VkAttachmentReference colorAttachmentReference = {};
    //attachment number will index into the pAttachments array in the parent renderpass itself
    colorAttachmentReference.attachment = 0;
    colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    //we are going to create 1 subpass, which is the minimum you can do
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentReference;
    
    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

    //connect the color attachment to the info
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    //connect the subpass to the info
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    
    VK_CHECK(vkCreateRenderPass(device, &renderPassInfo, VK_NULL_HANDLE, &renderPass));
}

void Engine::initializeFramebuffers() {
    //create the framebuffers for the swapchain images. This will connect the render-pass to the images for rendering
    VkFramebufferCreateInfo framebufferCreateInfo = {};
    framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferCreateInfo.pNext = VK_NULL_HANDLE;

    framebufferCreateInfo.renderPass = renderPass;
    framebufferCreateInfo.attachmentCount = 1;
    framebufferCreateInfo.width = settings.windowExtent.width;
    framebufferCreateInfo.height = settings.windowExtent.height;
    framebufferCreateInfo.layers = 1;

    //grab how many images we have in the swapchain
    const uint32_t swapchain_imagecount = swapchainImages.size();
    framebuffers = std::vector<VkFramebuffer>(swapchain_imagecount);

    //create framebuffers for each of the swapchain image views
    for (int i = 0; i < swapchain_imagecount; i++) {
        framebufferCreateInfo.pAttachments = &swapchainImageViews[i];
        VK_CHECK(vkCreateFramebuffer(device, &framebufferCreateInfo, VK_NULL_HANDLE, &framebuffers[i]));
    }
}

void Engine::initializeSyncStructures() {
    //create syncronization structures
        
    VkFenceCreateInfo fenceCreateInfo = {};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.pNext = VK_NULL_HANDLE;

    //we want to create the fence with the Create Signaled flag, so we can wait on it before using it on a GPU command (for the first frame)
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VK_CHECK(vkCreateFence(device, &fenceCreateInfo, nullptr, &renderFence));

    //for the semaphores we don't need any flags
    VkSemaphoreCreateInfo semaphoreCreateInfo = {};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphoreCreateInfo.pNext = VK_NULL_HANDLE;
    semaphoreCreateInfo.flags = 0;

    VK_CHECK(vkCreateSemaphore(device, &semaphoreCreateInfo, VK_NULL_HANDLE, &presentSemaphore));
    VK_CHECK(vkCreateSemaphore(device, &semaphoreCreateInfo, VK_NULL_HANDLE, &renderSemaphore));
}
