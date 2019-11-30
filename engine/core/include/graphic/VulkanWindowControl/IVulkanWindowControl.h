#ifndef IVULKANWINDOWCONTROL_H
#define IVULKANWINDOWCONTROL_H

#include "vulkan/vulkan.h"
#include "vulkan/vk_sdk_platform.h"

#include "KGEutilityl.h"

class IVulkanWindowControl
{
protected:
    IVulkanWindowControl() {}



public:
    virtual ~IVulkanWindowControl() {}

    virtual void Init(uint32_t Width, uint32_t Height) = 0;

    virtual VkSurfaceKHR CreateSurface(VkInstance& vkInstance) = 0;
};
#endif // IVULKANWINDOWCONTROL_H
