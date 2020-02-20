#ifndef KGEVKDESCRIPTORSET_H
#define KGEVKDESCRIPTORSET_H

#include <graphic/KGEVulkan.h>

class KGEVkDescriptorSet
{
    VkDescriptorSet m_descriptorSet;
    const kge::vkstructs::Device* m_device;
    const VkDescriptorPool* m_descriptorPool;
    const VkDescriptorSetLayout* m_descriptorSetLayout;

public:
    KGEVkDescriptorSet(const kge::vkstructs::Device *device,
                       const VkDescriptorPool* descriptorPool,
                       const VkDescriptorSetLayout* descriptorSetLayout,
                       const kge::vkstructs::UniformBuffer* uniformBufferWorld,
                       const kge::vkstructs::UniformBuffer* uniformBufferModels);
    ~KGEVkDescriptorSet();
    VkDescriptorSet descriptorSet() const;
};

#endif // KGEVKDESCRIPTORSET_H
