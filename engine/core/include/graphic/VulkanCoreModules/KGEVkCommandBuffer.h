#ifndef KGEVKCOMMANDBUFFER_H
#define KGEVKCOMMANDBUFFER_H

#include <graphic/KGEVulkan.h>

class KGEVkCommandBuffer
{
    const kge::vkstructs::Device* m_device;
    std::vector<VkCommandBuffer>* m_commandBuffersDraw;
    VkCommandPool *m_commandPool;
public:
    KGEVkCommandBuffer(std::vector<VkCommandBuffer>* commandBuffersDraw,
                       const kge::vkstructs::Device *device,
                       VkCommandPool *commandPool,
                       unsigned int count);
    ~KGEVkCommandBuffer();
};

#endif // KGEVKCOMMANDBUFFER_H
