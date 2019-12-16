#include "graphic/VulkanCoreModules/KGEVkDevice.h"

/**
* Инициализации устройства. Поиск подходящего физ. устройства, создание логического на его основе.
* @param kge::vkstructs::Device *device - указатель на структуру с хендлами устройства
* @param VkInstance vkInstance - хендл экземпляра Vulkan
* @param VkSurfaceKHR surface - хендл поверхности для которой будет проверяться поддержка необходимых очередей устройства
* @param std::vector<const char*> extensionsRequired - запрашиваемые расширения устройства
* @param std::vector<const char*> validationLayersRequired - запрашиваемые слои валидации
* @param bool uniqueQueueFamilies - нужны ли уникальные семейства очередей (разные) для показа и представления
*/
KGEVkDevice::KGEVkDevice(
        kge::vkstructs::Device *device,
        VkInstance vkInstance,
        VkSurfaceKHR surface,
        std::vector<const char *> deviceExtensionsRequired,
        std::vector<const char *> validationLayersRequired,
        bool uniqueQueueFamilies):
    m_device{device}
{
    //Проверяем количество доступных физических устройств
    unsigned int deviceCount = 0;
    vkEnumeratePhysicalDevices(vkInstance, &deviceCount, nullptr);

    std::cout << "Vulkan: Support device count " << deviceCount  << std::endl;
    // Если не нашлось видео-карт работающих с vulkan - ошибка
    if(deviceCount == 0){
        throw std::runtime_error("Vulkan: Can`t detect device with Vulkan support");
    }

    //Есть есть поддерживаемые устройства находим их и проходим по ним
    std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
    vkEnumeratePhysicalDevices(vkInstance, &deviceCount, physicalDevices.data());

    // Пройтись по всем видео-картам и проверить каждую на соответствие минимальным требованиям, устройство должно поддерживать:
    // - Запрашиваемые расширения vulkan
    // - Семейтсва очередей поддерживающие отрисовку и представление
    // - Совместимость swap-chain с поверхностью отображения
    for(const auto& physicalDevice : physicalDevices){

        // Получить информацию об очередях поддерживаемых устройством
        m_device->queueFamilies = kge::vkutility::GetQueueFamilyInfo(physicalDevice, surface, uniqueQueueFamilies);

        // Если очереди данного устройства не совместимы с рендерингом - переходим к следующему
        if (!(m_device->queueFamilies.IsRenderingCompatible())) {
            continue;
        }

        // Если данное устройство не поддерживает запрашиваемые расширения - переходим к следующему
        if (deviceExtensionsRequired.size() > 0 && !kge::vkutility::CheckDeviceExtensionSupported(physicalDevice, deviceExtensionsRequired)) {
            continue;
        }

        // Получить информацию о том как устройство может работать с поверхностью
        // Если для поверхности нет форматов и режимов показа (представления) - переходим к след. устройству
        kge::vkstructs::SurfaceInfo si = kge::vkutility::GetSurfaceInfo(physicalDevice, surface);
        if (si.formats.empty() || si.presentModes.empty()) {
            continue;
        }

        // Записать хендл физического устройства, которое прошло все проверки
        m_device->physicalDevice = physicalDevice;
    }

    // Если не нашлось видео-карт которые удовлетворяют всем требованиям - ошибка
    if (m_device->physicalDevice == nullptr) {
        throw std::runtime_error("Vulkan: Error in the 'InitDevice' function! Can't detect suitable device");
    }

    // Массив объектов структуры VkDeviceQueueCreateInfo содержащих информацию для инициализации очередей
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

    // Массив инедксов семейств (2 индекса - графич. семейство и семейство представления.
    // Индексы могут совпадать, семейство у обеих очередей может быть одно и то же)
    uint32_t queueFamilies[2] = { static_cast<uint32_t>(m_device->queueFamilies.graphics), static_cast<uint32_t>(m_device->queueFamilies.present) };
    const float defaultQueuePriority(0.0f);

    // Если графич. семейство и семейство представления - одно и то же (тот же индекс),
    // нет смысла создавать две очереди одного и того же семейства, можно обойтись одной
    for (int i = 0; i < (uniqueQueueFamilies ? 2 : 1); i++) {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamilies[i];
        queueCreateInfo.queueCount = 1;                                      // Выделяем одну очередь для каждого семейства
        queueCreateInfo.pQueuePriorities = &defaultQueuePriority;            // Массив приоритетов очередей в плане выделения ресурсов (одинаковые пр-теты, не используем)
        queueCreateInfos.push_back(queueCreateInfo);                         // Помещаем структуру в массив
    }


    // Информация о создаваемом логическом устройстве
    // Указываем структуры для создания очередей (queueCreateInfos) и используемые устройством расширения (deviceExtensionsRequired_)
    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
    deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());

    // Проверка запрашиваемых расширений, указать если есть (если не доступны - ошибка)
    if (!deviceExtensionsRequired.empty()) {
        if (!kge::vkutility::CheckDeviceExtensionSupported(m_device->physicalDevice, deviceExtensionsRequired)) {
            throw std::runtime_error("Vulkan: Not all required device extensions supported. Can't initialize renderer");
        }

        deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensionsRequired.size());
        deviceCreateInfo.ppEnabledExtensionNames = deviceExtensionsRequired.data();

        std::cout << "Vulkan: Device extensions: " << deviceCreateInfo.enabledExtensionCount << " " <<  *deviceCreateInfo.ppEnabledExtensionNames << std::endl;
    }

    // Проверка запрашиваемых слоев валидации, указать если есть (если не доступны - ошибка)
    if (!validationLayersRequired.empty()) {
        if (!kge::vkutility::CheckValidationLayersSupported(validationLayersRequired)) {
            throw std::runtime_error("Vulkan: Not all required validation layers supported. Can't initialize renderer");
        }

        deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayersRequired.size());
        deviceCreateInfo.ppEnabledLayerNames = validationLayersRequired.data();
    }

    // Особенности устройства (пока-что пустая структура)
    VkPhysicalDeviceFeatures deviceFeatures = {};
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
    // Создание логического устройства
    if (vkCreateDevice(device->physicalDevice, &deviceCreateInfo, nullptr, &device->logicalDevice) != VK_SUCCESS) {
        std::cout << "cant create device" << std::endl;
        throw std::runtime_error("Vulkan: Failed to create logical device. Can't initialize renderer");
    }

    // Получить хендлы очередей устройства (графической очереди и очереди представления)
    // Если использовано одно семейство, то индексы первых (нулевых) очередей для Graphics и Present будут одинаковы
    vkGetDeviceQueue(m_device->logicalDevice, m_device->queueFamilies.graphics, 0, &(m_device->queues.graphics));
    vkGetDeviceQueue(m_device->logicalDevice, m_device->queueFamilies.present, 0, &(m_device->queues.present));

    // Если в итоге устройство не готово - ошибка
    if (!m_device->IsReady()) {
        throw std::runtime_error("Vulkan: Failed to initialize device and queues. Can't initialize renderer");
    }

    // Сообщение об успешной инициализации устройства
    std::string deviceName = std::string(m_device->GetProperties().deviceName);
    std::string message = "Vulkan: Device successfully initialized (" + deviceName + ")";
    kge::tools::LogMessage(message);
}

KGEVkDevice::~KGEVkDevice()
{
    m_device->Deinit();
    std::cout << "Vulkan: Device successfully destroyed" << std::endl;
}
