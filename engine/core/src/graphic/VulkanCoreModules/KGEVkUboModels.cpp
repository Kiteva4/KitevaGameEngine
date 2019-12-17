#include "graphic/VulkanCoreModules/KGEVkUboModels.h"

/**
* Аллокация памяти под объект динамического UBO буфера
* @param const vktoolkit::Device &device - устройство (необходимо для получения выравнивания)
* @param unsigned int maxObjects - максимальное кол-вл объектов (для выяснения размера аллоцируемой памяти)
* @return vktoolkit::UboModelArray - указатель на аллоцированный массив матриц
*/
KGEVkUboModels::KGEVkUboModels(kge::vkstructs::UboModelArray* uboModels,
                               const kge::vkstructs::Device* device,
                               unsigned int maxObjects):
    m_uboModels{uboModels},
    m_device{device}
{
    // Получить оптимальное выравнивание для типа glm::mat4
    std::size_t dynamicAlignment = static_cast<size_t>(m_device->GetDynamicAlignment<glm::mat4>());

    // Вычислить размер буфера учитывая доступное вырванивание памяти (для типа glm::mat4 размером в 64 байта)
    std::size_t bufferSize = static_cast<size_t>(dynamicAlignment * maxObjects);

    // Аллоцировать память с учетом выравнивания
    *m_uboModels = static_cast<kge::vkstructs::UboModelArray>(aligned_alloc(bufferSize, dynamicAlignment));

    kge::tools::LogMessage("Vulkan: Dynamic UBO satage-buffer successfully allocated");
}

/**
* Освобождение памяти объекта динамического UBO буфера
* @param vktoolkit::UboModelArray * uboModels - указатель на массив матриц, память которого будет очищена
*/
KGEVkUboModels::~KGEVkUboModels()
{
    free(*m_uboModels);
    *m_uboModels = nullptr;
    kge::tools::LogMessage("Vulkan: Dynamic UBO satage-buffer successfully freed");
}
