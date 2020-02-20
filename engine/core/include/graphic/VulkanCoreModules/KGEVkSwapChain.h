#ifndef KGEVKSWAPCHAIN_H
#define KGEVKSWAPCHAIN_H
#include <graphic/KGEVulkan.h>

class KGEVkSwapChain
{
    //Структура содержит хендлы свопчейна, изображений, фрейм-буферов и тд
    kge::vkstructs::Swapchain m_swapchain;
    const kge::vkstructs::Device* m_device;
public:
    KGEVkSwapChain(const kge::vkstructs::Device* device,
                   VkSurfaceKHR surface,
                   VkSurfaceFormatKHR surfaceFormat,
                   VkFormat depthStencilFormat,
                   VkRenderPass renderPass,
                   unsigned int bufferCount,
                   kge::vkstructs::Swapchain * oldSwapchain = nullptr);
    ~KGEVkSwapChain();
    const kge::vkstructs::Swapchain& swapchain();
};

#endif // KGEVKSWAPCHAIN_H
