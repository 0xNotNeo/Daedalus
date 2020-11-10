#pragma once

#include <vulkan/vulkan.hpp>

#include "CommandBuffers.hpp"

namespace ddls
{
const std::vector<uint16_t> indices = {
    0, 1, 2, 2, 3, 0,
    4, 5, 6, 6, 7, 4
};

void createIndexBuffers(VkPhysicalDevice physicalDevice, VkDevice device, VkBuffer& indexBuffer, VkDeviceMemory& indexBufferMemory, VkCommandPool& commandPool, VkQueue graphicsQueue);
}
