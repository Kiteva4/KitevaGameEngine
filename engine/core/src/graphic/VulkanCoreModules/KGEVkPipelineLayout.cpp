#include "graphic/VulkanCoreModules/KGEVkPipelineLayout.h"

/**
* Инициализация размещения графического конвейера
* @param const vktoolkit::Device &device - устройство
* @param std::vector<VkDescriptorSetLayout> descriptorSetLayouts - хендлы размещениий дискрипторного набора (дает конвейеру инфу о дескрипторах)
* @return VkPipelineLayout - хендл размещения конвейера
*/
VkPipelineLayout KGEVkPipelineLayout::pipelineLayout() const
{
    return m_pipelineLayout;
}

KGEVkPipelineLayout::KGEVkPipelineLayout(const kge::vkstructs::Device* device,
                                         std::vector<VkDescriptorSetLayout> descriptorSetLayouts):
    m_device{device}
{
    VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = {};
    pPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pPipelineLayoutCreateInfo.pNext = nullptr;
    pPipelineLayoutCreateInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
    pPipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts.data();

    if (vkCreatePipelineLayout(m_device->logicalDevice, &pPipelineLayoutCreateInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("Vulkan: Error while creating pipeline layout");
    }

    kge::tools::LogMessage("Vulkan: Pipeline layout successfully initialized");
}

/**
* Деинициализация размещения графического конвейера
* @param const vktoolkit::Device &device - устройство
* @param VkPipelineLayout * pipelineLayout - указатель на хендл размещения
*/
KGEVkPipelineLayout::~KGEVkPipelineLayout()
{
    if (m_device->logicalDevice != nullptr && &m_pipelineLayout != nullptr && m_pipelineLayout != nullptr) {
        vkDestroyPipelineLayout(m_device->logicalDevice, m_pipelineLayout, nullptr);
        m_pipelineLayout = nullptr;

        kge::tools::LogMessage("Vulkan: Pipeline layout successfully deinitialized");
    }
}
