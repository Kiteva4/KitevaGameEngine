#include "graphic/VulkanCoreModules/KGEVkDescriptorSet.h"

/**
* Инициализация набор дескрипторов
* @param const vktoolkit::Device &device - устройство
* @param VkDescriptorPool descriptorPool - хендл дескрипторного пула из которого будет выделен набор
* @param VkDescriptorSetLayout descriptorSetLayout - хендл размещения дескрипторно набора
* @param const vktoolkit::UniformBuffer &uniformBufferWorld - буфер содержит необходимую для создания дескриптора информацию
* @param const vktoolkit::UniformBuffer &uniformBufferModels - буфер содержит необходимую для создания дескриптора информацию
*/
VkDescriptorSet KGEVkDescriptorSet::descriptorSet() const
{
    return m_descriptorSet;
}

KGEVkDescriptorSet::KGEVkDescriptorSet(const kge::vkstructs::Device* device,
                                       const VkDescriptorPool* descriptorPool,
                                       const VkDescriptorSetLayout* descriptorSetLayout,
                                       const kge::vkstructs::UniformBuffer* uniformBufferWorld,
                                       const kge::vkstructs::UniformBuffer* uniformBufferModels):
    m_device{device},
    m_descriptorPool{descriptorPool},
    m_descriptorSetLayout{descriptorSetLayout}
{
    // Получить новый набор дескрипторов из дескриптороного пула
    VkDescriptorSetAllocateInfo descriptorSetAllocInfo = {};
    descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocInfo.descriptorPool = *m_descriptorPool;
    descriptorSetAllocInfo.descriptorSetCount = 1;
    descriptorSetAllocInfo.pSetLayouts = m_descriptorSetLayout;

    if (vkAllocateDescriptorSets(m_device->logicalDevice, &descriptorSetAllocInfo, &m_descriptorSet) != VK_SUCCESS) {
        throw std::runtime_error("Vulkan: Error in vkAllocateDescriptorSets. Can't allocate descriptor set");
    }

    // Конфигурация добавляемых в набор дескрипторов
    std::vector<VkWriteDescriptorSet> writes =
    {
        {
            VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,      // Тип структуры
            nullptr,                                     // pNext
            m_descriptorSet,                         // Целевой набор дескрипторов
            0,                                           // Точка привязки (у шейдера)
            0,                                           // Элемент массив (массив не используется)
            1,                                           // Кол-во дескрипторов
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,           // Тип дескриптора
            nullptr,
            &uniformBufferWorld->descriptorBufferInfo,  // Информация о параметрах буфера
            nullptr
        },
        {
            VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,      // Тип структуры
            nullptr,                                     // pNext
            m_descriptorSet,                         // Целевой набор дескрипторов
            1,                                           // Точка привязки (у шейдера)
            0,                                           // Элемент массив (массив не используется)
            1,                                           // Кол-во дескрипторов
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,   // Тип дескриптора
            nullptr,
            &uniformBufferModels->descriptorBufferInfo, // Информация о параметрах буфера
            nullptr,
        },
    };

    // Обновить наборы дескрипторов
    vkUpdateDescriptorSets(m_device->logicalDevice, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);

    kge::tools::LogMessage("Vulkan: Descriptor set successfully initialized");
}

/**
* Деинициализация набор дескрипторов
* @param const vktoolkit::Device &device - устройство
* @param VkDescriptorPool descriptorPool - хендл дескрипторного пула из которого будет выделен набор
* @param VkDescriptorSet * descriptorSet - указатель на хендл набора дескрипторов
*/
KGEVkDescriptorSet::~KGEVkDescriptorSet()
{
    if (m_device->logicalDevice    != nullptr
            && m_descriptorPool   != nullptr
            && m_descriptorSet    != nullptr
            && m_descriptorSet   != nullptr)
    {
        if (vkFreeDescriptorSets(m_device->logicalDevice, *m_descriptorPool, 1, &m_descriptorSet) != VK_SUCCESS) {
            throw std::runtime_error("Vulkan: Error while destroying descriptor set");
        }
        m_descriptorSet = nullptr;

        kge::tools::LogMessage("Vulkan: Descriptor set successfully deinitialized");
    }
}
