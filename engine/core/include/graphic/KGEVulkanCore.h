#ifndef KGEVULKANCORE_H
#define KGEVULKANCORE_H

#include <string>
#include <vector>
#include <iostream>
#include <graphic/KGEVulkan.h>
#include <graphic/VulkanWindowControl/IVulkanWindowControl.h>

#ifdef NDEBUG
const bool DEBUG_MODE = false;
#else
const bool DEBUG_MODE = true;
#endif


class KGEVulkanCore
{
public:
    KGEVulkanCore(uint32_t width, uint32_t heigh, IVulkanWindowControl* windowControl);
    ~KGEVulkanCore();
private:

    uint32_t m_width;
    uint32_t m_heigh;

    VkInstance m_vkInstance{};
    //Инициализация инстанса
    VkInstance InitVkInstance(
            std::string appName,                                /*! Наименование приложения         */
            std::string engineName,                             /*! Наименование среды(движка)      */
            std::vector<const char*> extensionsRequired,        /*! Список запрашиваемых расширений */
            std::vector<const char*> validationLayersRequired); /*! Список слоев валидации          */

    //Деинициализация инстанса
    void DeinitInstance(VkInstance* vkInstance);

    VkDebugReportCallbackEXT m_validationReportCallback;

    VkSurfaceKHR m_vkSurface;
    VkSurfaceKHR InitWindowSurface(IVulkanWindowControl* windowControl);
    void DeinitWindowSurface(VkInstance vkInstance, VkSurfaceKHR * surface);
};


#endif // KGEVULKANCORE_H
