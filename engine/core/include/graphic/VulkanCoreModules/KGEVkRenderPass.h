#ifndef KGEVKRENDERPASS_H
#define KGEVKRENDERPASS_H
#include <graphic/KGEVulkan.h>

class KGEVkRenderPass
{
    VkRenderPass m_renderPass;
    const kge::vkstructs::Device* m_device;

public:
    KGEVkRenderPass(const kge::vkstructs::Device* device,
                    VkSurfaceKHR surface,
                    VkFormat colorAttachmentFormat,
                    VkFormat depthStencilFormat);
    ~KGEVkRenderPass();
    VkRenderPass renderPass() const;
};

#endif // KGEVKRENDERPASS_H
