#include "graphic/KGEVulkanCore.h"


KGEVulkanCore::KGEVulkanCore(uint32_t width,
                             uint32_t heigh,
                             IVulkanWindowControl* windowControl,
                             std::vector <const char*> extensions) :
    m_width(width),
    m_heigh(heigh),
    m_vkInstance(nullptr),
    m_validationReportCallback(nullptr),
    m_vkSurface(nullptr)
{
    m_vkInstance = InitVkInstance(
        "Application name",
        "Engine name",
        extensions,//{ VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME, VK_EXT_DEBUG_REPORT_EXTENSION_NAME },
        { "VK_LAYER_LUNARG_standard_validation" });

    m_vkSurface = InitWindowSurface(windowControl);
}

KGEVulkanCore::~KGEVulkanCore()
{

    DeinitWindowSurface(m_vkInstance, &m_vkSurface);
    DeinitInstance(&m_vkInstance);
}

VkInstance KGEVulkanCore::InitVkInstance(
    std::string applicationName,
    std::string engineName,
    std::vector<const char*> extensionsRequired,
    std::vector<const char*> validationLayersRequired)
{
    VkApplicationInfo applicationInfo = {};
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pApplicationName = applicationName.c_str();
    applicationInfo.pEngineName = engineName.c_str();
    applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo instanceCreateInfo = {};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &applicationInfo;

    bool validationQueried = false;
    //Если есть запрашиваемые расширения инстанса
    if (!extensionsRequired.empty())
    {
        if (!kge::vkutility::checkInstanceExtensionsSupported(extensionsRequired)) {
            throw std::runtime_error("Vulkan: Not supported required instance extensions!");
        }

        instanceCreateInfo.ppEnabledExtensionNames = extensionsRequired.data();
        instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(extensionsRequired.size());

        //Проверка на наличие запрошенного расширения Debug
        bool debugReportExtensionQueried = false;

        for (auto* extensionName : extensionsRequired)
        {
            if (strcmp(VK_EXT_DEBUG_REPORT_EXTENSION_NAME, extensionName) == 0) {
                debugReportExtensionQueried = true;
                break;
            }
        }

        //Было запрошено расширения
        if (debugReportExtensionQueried && !validationLayersRequired.empty()) {

            //Есть слои валидации
            if (!kge::vkutility::checkValidationLayersSupported(validationLayersRequired)) {
                throw std::runtime_error("Vulkan: Not all required layers supporeted!");
            }

            instanceCreateInfo.enabledLayerCount = (uint32_t)validationLayersRequired.size();
            instanceCreateInfo.ppEnabledLayerNames = validationLayersRequired.data();

            validationQueried = true;
            std::cout << "Vulkan: Validation enabled"  << std::endl;
        }
    }

    VkInstance vkInstance;

    if (vkCreateInstance(&instanceCreateInfo, nullptr, &(vkInstance)) != VK_SUCCESS) {
        throw std::runtime_error("Vulkan: Error in the 'vkCreateInstance' function");
    }

    std::cout << "Vulkan: Instance sucessfully created" << std::endl;

    if (validationQueried){
        VkDebugReportCallbackCreateInfoEXT debugReportCallbackCreateInfo = {};
        debugReportCallbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
        debugReportCallbackCreateInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
        debugReportCallbackCreateInfo.pfnCallback = kge::vkutility::DebugVulkanCallback; //????????? ?? ???????, ??????? ????? ?????????? ??? ??????????? ??????

        //Получаем адрес функции создания обьекта
        PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT =
            (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(
                vkInstance, "vkCreateDebugReportCallbackEXT");

        //Если получили адрес и успешно создали
        if (vkCreateDebugReportCallbackEXT(vkInstance, &debugReportCallbackCreateInfo, nullptr, &(this->m_validationReportCallback)) != VK_SUCCESS){
            throw std::runtime_error("Vulkan: Error in the 'vkCreateDebugReportCallbackEXT'");
        }

        std::cout << "Vulkan: Report callback sucessfully created" << std::endl;
    }

    return vkInstance;
}

void KGEVulkanCore::DeinitInstance(VkInstance* vkInstance)
{
    //Если был создан обьект m_validationReportCallback
    if (m_validationReportCallback != nullptr) {
        //Получаем адрес функции для его уничтожения
        PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT =
            (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(
            *vkInstance, "vkCreateDebugReportCallbackEXT");

        vkDestroyDebugReportCallbackEXT(
            *vkInstance,
            this->m_validationReportCallback,
            nullptr);

        this->m_validationReportCallback = nullptr;

        std::cout << "Vulkan: Report callback sucessfully destroyed" << std::endl;
    }

    if (*vkInstance != nullptr) {
        vkDestroyInstance(*vkInstance, nullptr);
        *vkInstance = nullptr;

        std::cout << "Vulkan: Instance sucessfully destroyed" << std::endl;
    }
}

VkSurfaceKHR KGEVulkanCore::InitWindowSurface(IVulkanWindowControl *windowControl)
{
    return windowControl->CreateSurface(m_vkInstance);
}

void KGEVulkanCore::DeinitWindowSurface(VkInstance vkInstance, VkSurfaceKHR *surface)
{
    if(surface != nullptr & *surface != nullptr){
        vkDestroySurfaceKHR(vkInstance, *surface, nullptr);
    }
}
