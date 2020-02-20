#include "graphic/VulkanCoreModules/KGEVkDescriptorSetLayout.h"

/**
* Инициализация описания размещения дескрипторного пула (под основной дескрипторный набор)
* @param const kge::vkstructs::Device &device - устройство
* @return VkDescriptorSetLayout - хендл размещения дескрипторного пула
* @note - Размещение - информация о том сколько и каких именно (какого типа) дескрипторов следует ожидать на определенных этапах конвейера
*/
/**
* Инициализация описания размещения дескрипторного пула (под текстурные наборы дескрипторов)
* @param const kge::vkstructs::Device &device - устройство
* @return VkDescriptorSetLayout - хендл размещения дескрипторного пула
* @note - Размещение - информация о том сколько и каких именно (какого типа) дескрипторов следует ожидать на определенных этапах конвейера
*/
KGEVkDescriptorSetLayout::KGEVkDescriptorSetLayout(const kge::vkstructs::Device *device,
                                                   SET_LAYOUT_TYPE setLayoutType):
    m_device{device}
{
    if(setLayoutType == SetLayoutMain) {
        // Необходимо описать привязки дескрипторов к этапам конвейера
        // Каждая привязка соостветствует типу дескриптора и может относиться к определенному этапу графического конвейера
        std::vector<VkDescriptorSetLayoutBinding> bindings =
        {
            {
                0,                                            // Индекс привязки
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,            // Тип дескриптора (буфер формы, обычный)
                1,                                            // Кол-во дескрипторов
                VK_SHADER_STAGE_VERTEX_BIT,                   // Этап конвейера (вершинный шейдер)
                nullptr
            },
            {
                1,                                            // Индекс привязки
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,    // Тип дескриптора (буфер формы, динамический)
                1,                                            // Кол-во дескрипторов
                VK_SHADER_STAGE_VERTEX_BIT,                   // Этап конвейера (вершинный шейдер)
                nullptr
            },
        };

        // Инициализировать размещение дескрипторного набора
        VkDescriptorSetLayoutCreateInfo descriptorLayoutInfo = {};
        descriptorLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorLayoutInfo.pNext = nullptr;
        descriptorLayoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        descriptorLayoutInfo.pBindings = bindings.data();

        if (vkCreateDescriptorSetLayout(m_device->logicalDevice, &descriptorLayoutInfo, nullptr, &m_descriptorSetLayout) != VK_SUCCESS) {
            throw std::runtime_error("Vulkan: Error in vkCreateDescriptorSetLayout. Can't initialize descriptor set layout");
        }

        kge::tools::LogMessage("Vulkan: Main descriptor set layout successfully initialized");

    }
    else if(setLayoutType == SetLayoutTextures){

        // Необходимо описать привязки дескрипторов к этапам конвейера
        // Каждая привязка соостветствует типу дескриптора и может относиться к определенному этапу графического конвейера
        std::vector<VkDescriptorSetLayoutBinding> bindings =
        {
            {
                0,                                            // Индекс привязки
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,    // Тип дескриптора (семплер изображения)
                1,                                            // Кол-во дескрипторов
                VK_SHADER_STAGE_FRAGMENT_BIT,                 // Этап конвейера (вершинный шейдер)
                nullptr
            },
        };

        // Инициализировать размещение дескрипторного набора
        VkDescriptorSetLayoutCreateInfo descriptorLayoutInfo = {};
        descriptorLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorLayoutInfo.pNext = nullptr;
        descriptorLayoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        descriptorLayoutInfo.pBindings = bindings.data();

        if (vkCreateDescriptorSetLayout(m_device->logicalDevice, &descriptorLayoutInfo, nullptr, &m_descriptorSetLayout) != VK_SUCCESS) {
            throw std::runtime_error("Vulkan: Error in vkCreateDescriptorSetLayout. Can't initialize descriptor set layout");
        }

        kge::tools::LogMessage("Vulkan: Texture descriptor set layout successfully initialized");
    }
}

/**
* Деинициализация размещения
* @param const kge::vkstructs::Device &device - устройство
* @VkDescriptorSetLayout * descriptorSetLayout - указатель на хендл размещения
*/
const VkDescriptorSetLayout& KGEVkDescriptorSetLayout::descriptorSetLayout()
{
    return m_descriptorSetLayout;
}

KGEVkDescriptorSetLayout::~KGEVkDescriptorSetLayout()
{
    if (m_device->logicalDevice != nullptr && m_descriptorSetLayout != nullptr && m_descriptorSetLayout != nullptr) {
        vkDestroyDescriptorSetLayout(m_device->logicalDevice, m_descriptorSetLayout, nullptr);
        m_descriptorSetLayout = nullptr;
        kge::tools::LogMessage("Vulkan: Descriptor set layout successfully deinitialized");
    }
}
