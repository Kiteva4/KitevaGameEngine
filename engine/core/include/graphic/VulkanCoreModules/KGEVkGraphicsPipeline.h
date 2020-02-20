#ifndef KGEVKGRAPHICSPIPELINE_H
#define KGEVKGRAPHICSPIPELINE_H
#include <graphic/KGEVulkan.h>

class KGEVkGraphicsPipeline
{
    VkPipeline m_pipeline;
    const kge::vkstructs::Device* m_device;

public:
    KGEVkGraphicsPipeline(const kge::vkstructs::Device* device,
                          VkPipelineLayout pipelineLayout,
                          const kge::vkstructs::Swapchain &swapchain,
                          VkRenderPass renderPass);
    ~KGEVkGraphicsPipeline();
    VkPipeline pipeline() const;
};

#endif // KGEVKGRAPHICSPIPELINE_H
