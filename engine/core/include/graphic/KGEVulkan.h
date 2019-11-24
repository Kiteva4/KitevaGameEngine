#ifndef KGEVULKAN_H
#define KGEVULKAN_H

#include <vector>

#define ENABLE_DEBUG_LAYERS

#include "vulkan/vulkan.h"
#include "vulkan/vk_sdk_platform.h"

#include "KGEutilityl.h"


#define CHECK_VULKAN_ERROR(msg, res)    \
    if (res != VK_SUCCESS) {            \
        OGLDEV_ERROR(msg, res);         \
        abort();                        \
    }

struct VulkanPhysicalDevices {
    std::vector<VkPhysicalDevice> m_devices;
    std::vector<VkPhysicalDeviceProperties> m_devProps;
    std::vector< std::vector<VkQueueFamilyProperties> > m_qFamilyProps;
    std::vector< std::vector<VkBool32> > m_qSupportsPresent;
    std::vector< std::vector<VkSurfaceFormatKHR> > m_surfaceFormats;
    std::vector<VkSurfaceCapabilitiesKHR> m_surfaceCaps;
};

void VulkanEnumExtProps(std::vector<VkExtensionProperties>& ExtProps);
void VulkanPrintImageUsageFlags(const VkImageUsageFlags& flags);
VkShaderModule VulkanCreateShaderModule(VkDevice& device, const char* pFileName);
void VulkanGetPhysicalDevices(const VkInstance& inst, const VkSurfaceKHR& surface, VulkanPhysicalDevices& physDevices);

#endif
