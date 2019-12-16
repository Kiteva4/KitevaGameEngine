#include "graphic/VulkanCoreModules/KGEVkUniformBufferWorld.h"

/**
* Создание мирового (глобального) unform-buffer'а
* @param const kge::vkstructs::Device &device - устройство
* @return kge::vkstructs::UniformBuffer - буфер, структура с хендлами буфера, его памяти, а так же доп. свойствами
*
* @note - unform-буфер это буфер доступный для шейдера посредством дескриптороа. В нем содержится информация о матрицах используемых
* для преобразования координат вершин сцены. В буфер помещается UBO объект содержащий необходимые матрицы. При каждом обновлении сцены
* можно отправлять объект с новыми данными (например, если сменилось положение камеры, либо угол ее поворота). Таким образом шейдер будет
* использовать для преобразования координат вершины новые данные.
*/
KGEVkUniformBufferWorld::KGEVkUniformBufferWorld(kge::vkstructs::UniformBuffer* uniformBufferWorld,
                                                 const kge::vkstructs::Device* device):
    m_device{device},
    m_uniformBufferWorld{uniformBufferWorld}
{
    // Создать буфер, выделить память, привязать память к буферу
    kge::vkstructs::Buffer buffer = kge::vkutility::CreateBuffer(
                *m_device,
                sizeof(kge::vkstructs::UboWorld),
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    // Основная конфиуграция результирущего буфера
    m_uniformBufferWorld->vkBuffer = buffer.vkBuffer;
    m_uniformBufferWorld->vkDeviceMemory = buffer.vkDeviceMemory;
    m_uniformBufferWorld->size = buffer.size;

    // Настройка информации для дескриптора
    m_uniformBufferWorld->configDescriptorInfo(buffer.size, 0);

    // Разметить буфер (сделать его доступным для копирования информации)
    m_uniformBufferWorld->map(m_device->logicalDevice, buffer.size, 0);

    kge::tools::LogMessage("Vulkan: Uniform buffer for world scene successfully allocated");
}

/**
* Деинициализация (очистка) командного буфера
* @param const kge::vkstructs::Device &device - устройство
* @param kge::vkstructs::UniformBuffer * uniformBuffer - указатель на структуру буфера
*/
KGEVkUniformBufferWorld::~KGEVkUniformBufferWorld()
{
    if (m_uniformBufferWorld != nullptr) {

        m_uniformBufferWorld->unmap(m_device->logicalDevice);

        if (m_uniformBufferWorld->vkBuffer != nullptr) {
            vkDestroyBuffer(m_device->logicalDevice, m_uniformBufferWorld->vkBuffer, nullptr);
            m_uniformBufferWorld->vkBuffer = nullptr;
        }

        if (m_uniformBufferWorld->vkDeviceMemory != nullptr) {
            vkFreeMemory(m_device->logicalDevice, m_uniformBufferWorld->vkDeviceMemory, nullptr);
            m_uniformBufferWorld->vkDeviceMemory = nullptr;
        }

        m_uniformBufferWorld->descriptorBufferInfo = {};
        *m_uniformBufferWorld = {};

        kge::tools::LogMessage("Vulkan: Uniform buffer successfully deinitialized");
    }
}
