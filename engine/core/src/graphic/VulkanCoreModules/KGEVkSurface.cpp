#include "graphic/VulkanCoreModules/KGEVkSurface.h"

VkSurfaceKHR KGEVkSurface::surface() const
{
    return m_surface;
}

KGEVkSurface::KGEVkSurface():
    m_surface{nullptr},
    m_instance{nullptr}
{

}

KGEVkSurface::KGEVkSurface(IVulkanWindowControl *windowControl, VkInstance instance) :
    m_instance{instance}
{
    m_surface = windowControl->CreateSurface(m_instance);
}

KGEVkSurface::~KGEVkSurface()
{
    if(m_surface != nullptr & (&m_surface) != nullptr){
        vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
    }
}
