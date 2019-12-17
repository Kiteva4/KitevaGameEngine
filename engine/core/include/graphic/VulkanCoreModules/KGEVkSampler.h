#ifndef KGEVKSAMPLER_H
#define KGEVKSAMPLER_H

#include <graphic/KGEVulkan.h>

class KGEVkSampler
{
    const kge::vkstructs::Device* m_device;
    VkSampler* m_sampler;
public:
    KGEVkSampler(VkSampler* m_sampler,
                 const kge::vkstructs::Device* device);
    ~KGEVkSampler();
};

#endif // KGEVKSAMPLER_H
