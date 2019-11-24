#include "graphic/GKEVulkanApp.h"
#include "graphic/VulkanWindowControl/LinuxXCBWindowControl.h"
#include "graphic/VulkanWindowControl/WindowsWindowControl.h"

GKEVulkanApp::GKEVulkanApp(const char *appName, unsigned int initWidth, unsigned int initHeight):
    m_initWidth(initWidth),
    m_initHeight(initHeight),
    m_core(appName)
{
    m_appName = std::string(appName);
}

GKEVulkanApp::~GKEVulkanApp()
{

}

void GKEVulkanApp::Init()
{
#ifdef _WIN32
    m_pWindowControl = new WindowsWindowControl(m_appName.c_str());
#else
    m_pWindowControl = new LinuxXCBWindowControl();
#endif
    m_pWindowControl->Init(m_initWidth, m_initHeight);

    m_core.Init(m_pWindowControl);

    /*! @brief m_core.GetDevice()       - устройство
        @brief m_core.GetQueueFamily()  - индекс набора очередей
        @brief 0                        - индекс очереди в этом наборе (в нашем случае 0 т.к. у нас только одна очередь).
        @brief &m_queue                 - ссылку на объект VkQueue с устройства*/
    vkGetDeviceQueue(m_core.GetDevice(), m_core.GetQueueFamily(), 0, &m_queue);

    CreateSwapChain();
    CreateCommandBuffer();
    CreateRenderPass();
    CreateFramebuffer();
    CreateShaders();
    CreatePipeline();
    RecordCommandBuffers();
}

void GKEVulkanApp::Run()
{
    for(;;)
        RenderScene();
}

void GKEVulkanApp::CreateSwapChain()
{
    /*! получить свойства поверхности из объекта ядра Vulkan */
    const VkSurfaceCapabilitiesKHR& SurfaceCaps = m_core.GetSurfaceCaps();

    assert(SurfaceCaps.currentExtent.width != -1);

    unsigned int NumImages = 2;

    assert(NumImages >= SurfaceCaps.minImageCount);
    assert(NumImages <= SurfaceCaps.maxImageCount);

    VkSwapchainCreateInfoKHR SwapChainCreateInfo = {};

    SwapChainCreateInfo.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    SwapChainCreateInfo.surface          = m_core.GetSurface();
    SwapChainCreateInfo.minImageCount    = NumImages;
    SwapChainCreateInfo.imageFormat      = m_core.GetSurfaceFormat().format;
    SwapChainCreateInfo.imageColorSpace  = m_core.GetSurfaceFormat().colorSpace;
    SwapChainCreateInfo.imageExtent      = SurfaceCaps.currentExtent;
    SwapChainCreateInfo.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    SwapChainCreateInfo.preTransform     = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    SwapChainCreateInfo.imageArrayLayers = 1;
    SwapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    SwapChainCreateInfo.presentMode      = VK_PRESENT_MODE_FIFO_KHR;
    SwapChainCreateInfo.clipped          = true;
    SwapChainCreateInfo.compositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    VkResult res = vkCreateSwapchainKHR(m_core.GetDevice(), &SwapChainCreateInfo, nullptr, &m_swapChainKHR);
    CHECK_VULKAN_ERROR("vkCreateSwapchainKHR error %d\n", res);

    printf("Swap chain created\n");

    unsigned int NumSwapChainImages = 0;
    res = vkGetSwapchainImagesKHR(m_core.GetDevice(), m_swapChainKHR, &NumSwapChainImages, nullptr);
    CHECK_VULKAN_ERROR("vkGetSwapchainImagesKHR error %d\n", res);
    assert(NumImages == NumSwapChainImages);

    printf("Number of images %d\n", NumSwapChainImages);

    m_images.resize(NumSwapChainImages);
    m_views.resize(NumSwapChainImages);
    m_cmdBufs.resize(NumSwapChainImages);

    res = vkGetSwapchainImagesKHR(m_core.GetDevice(), m_swapChainKHR, &NumSwapChainImages, &(m_images[0]));
    CHECK_VULKAN_ERROR("vkGetSwapchainImagesKHR error %d\n", res);
}

void GKEVulkanApp::CreateCommandBuffer()
{
    VkCommandPoolCreateInfo cmdPoolCreateInfo = {};
    cmdPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmdPoolCreateInfo.queueFamilyIndex = m_core.GetQueueFamily();

    VkResult res = vkCreateCommandPool(m_core.GetDevice(), &cmdPoolCreateInfo, nullptr, &m_cmdBufPool);
    CHECK_VULKAN_ERROR("vkCreateCommandPool error %d\n", res);

    printf("Command buffer pool created\n");

    VkCommandBufferAllocateInfo cmdBufAllocInfo = {};
    cmdBufAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdBufAllocInfo.commandPool = m_cmdBufPool;
    cmdBufAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmdBufAllocInfo.commandBufferCount = m_images.size();

    res = vkAllocateCommandBuffers(m_core.GetDevice(), &cmdBufAllocInfo, &m_cmdBufs[0]);
    CHECK_VULKAN_ERROR("vkAllocateCommandBuffers error %d\n", res);

    printf("Created command buffers\n");
}

void GKEVulkanApp::CreateRenderPass()
{
    VkAttachmentReference attachRef = {};
    attachRef.attachment = 0;
    attachRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpassDesc = {};
    subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDesc.colorAttachmentCount = 1;
    subpassDesc.pColorAttachments = &attachRef;

    VkAttachmentDescription attachDesc = {};
    attachDesc.format = m_core.GetSurfaceFormat().format;
    attachDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachDesc.initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    attachDesc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    attachDesc.samples = VK_SAMPLE_COUNT_1_BIT;

    VkRenderPassCreateInfo renderPassCreateInfo = {};
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCreateInfo.attachmentCount = 1;
    renderPassCreateInfo.pAttachments = &attachDesc;
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpassDesc;

    VkResult res = vkCreateRenderPass(m_core.GetDevice(), &renderPassCreateInfo, nullptr, &m_renderPass);
    CHECK_VULKAN_ERROR("vkCreateRenderPass error %d\n", res);

    printf("Created a render pass\n");
}

void GKEVulkanApp::CreateFramebuffer()
{
    m_fbs.resize(m_images.size());

    VkResult res;

    for (unsigned int i = 0 ; i < m_images.size() ; i++)
    {
        VkImageViewCreateInfo ViewCreateInfo = {};
        ViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        ViewCreateInfo.image = m_images[i];
        ViewCreateInfo.format = m_core.GetSurfaceFormat().format;
        ViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        ViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        ViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        ViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        ViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        ViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        ViewCreateInfo.subresourceRange.baseMipLevel = 0;
        ViewCreateInfo.subresourceRange.levelCount = 1;
        ViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        ViewCreateInfo.subresourceRange.layerCount = 1;

        res = vkCreateImageView(m_core.GetDevice(), &ViewCreateInfo, nullptr, &m_views[i]);
        CHECK_VULKAN_ERROR("vkCreateImageView error %d\n", res);

        VkFramebufferCreateInfo fbCreateInfo = {};
        fbCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fbCreateInfo.renderPass = m_renderPass;
        fbCreateInfo.attachmentCount = 1;
        fbCreateInfo.pAttachments = &m_views[i];
        fbCreateInfo.width = m_initWidth;
        fbCreateInfo.height = m_initHeight;
        fbCreateInfo.layers = 1;

        res = vkCreateFramebuffer(m_core.GetDevice(), &fbCreateInfo, nullptr, &m_fbs[i]);
        CHECK_VULKAN_ERROR("vkCreateFramebuffer error %d\n", res);
    }

    printf("Frame buffers created\n");
}

void GKEVulkanApp::CreateShaders()
{
    m_vsModule = VulkanCreateShaderModule(m_core.GetDevice(), "D:/GitHub/KitevaGameEngine/engine/core/Shaders/vs.spv");
    assert(m_vsModule);

    m_fsModule = VulkanCreateShaderModule(m_core.GetDevice(), "D:/GitHub/KitevaGameEngine/engine/core/Shaders/fs.spv");
    assert(m_fsModule);
}

void GKEVulkanApp::CreatePipeline()
{
    VkPipelineShaderStageCreateInfo shaderStageCreateInfo[2] = {};

    shaderStageCreateInfo[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStageCreateInfo[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStageCreateInfo[0].module = m_vsModule;
    shaderStageCreateInfo[0].pName = "main";
    shaderStageCreateInfo[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStageCreateInfo[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shaderStageCreateInfo[1].module = m_fsModule;
    shaderStageCreateInfo[1].pName = "main";

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    VkPipelineInputAssemblyStateCreateInfo pipelineIACreateInfo = {};
    pipelineIACreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    pipelineIACreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkViewport vp = {};
    vp.x = 0.0f;
    vp.y = 0.0f;
    vp.width  = (float)m_initWidth;
    vp.height = (float)m_initHeight;
    vp.minDepth = 0.0f;
    vp.maxDepth = 1.0f;

    VkPipelineViewportStateCreateInfo vpCreateInfo = {};
    vpCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    vpCreateInfo.viewportCount = 1;
    vpCreateInfo.pViewports = &vp;

    VkPipelineRasterizationStateCreateInfo rastCreateInfo = {};
    rastCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rastCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
    rastCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    rastCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rastCreateInfo.lineWidth = 1.0f;

    VkPipelineMultisampleStateCreateInfo pipelineMSCreateInfo = {};
    pipelineMSCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;

    VkPipelineColorBlendAttachmentState blendAttachState = {};
    blendAttachState.colorWriteMask = 0xf;

    VkPipelineColorBlendStateCreateInfo blendCreateInfo = {};
    blendCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    blendCreateInfo.logicOp = VK_LOGIC_OP_COPY;
    blendCreateInfo.attachmentCount = 1;
    blendCreateInfo.pAttachments = &blendAttachState;

    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = ARRAY_SIZE_IN_ELEMENTS(shaderStageCreateInfo);
    pipelineInfo.pStages = &shaderStageCreateInfo[0];
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &pipelineIACreateInfo;
    pipelineInfo.pViewportState = &vpCreateInfo;
    pipelineInfo.pRasterizationState = &rastCreateInfo;
    pipelineInfo.pMultisampleState = &pipelineMSCreateInfo;
    pipelineInfo.pColorBlendState = &blendCreateInfo;
    pipelineInfo.renderPass = m_renderPass;
    pipelineInfo.basePipelineIndex = -1;

    VkResult res = vkCreateGraphicsPipelines(m_core.GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_pipeline);
    CHECK_VULKAN_ERROR("vkCreateGraphicsPipelines error %d\n", res);

    printf("Graphics pipeline created\n");
}

void GKEVulkanApp::RecordCommandBuffers()
{
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

    VkClearColorValue clearColor = { {164.0f/256.0f, 30.0f/256.0f, 34.0f/256.0f, 0.0f} };
    VkClearValue clearValue = {};
    clearValue.color = clearColor;

    VkImageSubresourceRange imageRange = {};
    imageRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageRange.levelCount = 1;
    imageRange.layerCount = 1;

    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_renderPass;
    renderPassInfo.renderArea.offset.x = 0;
    renderPassInfo.renderArea.offset.y = 0;
    renderPassInfo.renderArea.extent.width = m_initWidth;
    renderPassInfo.renderArea.extent.height = m_initHeight;
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearValue;

    VkViewport viewport = { 0 };
    viewport.width = (float)m_initWidth;
    viewport.height = (float)m_initHeight;
    viewport.minDepth = (float)0.0f;
    viewport.maxDepth = (float)1.0f;

    VkRect2D scissor = { 0 };
    scissor.extent.width = m_initWidth;
    scissor.extent.height = m_initHeight;
    scissor.offset.x = 0;
    scissor.offset.y = 0;

    for (unsigned int i = 0 ; i < m_cmdBufs.size() ; i++)
    {
        VkResult res = vkBeginCommandBuffer(m_cmdBufs[i], &beginInfo);
        CHECK_VULKAN_ERROR("vkBeginCommandBuffer error %d\n", res);
        renderPassInfo.framebuffer = m_fbs[i];

        vkCmdBeginRenderPass(m_cmdBufs[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(m_cmdBufs[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);

        vkCmdSetViewport(m_cmdBufs[i], 0, 1, &viewport);

        vkCmdSetScissor(m_cmdBufs[i], 0, 1, &scissor);

        vkCmdDraw(m_cmdBufs[i], 3, 1, 0, 0);

        vkCmdEndRenderPass(m_cmdBufs[i]);

        res = vkEndCommandBuffer(m_cmdBufs[i]);
        CHECK_VULKAN_ERROR("vkEndCommandBuffer error %d\n", res);
    }

    printf("Command buffers recorded\n");
}

void GKEVulkanApp::RenderScene()
{
    unsigned int ImageIndex = 0;

    VkResult res = vkAcquireNextImageKHR(m_core.GetDevice(), m_swapChainKHR, UINT64_MAX, nullptr, nullptr, &ImageIndex);
    CHECK_VULKAN_ERROR("vkAcquireNextImageKHR error %d\n" , res);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount   = 1;
    submitInfo.pCommandBuffers      = &m_cmdBufs[ImageIndex];

    res = vkQueueSubmit(m_queue, 1, &submitInfo, nullptr);
    CHECK_VULKAN_ERROR("vkQueueSubmit error %d\n", res);

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.swapchainCount     = 1;
    presentInfo.pSwapchains        = &m_swapChainKHR;
    presentInfo.pImageIndices      = &ImageIndex;

    res = vkQueuePresentKHR(m_queue, &presentInfo);
    CHECK_VULKAN_ERROR("vkQueuePresentKHR error %d\n" , res);
}
