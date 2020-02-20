#ifndef KGEVKCOMMANDBUFFER_H
#define KGEVKCOMMANDBUFFER_H

#include <graphic/KGEVulkan.h>

class KGEVkCommandBuffer
{
    const kge::vkstructs::Device* m_device;
    std::vector<VkCommandBuffer> m_commandBuffersDraw;
    const VkCommandPool* m_commandPool;
public:
    KGEVkCommandBuffer(const kge::vkstructs::Device *device,
                       const VkCommandPool *commandPool,
                       unsigned int count);
    ~KGEVkCommandBuffer();
    std::vector<VkCommandBuffer> commandBuffersDraw() const;
};

#endif // KGEVKCOMMANDBUFFER_H
