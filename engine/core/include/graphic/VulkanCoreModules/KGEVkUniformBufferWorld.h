#ifndef KGEVKUNIFORMBUFFERWORLD_H
#define KGEVKUNIFORMBUFFERWORLD_H

#include <graphic/KGEVulkan.h>

class KGEVkUniformBufferWorld
{
    const kge::vkstructs::Device* m_device;
    kge::vkstructs::UniformBuffer m_uniformBufferWorld;
public:
    KGEVkUniformBufferWorld(const kge::vkstructs::Device* device);
    ~KGEVkUniformBufferWorld();
    kge::vkstructs::UniformBuffer *uniformBufferWorld();
};

#endif // KGEVKUNIFORMBUFFERWORLD_H
