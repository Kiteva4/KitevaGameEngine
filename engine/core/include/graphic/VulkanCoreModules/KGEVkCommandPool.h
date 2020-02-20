#ifndef KGEVKCOMMANDPOOL_H
#define KGEVKCOMMANDPOOL_H

#include <graphic/KGEVulkan.h>

class KGEVkCommandPool
{
    const kge::vkstructs::Device* m_device;
    VkCommandPool m_commandPool;
public:
    KGEVkCommandPool(const kge::vkstructs::Device* device,
                     unsigned int queueFamilyIndex);
    ~KGEVkCommandPool();
    const VkCommandPool &commandPool();
};

#endif // KGEVKCOMMANDPOOL_H
