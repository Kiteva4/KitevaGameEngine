#ifndef KGEVKGRAPHICSPIPELINE_H
#define KGEVKGRAPHICSPIPELINE_H
#include <graphic/KGEVulkan.h>

class KGEVkGraphicsPipeline
{
    VkPipeline* m_pipeline;
    const kge::vkstructs::Device* m_device;

public:
    KGEVkGraphicsPipeline(VkPipeline* pipeline,
                          const kge::vkstructs::Device* device,
                          VkPipelineLayout pipelineLayout,
                          const kge::vkstructs::Swapchain &swapchain,
                          VkRenderPass renderPass);
    ~KGEVkGraphicsPipeline();
};

#endif // KGEVKGRAPHICSPIPELINE_H
