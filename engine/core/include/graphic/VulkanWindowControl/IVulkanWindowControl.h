#ifndef IVULKANWINDOWCONTROL_H
#define IVULKANWINDOWCONTROL_H

#include "vulkan/vulkan.h"
#include "vulkan/vk_sdk_platform.h"

#include "KGEutilityl.h"

/*! @brief Класс декорации управления оконной поверхности */
class IVulkanWindowControl
{
protected:
    IVulkanWindowControl() {}

    ~IVulkanWindowControl() {}

public:

    virtual void Init(unsigned int Width, unsigned int Height) = 0;

    virtual VkSurfaceKHR CreateSurface(VkInstance& inst) = 0;
};
#endif // IVULKANWINDOWCONTROL_H
