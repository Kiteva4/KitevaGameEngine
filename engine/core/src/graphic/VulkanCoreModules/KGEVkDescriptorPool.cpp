#include "graphic/VulkanCoreModules/KGEVkDescriptorPool.h"
/**
* Инициализация основного декскрипторного пула
* @param const kge::vkstructs::Device &device - устройство
* @return VkDescriptorPool - хендл дескрипторного пула
* @note - дескрипторный пул позволяет выделять специальные наборы дескрипторов, обеспечивающие доступ к определенным буферам из шейдера
*/
const VkDescriptorPool& KGEVkDescriptorPool::descriptorPool()
{
    return m_descriptorPool;
}

KGEVkDescriptorPool::KGEVkDescriptorPool(const kge::vkstructs::Device* device):
    m_device{device}
{
    // Парамтеры размеров пула
    std::vector<VkDescriptorPoolSize> descriptorPoolSizes =
    {
        // Один дескриптор для глобального uniform-буфера
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER , 1 },
        // Один дескриптор для unform-буферов отдельных объектов (динамический)
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1 }
    };


    // Конфигурация пула
    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolInfo.poolSizeCount = static_cast<uint32_t>(descriptorPoolSizes.size());
    poolInfo.pPoolSizes = descriptorPoolSizes.data();
    poolInfo.maxSets = 1;

    // Создание дескрипторного пула
    if (vkCreateDescriptorPool(m_device->logicalDevice, &poolInfo, nullptr, &m_descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("Vulkan: Error in vkCreateDescriptorPool function. Cant't create descriptor pool");
    }

    kge::tools::LogMessage("Vulkan: Main descriptor pool successfully initialized");
}


/**
* Инициализация декскрипторного пула под текстурные наборы дескрипторов
* @param const kge::vkstructs::Device &device - устройство
* @param uint32_t maxDescriptorSets - максимальное кол-во наборов
* @return VkDescriptorPool - хендл дескрипторного пула
* @note - дескрипторный пул позволяет выделять специальные наборы дескрипторов, обеспечивающие доступ к определенным буферам из шейдера
*/
KGEVkDescriptorPool::KGEVkDescriptorPool(const kge::vkstructs::Device *device,
                                         uint32_t maxDescriptorSets):
    m_device{device}
{
    // Парамтеры размеров пула
    std::vector<VkDescriptorPoolSize> descriptorPoolSizes =
    {
        // Два дескриптор для текстурного семплера
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER , 2 },
    };

    // Конфигурация пула
    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.pNext = nullptr;
    poolInfo.poolSizeCount = static_cast<uint32_t>(descriptorPoolSizes.size());
    poolInfo.pPoolSizes = descriptorPoolSizes.data();
    poolInfo.maxSets = maxDescriptorSets;

    // Создание дескрипторного пула
    if (vkCreateDescriptorPool(m_device->logicalDevice, &poolInfo, nullptr, &m_descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("Vulkan: Error in vkCreateDescriptorPool function. Cant't create descriptor pool");
    }

    kge::tools::LogMessage("Vulkan: Texture descriptor pool successfully initialized");
}

/**
* Деинициализация дескрипторного пула
* @param const kge::vkstructs::Device &device - устройство
* @VkDescriptorPool * descriptorPool - указатель на хендл дескрипторного пула
*/
KGEVkDescriptorPool::~KGEVkDescriptorPool()
{
    if (m_descriptorPool != nullptr && &m_descriptorPool != nullptr) {
        vkDestroyDescriptorPool(m_device->logicalDevice, m_descriptorPool, nullptr);
        m_descriptorPool = nullptr;
        kge::tools::LogMessage("Vulkan: Descriptor pool successfully deinitialized");
    }
}
