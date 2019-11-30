#ifndef KGEVULKANCORE_H
#define KGEVULKANCORE_H

#include <string>
#include <vector>
#include <iostream>
#include <graphic/KGEVulkan.h>
#include <graphic/VulkanWindowControl/IVulkanWindowControl.h>

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif


class KGEVulkanCore
{
public:
    KGEVulkanCore(uint32_t width,
                  uint32_t heigh,
                  IVulkanWindowControl* windowControl,
                  std::vector <const char*> extensions);
    ~KGEVulkanCore();
private:

    uint32_t m_width;
    uint32_t m_heigh;

    /* Instance*/
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

    /* Surface */
    VkSurfaceKHR m_vkSurface;
    VkSurfaceKHR InitWindowSurface(IVulkanWindowControl* windowControl);
    void DeinitWindowSurface(VkInstance vkInstance, VkSurfaceKHR * surface);

    kge::vkstructs::Device m_device;
    kge::vkstructs::Device InitDevice(VkInstance vkInstance,
                                      VkSurfaceKHR surface,
                                      std::vector<const char*> extensionsRequired,
                                      std::vector<const char*> validationLayersRequired,
                                      bool uniqueQueueFamilies);
    void DeinitDevice(kge::vkstructs::Device* device);
};


#endif // KGEVULKANCORE_H
