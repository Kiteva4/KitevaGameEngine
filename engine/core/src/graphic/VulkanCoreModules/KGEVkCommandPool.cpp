#include "graphic/VulkanCoreModules/KGEVkCommandPool.h"

/**
* Инциализация командного пула
* @param const kge::vkstructs::Device &device - устройство
* @param unsigned int queueFamilyIndex - индекс семейства очередей команды которого будут передаваться в аллоцированных их пуда буферах
* @return VkCommandPool - хендл командного пула
* @note - из командных пулов осуществляется аллокация буферов команд, следует учитывать что для отедльных очередей нужны отдельные пулы
*/
const VkCommandPool &KGEVkCommandPool::commandPool()
{
    return m_commandPool;
}

KGEVkCommandPool::KGEVkCommandPool(const kge::vkstructs::Device *device,
                                   unsigned int queueFamilyIndex):
    m_device{device}
{
    // Описание пула
    VkCommandPoolCreateInfo commandPoolCreateInfo = {};
    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.queueFamilyIndex = static_cast<unsigned int>(queueFamilyIndex);
    commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    // Создание пула
    if (vkCreateCommandPool(device->logicalDevice, &commandPoolCreateInfo, nullptr, &m_commandPool) != VK_SUCCESS) {
        throw std::runtime_error("Vulkan: Error in vkCreateCommandPool function. Failed to create command pool");
    }

    kge::tools::LogMessage("Vulkan: Command pool successfully initialized");
}

/**
* Деинциализация командного пула
* @param const kge::vkstructs::Device &device - устройство
* @param VkCommandPool * commandPool - указатель на хендл командного пула
*/
KGEVkCommandPool::~KGEVkCommandPool()
{
    if (m_commandPool != nullptr && &m_commandPool != nullptr) {
        vkDestroyCommandPool(m_device->logicalDevice, m_commandPool, nullptr);
        m_commandPool = nullptr;
        kge::tools::LogMessage("Vulkan: Command pool successfully deinitialized");
    }
}
