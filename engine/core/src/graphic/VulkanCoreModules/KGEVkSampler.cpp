#include "graphic/VulkanCoreModules/KGEVkSampler.h"

/**
* Инициализация текстурного семплера
* @param const kge::vkstructs::Device &device - устройство
* @note - описывает как данные текстуры подаются в шейдер и как интерпретируются координаты
*/
VkSampler KGEVkSampler::sampler() const
{
    return m_sampler;
}

KGEVkSampler::KGEVkSampler(const kge::vkstructs::Device* device):
    m_device{device}
{
    // Настройка семплера
    VkSamplerCreateInfo samplerInfo = {};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;                      // Тип интерполяции когда тексели больше фрагментов
    samplerInfo.minFilter = VK_FILTER_LINEAR;                      // Тип интерполяции когда тексели меньше фрагментов
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;     // Повторять при выходе за пределы
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_FALSE;                       // Включть анизотропную фильтрацию
    samplerInfo.maxAnisotropy = 4;                                 // уровень фильтрации
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;    // Цвет грани
    samplerInfo.unnormalizedCoordinates = VK_FALSE;                // Использовать нормальзованные координаты (не пиксельные)
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

    // Создание семплера
    if (vkCreateSampler(m_device->logicalDevice, &samplerInfo, nullptr, &m_sampler) != VK_SUCCESS) {
        throw std::runtime_error("Vulkan: Error while creating texture sampler");
    }

    kge::tools::LogMessage("Vulkan: Texture sampler successfully initialized");
}

/**
* Деинициализация текстурного семплера
* @param const kge::vkstructs::Device &device - устройство
* @param VkSampler * sampler - деинициализация текстурного семплера
*/
KGEVkSampler::~KGEVkSampler()
{
    if (m_sampler != nullptr && &m_sampler != nullptr) {
        vkDestroySampler(m_device->logicalDevice, m_sampler, nullptr);
        m_sampler = nullptr;
    }
    kge::tools::LogMessage("Vulkan: Texture sampler successfully deinitialized");
}
