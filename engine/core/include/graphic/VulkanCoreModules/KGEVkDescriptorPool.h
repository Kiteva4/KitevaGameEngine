#ifndef KGEVKDESCRIPTORPOOL_H
#define KGEVKDESCRIPTORPOOL_H

#include <graphic/KGEVulkan.h>

class KGEVkDescriptorPool
{
    VkDescriptorPool * m_descriptorPool;
    const kge::vkstructs::Device* m_device;

public:
    KGEVkDescriptorPool(VkDescriptorPool * descriptorPool,
                        const kge::vkstructs::Device* device);

    KGEVkDescriptorPool(VkDescriptorPool * descriptorPool,
                        const kge::vkstructs::Device* device,
                        uint32_t maxDescriptorSets);

    ~KGEVkDescriptorPool();
};

#endif // KGEVKDESCRIPTORPOOL_H
