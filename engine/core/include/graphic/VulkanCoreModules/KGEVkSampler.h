#ifndef KGEVKSAMPLER_H
#define KGEVKSAMPLER_H

#include <graphic/KGEVulkan.h>

class KGEVkSampler
{
    const kge::vkstructs::Device* m_device;
    VkSampler m_sampler;
public:
    KGEVkSampler(const kge::vkstructs::Device* device);
    ~KGEVkSampler();
    VkSampler sampler() const;
};

#endif // KGEVKSAMPLER_H
