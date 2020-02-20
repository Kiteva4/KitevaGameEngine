#include "graphic/VulkanCoreModules/KGEVkUniformBufferModels.h"

/**
* Создание буфера для моделей (динамический uniform-bufer)
* @param const kge::vkstructs::Device &device - устройство
* @param unsigned int maxObjects - максимальное кол-во отдельных объектов на сцене
* @return kge::vkstructs::UniformBuffer - буфер, структура с хендлами буфера, его памяти, а так же доп. свойствами
*
* @note - в отличии от мирового uniform-буфера, uniform-буфер моделей содержит отдельные матрицы для каждой модели (по сути массив)
* и выделение памяти под передаваемый в такой буфер объект должно использовать выравнивание. У устройства есть определенные лимиты
* на выравнивание памяти, поэтому размер такого буфера вычисляется с учетом допустимого шага выравивания и кол-ва объектов которые
* могут быть на сцене.
*/
//kge::vkstructs::UniformBuffer& KGEVkUniformBufferModels::uniformBufferModels()
//{
//    return m_uniformBufferModels;
//}

KGEVkUniformBufferModels::KGEVkUniformBufferModels(const kge::vkstructs::Device* device,
                                                   unsigned int maxObjects):
    m_device{device}
{
    // Вычислить размер буфера учитывая доступное вырванивание памяти (для типа glm::mat4 размером в 64 байта)
    VkDeviceSize bufferSize = m_device->GetDynamicAlignment<glm::mat4>() * maxObjects;

    kge::vkstructs::Buffer buffer = kge::vkutility::CreateBuffer(
                *m_device,
                bufferSize,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    // Настройка результирубщего буфера (uniform-буфер)
    m_uniformBufferModels.vkBuffer = buffer.vkBuffer;
    m_uniformBufferModels.vkDeviceMemory = buffer.vkDeviceMemory;
    m_uniformBufferModels.size = buffer.size;

    // Настройка информации для дескриптора
    m_uniformBufferModels.configDescriptorInfo(64);

    // Разметить буфер (сделать его доступным для копирования информации)
    m_uniformBufferModels.map(m_device->logicalDevice, 64, 0);

    kge::tools::LogMessage("Vulkan: Uniform buffer for models successfully allocated");
}

/**
* Деинициализация (очистка) командного буфера
* @param const kge::vkstructs::Device &device - устройство
* @param kge::vkstructs::UniformBuffer * uniformBuffer - указатель на структуру буфера
*/
KGEVkUniformBufferModels::~KGEVkUniformBufferModels()
{
    if (&m_uniformBufferModels != nullptr) {

        m_uniformBufferModels.unmap(m_device->logicalDevice);

        if (m_uniformBufferModels.vkBuffer != nullptr) {
            vkDestroyBuffer(m_device->logicalDevice, m_uniformBufferModels.vkBuffer, nullptr);
            m_uniformBufferModels.vkBuffer = nullptr;
        }

        if (m_uniformBufferModels.vkDeviceMemory != nullptr) {
            vkFreeMemory(m_device->logicalDevice, m_uniformBufferModels.vkDeviceMemory, nullptr);
            m_uniformBufferModels.vkDeviceMemory = nullptr;
        }

        m_uniformBufferModels.descriptorBufferInfo = {};
        m_uniformBufferModels = {};

        kge::tools::LogMessage("Vulkan: Uniform buffer successfully deinitialized");
    }
}
