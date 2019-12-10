#ifndef KGEVKINSTANCE_H
#define KGEVKINSTANCE_H

#include <string>
#include <vector>

#include <vulkan/vulkan.h>

class KGEVkInstance
{   
    VkInstance m_vkInstance;
    VkDebugReportCallbackEXT m_validationReportCallback;

public:
    KGEVkInstance(std::string applicationName,
                  std::string engineName,
                  std::vector<const char*> extensionsRequired,
                  std::vector<const char*> validationLayersRequired);
    ~KGEVkInstance();
    const VkInstance& vkInstance();
    const VkDebugReportCallbackEXT& validationReportCallback();
};

#endif // KGEVKINSTANCE_H

/**
* Метод инициализации instance'а vulkan'а
* @param std::string applicationName - наименование приложения (информация может быть полезна разработчикам драйверов)
* @param std::string engineName - наименование движка (информация может быть полезна разработчикам драйверов)
* @param std::vector<const char*> extensionsRequired - запрашиваемые расширения экземпляра
* @param std::vector<const char*>validationLayersRequired - запрашиваемые слои валидации
* @return VkInstance - хендл экземпляра Vulkan
*/
//VkInstance KGEVulkanCore::InitInstance(
//        std::string applicationName,
//        std::string engineName,
//        std::vector<const char*> extensionsRequired,
//        std::vector<const char*> validationLayersRequired)
//{
//    // Структура с информацией о создаваемом приложении
//    // Здесь содержиться информация о названии, версии приложения и движка. Эта информация может быть полезна разработчикам драйверов
//    VkApplicationInfo applicationInfo = {};
//    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
//    applicationInfo.pApplicationName = applicationName.c_str();
//    applicationInfo.pEngineName = engineName.c_str();
//    applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
//    applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
//    applicationInfo.apiVersion = VK_API_VERSION_1_0;

//    // Структура с информацией о создаваемом экземпляре vulkan
//    // Здесь можно указать информацию о приложении (ссылка на структуру выше) а так же указать используемые расширения
//    VkInstanceCreateInfo instanceCreateInfo = {};
//    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
//    instanceCreateInfo.pApplicationInfo = &applicationInfo;

//    bool validationQueried = false;
//    // Если запрашиваются расширения
//    if (!extensionsRequired.empty())
//    {
//        // Если не все запрашиваемые расширения доступны - ошибка
//        if (!kge::vkutility::CheckInstanceExtensionsSupported(extensionsRequired)) {
//            throw std::runtime_error("Vulkan: Not supported required instance extensions!");
//        }

//        // Указать запрашиваемые расширения и их кол-во
//        instanceCreateInfo.ppEnabledExtensionNames = extensionsRequired.data();
//        instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(extensionsRequired.size());

//        //Проверка на наличие запрошенного расширения Debug
//        bool debugReportExtensionQueried = false;

//        // Если среди запрашиваемых расш. (которые точно поддерживаются, ппрверка была выше) есть EXT_DEBUG_REPORT
//        // значит расширение для обработки ошибок запрошено (и точно поддерживается)
//        for (auto* extensionName : extensionsRequired)
//        {
//            if (strcmp(VK_EXT_DEBUG_REPORT_EXTENSION_NAME, extensionName) == 0) {
//                debugReportExtensionQueried = true;
//                break;
//            }
//        }

//        // Если запрашивается расширение для обработки ошибок и предупреждений,
//        // есть ссмысл проверить поддержку запрашиваемых слоев валидации
//        if (debugReportExtensionQueried && !validationLayersRequired.empty()) {

//            // Если какие-то слои валидации не поддерживаются - ошибка
//            if (!kge::vkutility::CheckValidationLayersSupported(validationLayersRequired)) {
//                throw std::runtime_error("Vulkan: Not all required layers supporeted!");
//            }
//            // Указать слои валидации и их кол-во
//            instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayersRequired.size());
//            instanceCreateInfo.ppEnabledLayerNames = validationLayersRequired.data();

//            // Валидация запрашивается (поскольку есть и расширение и необходимые слои)
//            validationQueried = true;
//            std::cout << "Vulkan: Validation enabled"  << std::endl;
//        }
//    }

//    VkInstance vkInstance;

//    // Создание экземпляра
//    // Передается указатель на структуру CreateInfo, которую заполнили выше, и указатель на переменную хендла
//    // instance'а, куда и будет помещен сам хендл. Если функция не вернула VK_SUCCESS - ошибка
//    if (vkCreateInstance(&instanceCreateInfo, nullptr, &(vkInstance)) != VK_SUCCESS) {
//        throw std::runtime_error("Vulkan: Error in the 'vkCreateInstance' function");
//    }

//    std::cout << "Vulkan: Instance sucessfully created" << std::endl;

//    // Если запрошена валидация - создать callback метод для обработки исключений
//    if (validationQueried){

//        // Конфигурация callback'а
//        VkDebugReportCallbackCreateInfoEXT debugReportCallbackCreateInfo = {};
//        debugReportCallbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
//        debugReportCallbackCreateInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
//        debugReportCallbackCreateInfo.pfnCallback = kge::vkutility::DebugVulkanCallback; //????????? ?? ???????, ??????? ????? ?????????? ??? ??????????? ??????

//        // Получить адрес функции создания callback'а (поскольку это функция расширения)
//        PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT =
//                (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(
//                    vkInstance, "vkCreateDebugReportCallbackEXT");

//        // Создать debug report callback для вывода сообщений об ошибках
//        if (vkCreateDebugReportCallbackEXT(vkInstance, &debugReportCallbackCreateInfo, nullptr, &m_validationReportCallback) != VK_SUCCESS){
//            throw std::runtime_error("Vulkan: Error in the 'vkCreateDebugReportCallbackEXT'");
//        }

//        std::cout << "Vulkan: Report callback sucessfully created" << std::endl;
//    }

//    return vkInstance;
//}
