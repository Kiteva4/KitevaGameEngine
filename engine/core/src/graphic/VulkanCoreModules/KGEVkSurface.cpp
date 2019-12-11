#include "graphic/VulkanCoreModules/KGEVkSurface.h"

KGEVkSurface::KGEVkSurface():
    m_vkSurface{nullptr},
    m_vkInstance{nullptr}
{

}

KGEVkSurface::KGEVkSurface(VkSurfaceKHR* vkSurface, IVulkanWindowControl *windowControl, VkInstance vkInstance) :
    m_vkSurface{vkSurface},
    m_vkInstance{vkInstance}
{
    *m_vkSurface = windowControl->CreateSurface(m_vkInstance);
}

KGEVkSurface::~KGEVkSurface()
{
    if(m_vkSurface != nullptr & (*m_vkSurface) != nullptr){
        vkDestroySurfaceKHR(m_vkInstance, *m_vkSurface, nullptr);
    }
}
