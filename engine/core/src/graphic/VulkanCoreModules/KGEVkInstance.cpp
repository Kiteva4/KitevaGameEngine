#include "graphic/VulkanCoreModules/KGEVkInstance.h"
#include "graphic/KGEVulkan.h"
#include <cstring>

const VkInstance & KGEVkInstance::instance()
{
    return m_instance;
}

KGEVkInstance::KGEVkInstance(std::string applicationName,
                             std::string engineName,
        std::vector<const char*> extensionsRequired,
        std::vector<const char*> validationLayersRequired) :
    m_instance{}
{
    // Структура с информацией о создаваемом приложении
    // Здесь содержиться информация о названии, версии приложения и движка. Эта информация может быть полезна разработчикам драйверов
    VkApplicationInfo applicationInfo = {};
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pNext = nullptr;
    applicationInfo.pApplicationName = applicationName.c_str();
    applicationInfo.pEngineName = engineName.c_str();
    applicationInfo.applicationVersion = 1;//VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.engineVersion = 1;//VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.apiVersion = VK_API_VERSION_1_0;

    // Структура с информацией о создаваемом экземпляре vulkan
    // Здесь можно указать информацию о приложении (ссылка на структуру выше) а так же указать используемые расширения
    VkInstanceCreateInfo instanceCreateInfo = {};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pNext = nullptr;
    instanceCreateInfo.flags = 0;
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
        instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(extensionsRequired.size());
        instanceCreateInfo.ppEnabledExtensionNames = extensionsRequired.data();

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
    if (vkCreateInstance(&instanceCreateInfo, nullptr, &m_instance) != VK_SUCCESS) {
        throw std::runtime_error("Vulkan: Error in the 'vkCreateInstance' function");
    }

    std::cout << "Vulkan: Instance sucessfully created" << std::endl;
}

KGEVkInstance::~KGEVkInstance()
{
    if (m_instance != nullptr || m_instance != nullptr) {
        vkDestroyInstance(m_instance, nullptr);
        m_instance = nullptr;

        std::cout << "Vulkan: Instance sucessfully destroyed" << std::endl;
    }
}
