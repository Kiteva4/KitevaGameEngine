#ifndef KGEVULKANCORE_H
#define KGEVULKANCORE_H


#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <vulkan/vk_sdk_platform.h>
#include <graphic/KGEVulkan.h>
#include <graphic/VulkanWindowControl/IVulkanWindowControl.h>
#include <string>

class KGEVulkanCore
{
public:
    KGEVulkanCore(const char* appName);
    ~KGEVulkanCore();

    void Init(IVulkanWindowControl* pWindowControl);

    const VkPhysicalDevice& GetPhysDevice() const;

    const VkSurfaceFormatKHR& GetSurfaceFormat() const;

    const VkSurfaceCapabilitiesKHR GetSurfaceCaps() const;

    const VkSurfaceKHR& GetSurface() const { return m_surface; }

    uint32_t GetQueueFamily() const { return m_gfxQueueFamily; }

    VkInstance& GetInstance() { return m_inst; }

    VkDevice& GetDevice() { return m_device; }

    VkSemaphore CreateSemaphore();

private:
    void CreateInstance();
    void CreateSurface();
    void SelectPhysicalDevice();
    void CreateLogicalDevice();

    // Vulkan objects
    VkInstance m_inst;
    VkSurfaceKHR m_surface;
    VulkanPhysicalDevices m_physDevices;
    VkDevice m_device;

    // Internal stuff
    std::string m_appName;
    int m_gfxDevIndex;
    uint32_t m_gfxQueueFamily;
};

#endif // KGEVULKANCORE_H
