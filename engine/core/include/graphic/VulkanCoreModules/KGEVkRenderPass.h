#ifndef KGEVKRENDERPASS_H
#define KGEVKRENDERPASS_H
#include <graphic/KGEVulkan.h>

class KGEVkRenderPass
{
    VkRenderPass *m_renderPass;
    const kge::vkstructs::Device &m_device;

public:
    KGEVkRenderPass(
            VkRenderPass *renderPass,
            const kge::vkstructs::Device &device,
            VkSurfaceKHR surface,
            VkFormat colorAttachmentFormat,
            VkFormat depthStencilFormat);
    ~KGEVkRenderPass();
};

#endif // KGEVKRENDERPASS_H
