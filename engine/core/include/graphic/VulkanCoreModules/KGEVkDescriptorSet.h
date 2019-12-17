#ifndef KGEVKDESCRIPTORSET_H
#define KGEVKDESCRIPTORSET_H

#include <graphic/KGEVulkan.h>

class KGEVkDescriptorSet
{
    VkDescriptorSet* m_descriptorSet;
    const kge::vkstructs::Device* m_device;
    VkDescriptorPool* m_descriptorPool;
    VkDescriptorSetLayout* m_descriptorSetLayout;

public:
    KGEVkDescriptorSet(VkDescriptorSet* descriptorSet,
                       const kge::vkstructs::Device *device,
                       VkDescriptorPool* descriptorPool,
                       VkDescriptorSetLayout* descriptorSetLayout,
                       const kge::vkstructs::UniformBuffer* uniformBufferWorld,
                       const kge::vkstructs::UniformBuffer* uniformBufferModels);
    ~KGEVkDescriptorSet();
};

#endif // KGEVKDESCRIPTORSET_H
