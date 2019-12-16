#ifndef KGEVKUNIFORMBUFFERMODELS_H
#define KGEVKUNIFORMBUFFERMODELS_H

#include <graphic/KGEVulkan.h>

class KGEVkUniformBufferModels
{
    const kge::vkstructs::Device* m_device;
    kge::vkstructs::UniformBuffer* m_uniformBufferModels;
public:
    KGEVkUniformBufferModels(kge::vkstructs::UniformBuffer* uniformBufferModels,
                             const kge::vkstructs::Device* device,
                             unsigned int maxObjects);
    ~KGEVkUniformBufferModels();
};

#endif // KGEVKUNIFORMBUFFERMODELS_H
