#include "graphic/VulkanCoreModules/KGEVkSurface.h"

const VkSurfaceKHR &KGEVkSurface::vkSurface()
{
    return m_vkSurface;
}

KGEVkSurface::KGEVkSurface():
    m_vkSurface{nullptr},
    m_vkInstance{nullptr}
{

}

KGEVkSurface::KGEVkSurface(IVulkanWindowControl *windowControl, VkInstance vkInstance) :
    m_vkInstance{vkInstance}
{
    m_vkSurface = windowControl->CreateSurface(m_vkInstance);
}

KGEVkSurface::~KGEVkSurface()
{
    if(m_vkSurface != nullptr & (&m_vkSurface) != nullptr){
        vkDestroySurfaceKHR(m_vkInstance, m_vkSurface, nullptr);
    }
}
