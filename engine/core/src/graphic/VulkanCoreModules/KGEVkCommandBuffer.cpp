#include "graphic/VulkanCoreModules/KGEVkCommandBuffer.h"

/**
* Аллокация командных буферов
* @param const kge::vkstructs::Device &device - устройство
* @param VkCommandPool commandPool - хендл командного пула из которого будет осуществляться аллокация
* @param unsigned int count - кол-во аллоцируемых буферов
* @return std::vector<VkCommandBuffer> массив хендлов командных буферов
*/
std::vector<VkCommandBuffer> KGEVkCommandBuffer::commandBuffersDraw() const
{
    return m_commandBuffersDraw;
}

KGEVkCommandBuffer::KGEVkCommandBuffer(const kge::vkstructs::Device *device,
                                       const VkCommandPool *commandPool,
                                       unsigned int count):
    m_device{device},
    m_commandPool{commandPool}
{
    m_commandBuffersDraw.resize(count);
    // Конфигурация аллокации буферов
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = *commandPool;                                               // Указание командного пула
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;                                  // Передается в очередь непосредственно
    allocInfo.commandBufferCount = static_cast<unsigned int>(m_commandBuffersDraw.size()); // Кол-во командных буферов

    // Аллоцировать буферы команд
    if (vkAllocateCommandBuffers(device->logicalDevice, &allocInfo, m_commandBuffersDraw.data()) != VK_SUCCESS) {
        throw std::runtime_error("Vulkan: Error in vkAllocateCommandBuffers function. Failed to allocate command buffers");
    }

    kge::tools::LogMessage("Vulkan: Command buffers successfully allocated");
}

/**
* Деинициализация (очистка) командных буферов
* @param const kge::vkstructs::Device &device - устройство
* @param VkCommandPool commandPool - хендл командного пула из которого были аллоцированы буферы
* @param std::vector<VkCommandBuffer> * buffers - указатель на массив с хендлами буферов (он будет обнулен после очистки)
*/
KGEVkCommandBuffer::~KGEVkCommandBuffer()
{
    // Если массив индентификаторов буферов команд рисования не пуст
    if (m_device->logicalDevice != nullptr && m_commandBuffersDraw.data() != nullptr && !m_commandBuffersDraw.empty()) {
        // Очистисть память
        vkFreeCommandBuffers(m_device->logicalDevice, *m_commandPool, static_cast<unsigned int>(m_commandBuffersDraw.size()), m_commandBuffersDraw.data());
        // Очистить массив
        m_commandBuffersDraw.clear();

        kge::tools::LogMessage("Vulkan: Command buffers successfully freed");
    }
}
