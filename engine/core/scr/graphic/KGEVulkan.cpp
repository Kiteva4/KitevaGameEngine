#include "graphic/KGEVulkan.h"

//Проверка на наличие запрашиваемых расширений у инстанса
bool kge::vkutility::checkInstanceExtensionsSupported(std::vector<const char*> instanceExtensionsNames)
{

    std::cout << "Vulkan: Start require  extensions" << std::endl;
    //вектор для хранения доступных расширений инстанса
    std::vector<VkExtensionProperties> avialableExtensions;

    //количество доступных расширений инстанса
    unsigned int instanceExtensionsCount = 0;

    //Получаем количество доступных расширений инстанса
    vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionsCount, nullptr);

    //если отсутствуют доступные расширения - возвращаем false
    if (instanceExtensionsCount == 0) {
        return false;
    }

    //Инициализируем хранилище расширений количеством доступных расширений инстансом
    avialableExtensions.resize(instanceExtensionsCount);

    //Получаем все доступные расширения
    vkEnumerateInstanceExtensionProperties(
                nullptr,
                &instanceExtensionsCount,
                avialableExtensions.data());

    //  Проходим по запрашиваемым расширениям
    for (auto* requiredExtName : instanceExtensionsNames)
    {
        bool found = false;

        //Проходим по всем доступным расширениям
        for (auto& extProperties : avialableExtensions) {
            //Если доступен запрашиваемый
            if (strcmp(requiredExtName, extProperties.extensionName) == 0) {
                found = true;
                std::cout << "Vulkan: Extension: "<<  requiredExtName << " founded "  << std::endl;
                break;
            }
        }

        //Не найдено какое то расширение
        if (!found)
            return false;
    }

    std::cout << "Vulkan: all required extensions found successfully!" << std::endl;
    //Все расширения найдены
    return true;
}

//Проверка на наличие запрашиваемых слолев валидации
bool kge::vkutility::checkValidationLayersSupported(std::vector<const char*> instanceVLayersNames)
{

    std::cout << "Vulkan: Start require  layers" << std::endl;

    //вектор для хранения доступных слоев валидации
    std::vector<VkLayerProperties> avialableLayers;

    //количество доступных слоев валидации инстанса
    unsigned int instanceLayersCount = 0;

    //Получаем количество доступных слоев валидации инстанса
    vkEnumerateInstanceLayerProperties(&instanceLayersCount, nullptr);

    //если отсутствуют доступные слои валидации - возвращаем false
    if (instanceLayersCount == 0) {
        return false;
    }

    //Инициализируем хранилище слоев валидации количеством доступных расширений инстансом
    avialableLayers.resize(instanceLayersCount);

    //Получаем все доступные слои валидации
    vkEnumerateInstanceLayerProperties(&instanceLayersCount, avialableLayers.data());

    //Проходим по запрашиваемым слоям валидации
    for (auto* requiredVLayerName : instanceVLayersNames)
    {
        bool found = false;

        //Проходим по всем доступным слоям валидации
        for (auto& vLayerProperties : avialableLayers) {
            //Если доступен запрашиваемый слой валидации
            if (strcmp(requiredVLayerName, vLayerProperties.layerName) == 0) {
                found = true;
                std::cout << "Vulkan: Layer: "<<  requiredVLayerName << " founded "  << std::endl;
                break;
            }
        }

        //Не найден какой то из запрашиваемых слоев
        if (!found) {
            return false;
        }
    }

    std::cout << "Vulkan: all required layers found successfully!" << std::endl;
    //Все запрашиваемые слои найдены
    return true;
}

//Функция, которая будет вызываться слоем валидации при обнаружении ошибок
VKAPI_ATTR VkBool32 VKAPI_CALL kge::vkutility::DebugVulkanCallback(
        VkDebugReportFlagsEXT flags,
        VkDebugReportObjectTypeEXT objType,
        uint64_t obj,
        size_t location,
        int32_t code,
        const char* layerPrefix,
        const char* msg,
        void* userData)
{
    std::cout << "Vulkan validation layer :" << std::endl;
    return VK_FALSE;
}

bool kge::vkutility::CheckDeviceExtensionSupported(
        VkPhysicalDevice physicalDevice,
        std::vector<const char *> deviceExtensionsNames)
{
    std::vector <VkExtensionProperties> aviableExtensions;
    uint32_t deviceExtensionCount = 0;
    vkEnumerateDeviceExtensionProperties(
                physicalDevice,
                nullptr,
                &deviceExtensionCount,
                nullptr);

    if(deviceExtensionCount == 0) {
        return false;
    }

    aviableExtensions.resize(deviceExtensionCount);

    vkEnumerateDeviceExtensionProperties(
                physicalDevice,
                nullptr,
                &deviceExtensionCount,
                aviableExtensions.data());

    for( const char* requiredExtName : deviceExtensionsNames){
        bool found = false;
        for(const auto& extProperties : aviableExtensions){
            if(strcmp(requiredExtName, extProperties.extensionName) == 0) {
                found = true;
                break;
            }
        }

        if(!found){
            return false;
        }
    }

    return true;
}

//Тест тест
kge::vkstructs::QueueFamilyInfo kge::vkutility::GetQueueFamilyInfo(
        VkPhysicalDevice physicalDevice,
        VkSurfaceKHR surface,
        bool uniqueStrict)
{
    /* */
    kge::vkstructs::QueueFamilyInfo qFamilyInfo;
    /* */
    /**/
    uint32_t queuFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queuFamilyCount, nullptr);

    /* */
    std::vector<VkQueueFamilyProperties> queueFamilies(queuFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queuFamilyCount, queueFamilies.data());

    /* */
    for(unsigned int i = 0; i < queueFamilies.size(); i++){
        //
        if(queueFamilies[i].queueCount > 0 && queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT){
            qFamilyInfo.graphics = i;
            break;
        }
    }

    /**/
    for(unsigned int i = 0; i < queueFamilies.size(); i++) {
        //
        if(static_cast<int>(i) == qFamilyInfo.graphics && uniqueStrict){
            continue;
        }

        unsigned int presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);

        if(queueFamilies[i].queueCount > 0 && presentSupport){
            qFamilyInfo.present = static_cast<int>(i);
            break;
        }
    }

    return qFamilyInfo;
}



kge::vkstructs::SurfaceInfo kge::vkutility::GetSurfaceInfo(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
    kge::vkstructs::SurfaceInfo surfaceInfo;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &(surfaceInfo.capabilities));

    unsigned int formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);

    if(formatCount > 0) {
        surfaceInfo.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, surfaceInfo.formats.data());
    }

    unsigned int presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);

    if(presentModeCount > 0){
        surfaceInfo.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, surfaceInfo.presentModes.data());
    }

    return surfaceInfo;
}
