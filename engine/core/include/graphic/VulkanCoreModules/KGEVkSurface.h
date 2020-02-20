#ifndef KGEVULKANSURFACE_H
#define KGEVULKANSURFACE_H
#include <vulkan/vulkan.h>
#include <graphic/VulkanWindowControl/IVulkanWindowControl.h>

class KGEVkSurface
{
    VkSurfaceKHR m_surface;
    VkInstance m_instance;
public:

    KGEVkSurface();
    /**
    * Инициализация поверхности отображения (поверхность окна)
    * @param VkInstance vkInstance - хендл экземпляра Vulkan
    * @return VkSurfaceKHR - хендл созданной поверхности
    */
    KGEVkSurface(IVulkanWindowControl *windowControl, VkInstance instance);
    ~KGEVkSurface();
    VkSurfaceKHR surface() const;
};

#endif // KGEVULKANSURFACE_H
