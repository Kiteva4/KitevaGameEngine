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
                extensions,
                { "VK_LAYER_LUNARG_standard_validation" });

    m_vkSurface = InitWindowSurface(windowControl);

    m_device = InitDevice(
                m_vkInstance, m_vkSurface,
                { VK_KHR_SWAPCHAIN_EXTENSION_NAME },
                { "VK_LAYER_LUNARG_standard_validation" },
                false);
}

KGEVulkanCore::~KGEVulkanCore()
{

    DeinitDevice(&m_device);
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

kge::vkstructs::Device KGEVulkanCore::InitDevice(VkInstance vkInstance,
                                                 VkSurfaceKHR surface,
                                                 std::vector<const char *> extensionsRequired,
                                                 std::vector<const char *> validationLayersRequired,
                                                 bool uniqueQueueFamilies)
{
    kge::vkstructs::Device resultDevice = {};

    //Проверяем количество доступных физических устройств
    unsigned int deviceCount = 0;
    vkEnumeratePhysicalDevices(vkInstance, &deviceCount, nullptr);

    if(deviceCount == 0){
        throw std::runtime_error("Vulkan: Can`t detect device with Vulkan support");
    }

    //Есть есть поддерживаемые устройства находим их и проходим по ним
    std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
    vkEnumeratePhysicalDevices(vkInstance, &deviceCount, physicalDevices.data());

    for(const auto& physicalDevice : physicalDevices){

        resultDevice.queueFamilies = kge::vkutility::GetQueueFamilyInfo(
                    physicalDevice,
                    surface,
                    uniqueQueueFamilies);

        //Поддерживает ли набор рендеринг
        if(!resultDevice.queueFamilies.IsRenderingCompatible()){
            continue;
        }

        //Проверяем поддержку запрошенных расширений
        if(!extensionsRequired.empty() && !kge::vkutility::CheckDeviceExtensionSupported(physicalDevice, extensionsRequired)){
            continue;
        }

        //Получаем информацию о поверхности
        kge::vkstructs::SurfaceInfo si = kge::vkutility::GetSurfaceInfo(physicalDevice, surface);
        //Если пусты форматы или представления для данного физического устройства
        if(si.formats.empty() || si.presentModes.empty()){
            continue;
        }

        //Все проверки пройдены, в хендл физического устройства присваиваем данный хендл который проверяем
        resultDevice.physicalDevice = physicalDevice;
    }

    //если в результирующем девайсе physicalDevice == nullptr
    if(resultDevice.physicalDevice == nullptr){
        throw std::runtime_error("Vulkan: Error in the 'InitDevice' function! Cant detect device");
    }

    //Структуры, которые описывают очереди, которые мы собираемся в дальнейшем получить от устройства после того как его создадим
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

    //Массивы индексов семейств очередей, очереди которых мы хотим в дальнейшем получить
    uint32_t queueFamilies[2] = {
        static_cast<uint32_t>(resultDevice.queueFamilies.graphics),
        static_cast<uint32_t>(resultDevice.queueFamilies.present)
    };

    //TODO
    for(unsigned int i = 0; i < (uniqueQueueFamilies ? 2 : 1); i++){

        VkDeviceQueueCreateInfo queueCreateInfo = {};

        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamilies[i];
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = nullptr;

        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkDeviceCreateInfo deviceCreateInfo = {};

    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
    deviceCreateInfo.queueCreateInfoCount = static_cast<unsigned int>(queueCreateInfos.size());

    if(!extensionsRequired.empty()){
        if(!kge::vkutility::CheckDeviceExtensionSupported(resultDevice.physicalDevice, extensionsRequired)){
            throw std::runtime_error("Vulkan: Not all required device extensions supported. Error!");
        }

        deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(extensionsRequired.size());
        deviceCreateInfo.ppEnabledExtensionNames = extensionsRequired.data();
    }

    if(!validationLayersRequired.empty()){
        if(!kge::vkutility::checkValidationLayersSupported(validationLayersRequired)) {
            throw std::runtime_error("Vulkan: Not all required layers supported. Error!");
        }

        deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayersRequired.size());
        deviceCreateInfo.ppEnabledLayerNames = validationLayersRequired.data();
    }

    VkPhysicalDeviceFeatures deviceFeatures = {};
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

    //Создаем логическое устройство
    if(vkCreateDevice(resultDevice.physicalDevice, &deviceCreateInfo, nullptr, &resultDevice.logicalDevice) != VK_SUCCESS  ){
        throw std::runtime_error("Vulkan: Failed to create logical device.");
    }

    //Получение очереди логичнского устройства для графических команд
    vkGetDeviceQueue(
                resultDevice.logicalDevice, //Хендл логического устройства
                static_cast<uint32_t>(resultDevice.queueFamilies.graphics), //Индекс семейства очередей
                0,  //Индекс очереди - первая
                &resultDevice.queues.graphics //Указатель на переменную, в которую будет записан хендл очереди
                );
    //Получение очереди логичнского устройства для представлений
    vkGetDeviceQueue(
                resultDevice.logicalDevice, //Хендл логического устройства
                static_cast<uint32_t>(resultDevice.queueFamilies.present), //Индекс семейства очередей
                0,  //Индекс очереди - первая
                &resultDevice.queues.present //Указатель на переменную, в которую будет записан хендл очереди
                );

    if(!resultDevice.IsReady()){
        throw std::runtime_error("Vulkan: Failed to initialize devicd");
    }

    std::cout << "Vulkan: Device successfully initialized ("
              << resultDevice.GetProperties().deviceName << ")" <<std::endl;

    std::cout << "           deviceID " << resultDevice.GetProperties().deviceID
              << "           vendorID " << resultDevice.GetProperties().vendorID
              << "         apiVersion " << resultDevice.GetProperties().apiVersion
              << "         deviceType " << resultDevice.GetProperties().deviceType
              << "      driverVersion " << resultDevice.GetProperties().driverVersion
              << "  pipelineCacheUUID " << resultDevice.GetProperties().pipelineCacheUUID
              << std::endl;

    return resultDevice;
}

void KGEVulkanCore::DeinitDevice(kge::vkstructs::Device *device)
{
    device->Deinit();
    std::cout << "Vulkan: Device successfully destroyd" << std::endl;
}
