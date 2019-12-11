#ifndef KGEVULKANSURFACE_H
#define KGEVULKANSURFACE_H
#include <vulkan/vulkan.h>
#include <graphic/VulkanWindowControl/IVulkanWindowControl.h>

class KGEVkSurface
{
    VkSurfaceKHR m_vkSurface;
    VkInstance m_vkInstance;
public:

    KGEVkSurface();
    /**
    * Инициализация поверхности отображения (поверхность окна)
    * @param VkInstance vkInstance - хендл экземпляра Vulkan
    * @return VkSurfaceKHR - хендл созданной поверхности
    */
    KGEVkSurface(IVulkanWindowControl *windowControl, VkInstance vkInstance);
    ~KGEVkSurface();
    const VkSurfaceKHR& vkSurface();
};

#endif // KGEVULKANSURFACE_H
