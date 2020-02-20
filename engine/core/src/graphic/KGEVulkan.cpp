#include "graphic/KGEVulkan.h"
#include <filesystem>
#include <cstring>
#include <ctime>
#include <pwd.h>
#include <unistd.h>
namespace fs = std::filesystem;

/**
* Проверка поддержки расширений устройства
* @param deviceExtensionsNames - масив c-строчек содержащих имена расширений
* @return bool - состояние наличия поддержки
* @note - аналагично, как и с расширениями instance'а, некоторый функционал vulkan'а есть только в расширениях
*/
bool kge::vkutility::CheckInstanceExtensionsSupported(std::vector<const char*> instanceExtensionsNames)
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
                //                std::cout << "Vulkan: Extension: "<<  requiredExtName << " founded "  << std::endl;
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

/**
* Проверка поддержки слоев валидации
* @param validationLayersNames - масив c-строчек содержащих имена слоев валидации
* @return bool - состояние наличия поддержки
* @note - слои предоставляют возможность отлаживать программу и проверять данные, если это надо - нужно убедится что они поддерживаются
*/
bool kge::vkutility::CheckValidationLayersSupported(std::vector<const char*> instanceVLayersNames)
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
                //                std::cout << "Vulkan: Layer: "<<  requiredVLayerName << " founded "  << std::endl;
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

#include <sstream>
//Функция, которая будет вызываться слоем валидации при обнаружении ошибок
VKAPI_ATTR VkBool32 VKAPI_CALL kge::vkutility::DebugVulkanCallback(
        VkDebugReportFlagsEXT msgFlags,
        VkDebugReportObjectTypeEXT objType,
        uint64_t srcObject,
        size_t location,
        int32_t msgCode,
        const char* pLayerPrefix,
        const char* pMsg,
        void* pUserData)
{
    std::cout << ">REPORT: ";
    std::ostringstream message;

    if (msgFlags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
        message << "ERROR: ";
    } else if (msgFlags & VK_DEBUG_REPORT_WARNING_BIT_EXT) {
        message << "WARNING: ";
    } else if (msgFlags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) {
        message << "PERFORMANCE WARNING: ";
    } else if (msgFlags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT) {
        message << "INFO: ";
    } else if (msgFlags & VK_DEBUG_REPORT_DEBUG_BIT_EXT) {
        message << "DEBUG: ";
    }
    message << "[" << pLayerPrefix << "] Code " << msgCode << " : " << pMsg;

#ifdef _WIN32
    MessageBox(NULL, message.str().c_str(), "Alert", MB_OK);
#else
    std::cout << message.str() << std::endl;
#endif

    /*
         * false indicates that layer should not bail-out of an
         * API call that had validation failures. This may mean that the
         * app dies inside the driver due to invalid parameter(s).
         * That's what would happen without validation layers, so we'll
         * keep that behavior here.
         */
    return false;
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

/**
* Метод получения информации о семействах очередей (ID'ы нужных семейств очередей и т.п.)
* @param physicalDevice - хендл физического устройства информацю о семействах очередей которого нужно получить
* @param surface - хендл поверхности для которой осуществляется проверка поддержки тех или иных семейств
* @param uniqueStrict - нужно ли заправшивать уникальные семейства для команд рисования и представления (семейство может быть одно)
* @return QueueFamilyInfo - объект с ID'ами семейств очередей команд рисования (graphics) и представления (present)
*/
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


/**
* Метод получения информации об особенностях поверхности
* @param VkPhysicalDevice physicalDevice - хендл физического устройства которое предоставляет поддержку тех или иных форматов конкретной поверхностью
* @param VkSurfaceKHR surface - хендл поверхности о которой нужно получить информацию
* @return SurfaceInfo - объект у которого есть массивы форматов (formats), режимов (presentModes) и набор возможностей (capabilities)
*/
kge::vkstructs::SurfaceInfo kge::vkutility::GetSurfaceInfo(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
    kge::vkstructs::SurfaceInfo surfaceInfo = {};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceInfo.capabilities);

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

/**
* Получить индекс типа памяти, которая поддерживает конкретные особенности
* @param physicalDevice - хендл физического устройства информацю о возможных типах памяти которого нужно получить
* @param unsigned int typeFlags - побитовая маска с флагами типов запрашиваемой памяти
* @param VkMemoryPropertyFlags properties - параметры запрашиваемой памяти
* @return int - возвращает индекс типа памяти, который соответствует всем условиям
* @note - данный метод используется, например, при создании буферов
*/
int kge::vkutility::GetMemoryTypeIndex(VkPhysicalDevice physicalDevice, unsigned int typeFlags, VkMemoryPropertyFlags properties)
{
    // Получить настройки памяти физического устройства
    VkPhysicalDeviceMemoryProperties deviceMemoryProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &deviceMemoryProperties);

    // Пройтись по всем типам и найти подходящий
    // Для опредения нужного индекса типа памяти использются побитовые операции, подробнее о побитовых операциях - https://ravesli.com/urok-45-pobitovye-operatory/
    for (unsigned int i = 0; i < deviceMemoryProperties.memoryTypeCount; i++) {
        if ((typeFlags & (1 << i)) && (deviceMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    // Отрицательное значение в случае отсутствия необходимого индекса
    return -1;
}

/**
* Создание буфера
* @param vkstructs::Device &device - устройство в памяти которого, либо с доступном для которого, будет создаваться буфер
* @param VkDeviceSize size - размер создаваемого буфера
* @param VkBufferUsageFlags usage - как буфер будет использован (например, как вершинный - VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
* @param VkMemoryPropertyFlags properties - свойства памяти буфера (память устройства, память хоста, для "кого" память видима и т.д.)
* @param VkSharingMode sharingMode - настройка доступа к памяти буфера для очередей (VK_SHARING_MODE_EXCLUSIVE - с буфером работает одна очередь)
* @return vkstructs::Buffer - структура содержающая хендл буфера, хендл памяти а так же размер буфера
*/
kge::vkstructs::Buffer kge::vkutility::CreateBuffer(const kge::vkstructs::Device &device,
                                                    VkDeviceSize size,
                                                    VkBufferUsageFlags usage,
                                                    VkMemoryPropertyFlags properties,
                                                    VkSharingMode sharingMode)
{
    // Объект буфера что будет отдан функцией
    vkstructs::Buffer resultBuffer;

    // Установить размер
    resultBuffer.size = size;

    // Настройка создания vk-буфера
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = sharingMode;
    bufferInfo.flags = 0;

    // Попытка создания буфера
    if (vkCreateBuffer(device.logicalDevice, &bufferInfo, nullptr, &(resultBuffer.vkBuffer)) != VK_SUCCESS) {
        throw std::runtime_error("Vulkan: Error while creating buffer. Can't create!");
    }

    // Получить требования буфера к памяти
    VkMemoryRequirements memRequirements = {};
    vkGetBufferMemoryRequirements(device.logicalDevice, resultBuffer.vkBuffer, &memRequirements);

    // Получить индекс типа памяти соответствующего требованиям буфера
    int memoryTypeIndex = vkutility::GetMemoryTypeIndex(device.physicalDevice, memRequirements.memoryTypeBits, properties);
    if (memoryTypeIndex < 0) {
        throw std::runtime_error("Vulkan: Error while creating buffer. Can't find suitable memory type!");
    }

    // Настрйока выделения памяти (учитывая требования и полученный индекс)
    VkMemoryAllocateInfo memoryAllocateInfo = {};
    memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocateInfo.allocationSize = memRequirements.size;
    memoryAllocateInfo.memoryTypeIndex = static_cast<unsigned int>(memoryTypeIndex);

    // Выделение памяти для буфера
    if (vkAllocateMemory(device.logicalDevice, &memoryAllocateInfo, nullptr, &(resultBuffer.vkDeviceMemory)) != VK_SUCCESS) {
        throw std::runtime_error("Vulkan: Error while allocating buffer memory!");
    }

    // Привязать память к буферу
    vkBindBufferMemory(device.logicalDevice, resultBuffer.vkBuffer, resultBuffer.vkDeviceMemory, 0);

    // Вернуть буфер
    return resultBuffer;
}

/**
* Создание простого однослойного изображения
* @param kge::vkstructs::Device &device - устройство в памяти которого, либо с доступном для которого, будет создаваться изображение
* @param VkImageType imageType - тип изображения (1D, 2D. 3D текстура)
* @param VkFormat format - формат изображения
* @param VkExtent3D extent - расширение (разрешение) изображения
* @param VkImageUsageFlags usage - использование изображения (в качестве чего, назначение)
* @param VkImageAspectFlags subresourceRangeAspect - использование области подресурса (???)
* @param VkSharingMode sharingMode - настройка доступа к памяти изображения для очередей (VK_SHARING_MODE_EXCLUSIVE - с буфером работает одна очередь)
*/
kge::vkstructs::Image kge::vkutility::CreateImageSingle(const kge::vkstructs::Device &device,
                                                        VkImageType imageType,
                                                        VkFormat format,
                                                        VkExtent3D extent,
                                                        VkImageUsageFlags usage,
                                                        VkImageAspectFlags subresourceRangeAspect,
                                                        VkImageLayout initialLayout,
                                                        VkMemoryPropertyFlags memoryProperties,
                                                        VkImageTiling tiling,
                                                        VkSharingMode sharingMode)
{
    // Результирующий объект изображения
    vkstructs::Image resultImage;
    resultImage.extent = extent;
    resultImage.format = format;

    // Конфигурация изображения
    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.pNext = nullptr;
    imageInfo.imageType = imageType;
    imageInfo.format = format;
    imageInfo.extent = extent;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling = tiling;
    imageInfo.sharingMode = sharingMode;
    imageInfo.usage = usage;
    imageInfo.initialLayout = initialLayout;

    // Создание изображения
    if (vkCreateImage(device.logicalDevice, &imageInfo, nullptr, &(resultImage.vkImage)) != VK_SUCCESS) {
        throw std::runtime_error("Vulkan: Error while creating image");
    }

    // Получить требования к памяти с учетом параметров изображения
    VkMemoryRequirements memReqs = {};
    vkGetImageMemoryRequirements(device.logicalDevice, resultImage.vkImage, &memReqs);

    // Конфигурация аллокации памяти (память на устройстве)
    VkMemoryAllocateInfo memoryAllocInfo = {};
    memoryAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocInfo.allocationSize = memReqs.size;
    memoryAllocInfo.memoryTypeIndex = static_cast<uint32_t>(vkutility::GetMemoryTypeIndex(device.physicalDevice, memReqs.memoryTypeBits, memoryProperties));
    memoryAllocInfo.pNext = nullptr;

    // Аллоцировать
    if (vkAllocateMemory(device.logicalDevice, &memoryAllocInfo, nullptr, &(resultImage.vkDeviceMemory)) != VK_SUCCESS) {
        throw std::runtime_error("Vulkan: Error while allocating memory for image");
    }

    // Привязать
    if (vkBindImageMemory(device.logicalDevice, resultImage.vkImage, resultImage.vkDeviceMemory, 0) != VK_SUCCESS) {
        throw std::runtime_error("Vulkan: Error while binding memory to image");
    }

    // Подходящий тип view-объекта (в зависимости от типа изображения)
    VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_1D;
    if (imageInfo.imageType == VK_IMAGE_TYPE_2D) {
        viewType = VK_IMAGE_VIEW_TYPE_2D;
    }
    else if (imageInfo.imageType == VK_IMAGE_TYPE_3D) {
        viewType = VK_IMAGE_VIEW_TYPE_3D;
    }

    // Конфигурация view-объекта
    VkImageViewCreateInfo imageViewInfo = {};
    imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewInfo.viewType = viewType;
    imageViewInfo.format = format;
    imageViewInfo.subresourceRange = {};
    imageViewInfo.subresourceRange.aspectMask = subresourceRangeAspect;
    imageViewInfo.subresourceRange.baseMipLevel = 0;
    imageViewInfo.subresourceRange.levelCount = 1;
    imageViewInfo.subresourceRange.baseArrayLayer = 0;
    imageViewInfo.subresourceRange.layerCount = 1;
    imageViewInfo.image = resultImage.vkImage;

    // Создание view-обхекта
    if (vkCreateImageView(device.logicalDevice, &imageViewInfo, nullptr, &(resultImage.vkImageView)) != VK_SUCCESS) {
        throw std::runtime_error("Vulkan: Error while creating image view");
    }

    return resultImage;
}

/**
* Получить описание привязок вершинных данных к конвейеру
* @param unsigned int bindingIndex - индекс привязки буфера вершин к конвейеру
* @return std::vector<VkVertexInputBindingDescription> - массив описаний привязок
*
* @note - при привязывании буфера вершин к конвейеру, указывается индекс привязки. Нужно получить информацию
* для конкретной привязки, о том как конвейер будет интерпретировать привязываемый буфер, какого размера
* один элемент (вершина) в буфере, как переходить к следующему и тд. Вся информация в этой структуре
*/
std::vector<VkVertexInputBindingDescription> kge::vkutility::GetVertexInputBindingDescriptions(unsigned int bindingIndex)
{
    return
    {
        {
            bindingIndex,                   // Индекс привязки вершинных буферов
                    sizeof(kge::vkstructs::Vertex),      // Размерность шага
                    VK_VERTEX_INPUT_RATE_VERTEX     // Правила перехода к следующим
        }
    };
}

/**
* Отправить команду на исполнение и очистить буфер
* @param const kge::vkstructs::Device &device - устройство
* @param VkCommandPool commandPool - командный пул, из которого был выделен буфер
* @param VkCommandBuffer commandBuffer - командный буфер
*/
void kge::vkutility::FlushSingleTimeCommandBuffer(const kge::vkstructs::Device &device,
                                                  VkCommandPool commandPool,
                                                  VkCommandBuffer commandBuffer,
                                                  VkQueue queue)
{
    // Завершаем наполнение командного буффера
    vkEndCommandBuffer(commandBuffer);

    // Отправка команд в очередь
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    vkQueueSubmit(queue, 1, &submitInfo, nullptr);

    // Ожидаем выполнения команды
    vkQueueWaitIdle(queue);

    // Очищаем буфер
    vkFreeCommandBuffers(device.logicalDevice, commandPool, 1, &commandBuffer);
}

/**
* Создать буфер одиночных команд
* @param const kge::vkstructs::Device &device - устройство
* @param VkCommandPool commandPool - командный пул, из которого будет выделен буфер
* @return VkCommandBuffer - хендл нового буфера
*/
VkCommandBuffer kge::vkutility::CreateSingleTimeCommandBuffer(const kge::vkstructs::Device &device,
                                                              VkCommandPool commandPool)
{
    // Хендл нового буфера
    VkCommandBuffer commandBuffer;

    // Аллоцировать буфер
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;
    vkAllocateCommandBuffers(device.logicalDevice, &allocInfo, &commandBuffer);

    // Начать командный буфер (готов к записи команд)
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // Используем один раз и ожидаем результата
    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    // Отдать хендл
    return commandBuffer;
}

/**
* Копировать изображение
* @param VkCommandBuffer cmdBuffer - хендл командного буфера, в который будет записана команда смены размещения
* @param VkImage srcImage - исходное изображение, память которого нужно скопировать
* @param VkImage dstImage - целевое изображение, в которое нужно перенести память
* @param uint32_t width - ширина
* @param uint32_t height - высота
*/
void kge::vkutility::CmdImageCopy(VkCommandBuffer cmdBuffer,
                                  VkImage srcImage,
                                  VkImage dstImage,
                                  uint32_t width,
                                  uint32_t height)
{
    // Описание слоев подресурса (мип-уровни не используются)
    VkImageSubresourceLayers subresourceLayers = {};
    subresourceLayers.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresourceLayers.baseArrayLayer = 0;
    subresourceLayers.mipLevel = 0;
    subresourceLayers.layerCount = 1;

    // Конфигурация копирования
    VkImageCopy region = {};
    region.srcSubresource = subresourceLayers;
    region.dstSubresource = subresourceLayers;
    region.srcOffset = { 0, 0, 0 };
    region.dstOffset = { 0, 0, 0 };
    region.extent.width = width;
    region.extent.height = height;
    region.extent.depth = 1;

    // Записать команду копирования в буфер
    vkCmdCopyImage(
                cmdBuffer,
                srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1, &region
                );
}

/**
* Изменить размещение изображения
* @param VkCommandBuffer cmdBuffer - хендл командного буфера, в который будет записана команда смены размещения
* @param VkImage image - хендл изображения, размещение которого нужно сменить
* @param VkImageLayout oldImageLayout - старое размещение
* @param VkImageLayout newImageLayout - новое размещение
* @param VkImageSubresourceRange subresourceRange - описывает какие регионы изображения подвергнутся переходу размещения
*/
void kge::vkutility::CmdImageLayoutTransition(VkCommandBuffer cmdBuffer,
                                              VkImage image,
                                              VkImageLayout oldImageLayout,
                                              VkImageLayout newImageLayout,
                                              VkImageSubresourceRange subresourceRange)
{
    // Создать барьер памяти изображения
    VkImageMemoryBarrier imageMemoryBarrier = {};
    imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imageMemoryBarrier.pNext = nullptr;
//    imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
//    imageMemoryBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    imageMemoryBarrier.oldLayout = oldImageLayout;
    imageMemoryBarrier.newLayout = newImageLayout;
    imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
    imageMemoryBarrier.subresourceRange.levelCount = 1;
    imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
    imageMemoryBarrier.subresourceRange.layerCount = 1;
    imageMemoryBarrier.image = image;
    imageMemoryBarrier.subresourceRange = subresourceRange;

    // В зависимоти от старого (исходного) размещения меняется исходная маска доступа
    switch (oldImageLayout)
    {
    case VK_IMAGE_LAYOUT_UNDEFINED:
        imageMemoryBarrier.srcAccessMask = 0;
        break;
    case VK_IMAGE_LAYOUT_PREINITIALIZED:
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
        break;
    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        break;
    default:
        std::cout << "WARNING!_133: default switch" << std::endl;
        break;

    }

    // В зависимости от нового (целевого) размещения меняется целевая маска доступа
    switch (newImageLayout)
    {
    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        break;
    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        break;
    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        break;
    default:
        std::cout << "WARNING!_134: default switch" << std::endl;
        break;
    }

    // Разметить барьер на верише конвейера (в самом начале)
    VkPipelineStageFlags srcStageFlags = VK_PIPELINE_STAGE_TRANSFER_BIT;
    VkPipelineStageFlags destStageFlags = VK_PIPELINE_STAGE_TRANSFER_BIT;

    // Отправить команду барьера в командный буфер
    vkCmdPipelineBarrier(cmdBuffer,
                         srcStageFlags,
                         destStageFlags,
                         0,
                         0,
                         nullptr,
                         0,
                         nullptr,
                         1,
                         &imageMemoryBarrier);
}

/**
* Загрузка шейдерного модуля из файла
* @param std::string filename - наименование файла шейдера, поиск по умолчанию в папке shaders
* @param VkDevice logicalDevice - хендл логичекского устройства, нужен для созданния шейдерного модуля
* @return VkShaderModule - хендл шейдерного модуля созданного из загруженного файла
*/
VkShaderModule kge::vkutility::LoadSPIRVShader(std::filesystem::path shaderFilePath,
                                               VkDevice logicalDevice)
{
    // Размер
    size_t shaderSize;

    // Содержимое файла (код шейдера)
    char* shaderCode = nullptr;

    // Загрузить код шейдера
    bool loaded = tools::LoadBytesFromFile(shaderFilePath, &shaderCode, &shaderSize);

    // Если не удалось загрузить или файл пуст
    if (!loaded || shaderSize == 0){
        std::string msg = "Vulkan: Error while loading shader code from file " + std::to_string(*shaderFilePath.c_str());
        throw std::runtime_error(msg);
    }

    // Конфигурация шейдерного модуля
    VkShaderModuleCreateInfo moduleCreateInfo{};
    moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    moduleCreateInfo.codeSize = shaderSize;
    moduleCreateInfo.pCode = reinterpret_cast<unsigned int*>(shaderCode);

    // Создать шейдерный модуль
    VkShaderModule shaderModule;
    if (vkCreateShaderModule(logicalDevice, &moduleCreateInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        std::string msg = *(&"Vulkan: Error whiler creating shader module from file " +*shaderFilePath.c_str());
        throw std::runtime_error(msg);
    }

    // Удаляем код шейдера (хранить его более не нужно, поскольку он был передан в шейдерный модуль)
    delete[] shaderCode;

    // Вернуть хендл шейдерного модуля
    return shaderModule;
}

/**
* Получить описание аттрибутов привязываемых к конвейеру вершин
* @param unsigned int bindingIndex - индекс привязки буфера вершин к конвейеру
* @return std::vector<VkVertexInputAttributeDescription> - массив описаний атрибутов передаваемых вершин
*
* @note - конвейеру нужно знать как интерпретировать данные о вершинах. Какие у каждой вершины, в привязываемом буфере,
* есть параметры (аттрибуты). В какой последовательности они идут, какого типа каждый аттрибут.
*/
std::vector<VkVertexInputAttributeDescription> kge::vkutility::GetVertexInputAttributeDescriptions(unsigned int bindingIndex)
{
    return
    {
        {
            0,                                      // Индекс аттрибута (location в шейдере)
            bindingIndex,                           // Индекс привязки вершинных буферов
                    VK_FORMAT_R32G32B32_SFLOAT,             // Тип аттрибута (соответствует vec3 у шейдера)
                    offsetof(vkstructs::Vertex, position)   // Cдвиг в структуре
        },
        {
            1,
            bindingIndex,
                    VK_FORMAT_R32G32B32_SFLOAT,
                    offsetof(vkstructs::Vertex, color)
        },
        {
            2,
            bindingIndex,
                    VK_FORMAT_R32G32_SFLOAT,               // Тип аттрибута (соответствует vec2 у шейдера)
                    offsetof(vkstructs::Vertex, texCoord)
        },
        {
            3,
            bindingIndex,
                    VK_FORMAT_R32_UINT,                    // Тип аттрибута (соответствует uint у шейдера)
                    offsetof(vkstructs::Vertex, textureUsed)
        },
    };
}

/**
* Путь к рабочему каталогу
* @return std::string - строка содержащая путь к директории
* @note нужно учитывать что рабочий каталок может зависеть от конфигурации проекта
*/
std::filesystem::path kge::tools::WorkingDir()
{
    struct passwd *pw = getpwuid(getuid());
    const char *homedir = pw->pw_dir;
    return std::filesystem::path(homedir).concat("/GitHub/KitevaGameEngine/");
}

/**
* Путь к каталогу с исполняемым файлом (директория содержащая запущенный .exe)
* @return std::string - строка содержащая путь к директории
*/
std::filesystem::path kge::tools::ExeDir()
{
    return std::filesystem::current_path();
}

/**
* Получить текущее время
* @return std::time_t - числовое значение текущего момента системного времени
*/
std::time_t kge::tools::CurrentTime()
{
    return std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
}

/**
* Получить время в виде отформатированной строки (напр. ГГГГ-ММ-ДД ЧЧ:ММ:CC)
* @param std::time_t * time - числовое значение времени
* @param const char * format - формат в виде c-строки (напр. %Y-%m-%d %H:%M:%S)
* @return std::string - строка с отформатированным временем
* @note подробнее о форматах - http://www.cplusplus.com/reference/ctime/strftime/
*/
std::string kge::tools::TimeToStr(const std::time_t& time, const char * format)
{
    tm timeInfo;
    localtime_r(&time, &timeInfo);
    char strTime[32];
    strftime(strTime, 32, format, &timeInfo);
    return strTime;
}

/**
* Логирование. Пишет в файл и консоль (если она еть) строку со временем и указанным сообщением
* @param std::string message - строка с соощением
* @param bool printTime - выводить ли время
*/
void kge::tools::LogMessage(std::string message, bool printTime)
{
    std::string result = "";

    if (printTime) {
        time_t time = tools::CurrentTime();
        result += tools::TimeToStr(time, "[%Y-%m-%d %H:%M:%S] ");
    }

    result += message + '\n';
    std::cout << result;

    try {
        std::filesystem::path filePath = std::string("/home/vxuser/GitHub/KitevaGameEngine/") + LOG_FILENAME;

        std::fstream fs;
        fs.exceptions(std::ios::failbit | std::ios::badbit);
        fs.open(filePath, std::ios_base::app);

        fs << result;
        fs.close();
    }
    catch (const std::exception &ex) {
        std::cout << ex.what() << std::endl;
    }
}

/**
* То же что и LogMessage, с той разницей что перед сообщением будет добавляться слово "ERROR!"
* @param std::string message - строка с соощением
* @param bool printTime - выводить ли время
*/
void kge::tools::LogError(std::string message, bool printTime)
{
    std::string newMessage = "ERROR! ";
    newMessage += message;
    tools::LogMessage(message, printTime);
}

/**
* Конвертация из обычной string-строки в "широкую" wstring
* @param const std::string& str - исходная string строка
* @param UINT codePage - идентификатор поддерживаемой операционной системой "кодовй страницы"
* @param DWORD dwFlags - тип конвертации (как конвертировать простые символы в составные)
* @return std::wstring - wide (широкая) строка
* @note это обертка winapi метода MultiByteToWideChar - https://msdn.microsoft.com/en-us/library/windows/desktop/dd319072(v=vs.85).aspx
*/
//std::wstring kge::tools::StrToWide(const std::string& str, UINT codePage, DWORD dwFlags)
//{
//    std::wstring result;
//    wchar_t* newString = new wchar_t[str.length() + 1];
//    MultiByteToWideChar(codePage, dwFlags, str.c_str(), static_cast<int>(str.length()) + 1, newString, static_cast<int>(str.length()) + 1);
//    result.append(newString);
//    delete[] newString;
//    return result;
//}

/**
* Конвертация из "широкой" wstring-строки в обычную string
* @param const std::wstring& wstr - исходная wstring (широкая) строка
* @param UINT codePage - идентификатор поддерживаемой операционной системой "кодовй страницы"
* @param DWORD dwFlags - тип конвертации (как конвертировать составные символы в простые)
* @return std::wstring - string строка
* @note это обертка winapi метода WideCharToMultiByte - https://msdn.microsoft.com/en-us/library/windows/desktop/dd374130(v=vs.85).aspx
*/
//std::string kge::tools::WideToStr(const std::wstring& wstr, UINT codePage, DWORD dwFlags)
//{
//    std::string result;
//    char* newString = new char[wstr.length() + 1];
//    WideCharToMultiByte(codePage, dwFlags, wstr.c_str(), static_cast<int>(wstr.length()) + 1, newString, static_cast<int>(wstr.length()) + 1, nullptr, FALSE);
//    result.append(newString);
//    delete[] newString;
//    return result;
//}

/**
* Загрузка бинарных данных из файла
* @param const std::string &path - путь к файлу
* @param char** pData - указатель на указатель на данные, память для которых будет аллоцирована
* @param size_t * size - указатель на параметр размера (размер будет получен при загрузке)
* @return bool - состояние загрузки (удалось или нет)
*/
bool kge::tools::LoadBytesFromFile(const std::filesystem::path &path, char** pData, size_t * size)
{
    // Открытие файла в режиме бинарного чтения
    std::ifstream is(path.c_str(), std::ios::binary | std::ios::in | std::ios::ate);

    // Если файл открыт
    if (is.is_open())
    {
        *size = static_cast<size_t>(is.tellg()); // Получить размер
        is.seekg(0, std::ios::beg); // Перейти в начало файла
        *pData = new char[*size];   // Аллокировать необходимое кол-во памяти
        is.read(*pData, *size);     // Прочесть файл и поместить данные в буфер
        is.close();                 // Закрыть файл

        return true;
    }

    return false;
}
