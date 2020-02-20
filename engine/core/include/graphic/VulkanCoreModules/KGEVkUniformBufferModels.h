#ifndef KGEVKUNIFORMBUFFERMODELS_H
#define KGEVKUNIFORMBUFFERMODELS_H

#include <graphic/KGEVulkan.h>

class KGEVkUniformBufferModels
{
    const kge::vkstructs::Device* m_device;
public:
    kge::vkstructs::UniformBuffer m_uniformBufferModels;
    KGEVkUniformBufferModels(const kge::vkstructs::Device* device,
                             unsigned int maxObjects);
    ~KGEVkUniformBufferModels();
//    kge::vkstructs::UniformBuffer &uniformBufferModels();
};

#endif // KGEVKUNIFORMBUFFERMODELS_H
