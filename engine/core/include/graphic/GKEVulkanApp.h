#pragma once

#include "VulkanWindowControl/IVulkanWindowControl.h"
#include "KGEVulkanCore.h"

#include <vector>

class GKEVulkanApp
{
public:

    GKEVulkanApp(const char* appName, unsigned int initWidth , unsigned int initHeight);

    ~GKEVulkanApp();

    void Init();

    void Run();

private:

    unsigned int m_initWidth;
    unsigned int m_initHeight;

    /*! Функция цепочки переключений */
    void CreateSwapChain();
    void CreateCommandBuffer();
    void CreateRenderPass();
    void CreateFramebuffer();
    void CreateShaders();
    void CreatePipeline();
    void RecordCommandBuffers();
    void RenderScene();

    std::string m_appName;
    IVulkanWindowControl* m_pWindowControl;
    KGEVulkanCore m_core;
    /*! вектор изображений */
    std::vector<VkImage> m_images;
    /*! объект цепочки переключений */
    VkSwapchainKHR m_swapChainKHR;
    /*! очередь команд */
    VkQueue m_queue;
    /*! вектор буферов команд */
    std::vector<VkCommandBuffer> m_cmdBufs;
    /*! пул буферов команд */
    VkCommandPool m_cmdBufPool;

    std::vector<VkImageView> m_views;
    VkRenderPass m_renderPass;
    std::vector<VkFramebuffer> m_fbs;
    VkShaderModule m_vsModule;
    VkShaderModule m_fsModule;
    VkPipeline m_pipeline;
};
