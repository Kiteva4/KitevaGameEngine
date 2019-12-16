#ifndef KGEVKPIPELINELAYOUT_H
#define KGEVKPIPELINELAYOUT_H
#include <graphic/KGEVulkan.h>

class KGEVkPipelineLayout
{
    const kge::vkstructs::Device* m_device;
    VkPipelineLayout* m_pipelineLayout;
public:
    KGEVkPipelineLayout(VkPipelineLayout* pipelineLayout,
                        const kge::vkstructs::Device* device,
                        std::vector<VkDescriptorSetLayout> descriptorSetLayouts);
    ~KGEVkPipelineLayout();
};

#endif // KGEVKPIPELINELAYOUT_H
