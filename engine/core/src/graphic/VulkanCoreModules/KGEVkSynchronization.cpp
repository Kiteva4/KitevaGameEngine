#include "graphic/VulkanCoreModules/KGEVkSynchronization.h"

/**
* Инициализация примитивов синхронизации
* @param const vktoolkit::Device &device - устройство
* @return vktoolkit::Synchronization - структура с набором хендлов семафоров
* @note - семафоры синхронизации позволяют отслеживать состояние рендеринга и в нужный момент показывать изображение
*/
KGEVkSynchronization::KGEVkSynchronization(kge::vkstructs::Synchronization* sync,
                                           const kge::vkstructs::Device* device):
    m_sync{sync},
    m_device{device}
{
    // Информация о создаваемом семафоре (ничего не нужно указывать)
    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    // Создать примитивы синхронизации
    if (vkCreateSemaphore(m_device->logicalDevice, &semaphoreInfo, nullptr, &m_sync->readyToRender) != VK_SUCCESS ||
            vkCreateSemaphore(m_device->logicalDevice, &semaphoreInfo, nullptr, &m_sync->readyToPresent) != VK_SUCCESS) {
        throw std::runtime_error("Vulkan: Error while creating synchronization primitives");
    }

    kge::tools::LogMessage("Vulkan: Synchronization primitives sucessfully initialized");
}

/**
* Деинициализация примитивов синхронизации
* @param const vktoolkit::Device &device - устройство
* @param vktoolkit::Synchronization * sync - указатель на структуру с хендлами семафоров
*/
KGEVkSynchronization::~KGEVkSynchronization()
{
    if (m_sync != nullptr) {
        if (m_sync->readyToRender != nullptr) {
            vkDestroySemaphore(m_device->logicalDevice, m_sync->readyToRender, nullptr);
            m_sync->readyToRender = nullptr;
        }

        if (m_sync->readyToRender != nullptr) {
            vkDestroySemaphore(m_device->logicalDevice, m_sync->readyToRender, nullptr);
            m_sync->readyToRender = nullptr;
        }

        kge::tools::LogMessage("Vulkan: Synchronization primitives sucessfully deinitialized");
    }
}
