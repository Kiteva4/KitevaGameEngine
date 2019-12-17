#include "graphic/VulkanCoreModules/KGEVkInstance.h"
#include "graphic/KGEVulkan.h"
#include <cstring>

KGEVkInstance::KGEVkInstance(
        VkInstance  *vkInstance,
        std::string applicationName,
        std::string engineName,
        std::vector<const char*> extensionsRequired,
        std::vector<const char*> validationLayersRequired) :
    m_vkInstance{vkInstance},
    m_validationReportCallback(nullptr)
{
    // Структура с информацией о создаваемом приложении
    // Здесь содержиться информация о названии, версии приложения и движка. Эта информация может быть полезна разработчикам драйверов
    VkApplicationInfo applicationInfo = {};
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pApplicationName = applicationName.c_str();
    applicationInfo.pEngineName = engineName.c_str();
    applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.apiVersion = VK_API_VERSION_1_0;

    // Структура с информацией о создаваемом экземпляре vulkan
    // Здесь можно указать информацию о приложении (ссылка на структуру выше) а так же указать используемые расширения
    VkInstanceCreateInfo instanceCreateInfo = {};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &applicationInfo;

    bool validationQueried = false;

    // Если запрашиваются расширения
    if (!extensionsRequired.empty())
    {
        // Если не все запрашиваемые расширения доступны - ошибка
        if (!kge::vkutility::CheckInstanceExtensionsSupported(extensionsRequired)) {
            throw std::runtime_error("Vulkan: Not supported required instance extensions!");
        }

        // Указать запрашиваемые расширения и их кол-во
        instanceCreateInfo.ppEnabledExtensionNames = extensionsRequired.data();
        instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(extensionsRequired.size());

        //Проверка на наличие запрошенного расширения Debug
        bool debugReportExtensionQueried = false;

        // Если среди запрашиваемых расш. (которые точно поддерживаются, ппрверка была выше) есть EXT_DEBUG_REPORT
        // значит расширение для обработки ошибок запрошено (и точно поддерживается)
        for (auto* extensionName : extensionsRequired)
        {
            if (strcmp(VK_EXT_DEBUG_REPORT_EXTENSION_NAME, extensionName) == 0) {
                debugReportExtensionQueried = true;
                break;
            }
        }

        // Если запрашивается расширение для обработки ошибок и предупреждений,
        // есть ссмысл проверить поддержку запрашиваемых слоев валидации
        if (debugReportExtensionQueried && !validationLayersRequired.empty()) {

            // Если какие-то слои валидации не поддерживаются - ошибка
            if (!kge::vkutility::CheckValidationLayersSupported(validationLayersRequired)) {
                throw std::runtime_error("Vulkan: Not all required layers supporeted!");
            }
            // Указать слои валидации и их кол-во
            instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayersRequired.size());
            instanceCreateInfo.ppEnabledLayerNames = validationLayersRequired.data();

            // Валидация запрашивается (поскольку есть и расширение и необходимые слои)
            validationQueried = true;
            std::cout << "Vulkan: Validation enabled"  << std::endl;
        }
    }

    // Передается указатель на структуру CreateInfo, которую заполнили выше, и указатель на переменную хендла
    // instance'а, куда и будет помещен сам хендл. Если функция не вернула VK_SUCCESS - ошибка
    if (vkCreateInstance(&instanceCreateInfo, nullptr, m_vkInstance) != VK_SUCCESS) {
        throw std::runtime_error("Vulkan: Error in the 'vkCreateInstance' function");
    }

    std::cout << "Vulkan: Instance sucessfully created" << std::endl;

    // Если запрошена валидация - создать callback метод для обработки исключений
    if (validationQueried){

        // Конфигурация callback'а
        VkDebugReportCallbackCreateInfoEXT debugReportCallbackCreateInfo = {};
        debugReportCallbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
        debugReportCallbackCreateInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
        debugReportCallbackCreateInfo.pfnCallback = kge::vkutility::DebugVulkanCallback;

        // Получить адрес функции создания callback'а (поскольку это функция расширения)
        PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT =
                reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(vkGetInstanceProcAddr(*m_vkInstance, "vkCreateDebugReportCallbackEXT"));

        // Создать debug report callback для вывода сообщений об ошибках
        if (vkCreateDebugReportCallbackEXT(*m_vkInstance, &debugReportCallbackCreateInfo, nullptr, &m_validationReportCallback) != VK_SUCCESS){
            throw std::runtime_error("Vulkan: Error in the 'vkCreateDebugReportCallbackEXT'");
        }

        std::cout << "Vulkan: Report callback sucessfully created" << std::endl;
    }
}

KGEVkInstance::~KGEVkInstance()
{
    //Если был создан обьект m_validationReportCallback
    if (m_validationReportCallback != nullptr) {
        //Получаем адрес функции для его уничтожения
        PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(vkGetInstanceProcAddr(*m_vkInstance, "vkCreateDebugReportCallbackEXT"));

        vkDestroyDebugReportCallbackEXT(
                    *m_vkInstance,
                    m_validationReportCallback,
                    nullptr);

        m_validationReportCallback = nullptr;

        std::cout << "Vulkan: Report callback sucessfully destroyed" << std::endl;
    }

    if (*m_vkInstance != nullptr || m_vkInstance != nullptr) {
        vkDestroyInstance(*m_vkInstance, nullptr);
        m_vkInstance = nullptr;

        std::cout << "Vulkan: Instance sucessfully destroyed" << std::endl;
    }
}

const VkDebugReportCallbackEXT &KGEVkInstance::validationReportCallback()
{
    return m_validationReportCallback;
}

