#include "graphic/VulkanCoreModules/KGEVkGraphicsPipeline.h"

/**
* Инициализация графического конвейера
* @param const kge::vkstructs::Device &device - устройство
* @param VkPipelineLayout pipelineLayout - хендл размещения конвейера
* @param vktoolkit::Swapchain &swapchain - swap-chain, для получения информации о разрешении
* @param VkRenderPass renderPass - хендл прохода рендеринга (на него ссылается конвейер)
*
* @note - графический конвейер производит рендериннг принимая вершинные данные на вход и выводя пиксели в
* буферы кадров. Конвейер состоит из множества стадий, некоторые из них программируемые (шейдерные). Конвейер
* относится к конкретному проходу рендеринга (и подпроходу). В методе происходит конфигурация всех стадий,
* загрузка шейдеров, настройка конвейера.
*/
KGEVkGraphicsPipeline::KGEVkGraphicsPipeline(VkPipeline* pipeline,
                                             const kge::vkstructs::Device* device,
                                             VkPipelineLayout pipelineLayout,
                                             const kge::vkstructs::Swapchain &swapchain,
                                             VkRenderPass renderPass):
    m_pipeline{pipeline},
    m_device{device}
{
    // Конфигурация привязок и аттрибутов входных данных (вершинных)
    std::vector<VkVertexInputBindingDescription> bindingDescription = kge::vkutility::GetVertexInputBindingDescriptions(0);
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions = kge::vkutility::GetVertexInputAttributeDescriptions(0);

    // Конфигурация стадии ввода вершинных данных
    VkPipelineVertexInputStateCreateInfo vertexInputStage = {};
    vertexInputStage.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputStage.pNext = nullptr;
    vertexInputStage.flags = 0;
    vertexInputStage.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescription.size());
    vertexInputStage.pVertexBindingDescriptions = bindingDescription.data();
    vertexInputStage.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputStage.pVertexAttributeDescriptions = attributeDescriptions.data();

    // Описание этапа "сборки" входных данных
    // Конвейер будет "собирать" вершинны в набор треугольников
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyStage = {};
    inputAssemblyStage.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyStage.pNext = nullptr;
    inputAssemblyStage.flags = 0;
    inputAssemblyStage.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;					// Список треугольников
    inputAssemblyStage.primitiveRestartEnable = false;								// Перезагрузка примитивов не используется

    // Прогамируемые (шейдерные) этапы конвейера
    // Используем 2 шейдера - вершинный (для каждой вершины) и фрагментный (пиксельный, для каждого пикселя)
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages = {
        {
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            nullptr,
            0,
            VK_SHADER_STAGE_VERTEX_BIT,
            kge::vkutility::LoadSPIRVShader(std::filesystem::path("/home/vxuser/GitHub/KitevaGameEngine/shaders/vert.spv"), device->logicalDevice),
            "main",
            nullptr
        },
        {
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            nullptr,
            0,
            VK_SHADER_STAGE_FRAGMENT_BIT,
            kge::vkutility::LoadSPIRVShader(std::filesystem::path("/home/vxuser/GitHub/KitevaGameEngine/shaders/frag.spv"), device->logicalDevice),
            "main",
            nullptr
        }
    };

    // Описываем область просмотра
    // Размеры равны размерам изображений swap-chain, которые в свою очередь равны размерам поверхности отрисовки
    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(swapchain.imageExtent.width);
    viewport.height = static_cast<float>(swapchain.imageExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    // Настройки обрезки изображения (не обрезать, размеры совпадают с областью просмотра)
    VkRect2D scissor = {};
    scissor.offset = { 0, 0 };
    scissor.extent = swapchain.imageExtent;

    // Описываем этап вывода в область просмотра
    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.pNext = nullptr;
    viewportState.flags = 0;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    // Описываем этап растеризации
    VkPipelineRasterizationStateCreateInfo rasterizationStage = {};
    rasterizationStage.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationStage.pNext = nullptr;
    rasterizationStage.flags = 0;
    rasterizationStage.depthClampEnable = VK_FALSE;                     // Фрагменты за ближней и дальней гранью камеры отбрасываются (VK_TRUE для противоположного эффекта)
    rasterizationStage.rasterizerDiscardEnable = VK_FALSE;              // Отключение растеризации геометрии - не нужно (VK_TRUE для противоположного эффекта)
    rasterizationStage.polygonMode = VK_POLYGON_MODE_FILL;              // Закрашенные полигоны
    rasterizationStage.lineWidth = 1.0f;                                // Ширина линии
    rasterizationStage.cullMode = VK_CULL_MODE_BACK_BIT;                // Отсечение граней (отсекаются те, что считаются задними)
    rasterizationStage.frontFace = VK_FRONT_FACE_CLOCKWISE;             // Порялок следования вершин для лицевой грани - по часовой стрелке
    rasterizationStage.depthBiasEnable = VK_FALSE;                      // Контроль значений глубины
    rasterizationStage.depthBiasConstantFactor = 0.0f;
    rasterizationStage.depthBiasClamp = 0.0f;
    rasterizationStage.depthBiasSlopeFactor = 0.0f;

    // Описываем этап z-теста (теста глубины)
    // Активировать тест глубины, использовать сравнение "меньше или равно"
    VkPipelineDepthStencilStateCreateInfo depthStencilStage = {};
    depthStencilStage.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilStage.pNext = nullptr;
    depthStencilStage.flags = 0;
    depthStencilStage.depthTestEnable = VK_TRUE;
    depthStencilStage.depthWriteEnable = VK_TRUE;
    depthStencilStage.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    depthStencilStage.depthBoundsTestEnable = VK_FALSE;
    depthStencilStage.back.failOp = VK_STENCIL_OP_KEEP;
    depthStencilStage.back.passOp = VK_STENCIL_OP_KEEP;
    depthStencilStage.back.compareOp = VK_COMPARE_OP_ALWAYS;
    depthStencilStage.stencilTestEnable = VK_FALSE;
    depthStencilStage.front = depthStencilStage.back;

    // Описываем этап мультисемплинга (сглаживание пиксельных лесенок)
    // Мултисемплинг - добавляет дополнительные ключевые точки в пиксел (семплы), для более точного вычисления его цвета
    // за счет чего возникает эффект более сглаженных линий (в данном примере не используется мульти-семплинг)
    VkPipelineMultisampleStateCreateInfo multisamplingStage = {};
    multisamplingStage.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisamplingStage.sampleShadingEnable = VK_FALSE;
    multisamplingStage.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisamplingStage.minSampleShading = 1.0f;
    multisamplingStage.pSampleMask = nullptr;
    multisamplingStage.alphaToCoverageEnable = VK_FALSE;
    multisamplingStage.alphaToOneEnable = VK_FALSE;

    // Этап смешивания цвета (для каждого фрейм-буфера)
    // На этом этапе можно настроить как должны отображаться, например, полупрозрачные объекты
    // В начлае нужно описать состояния смешивания для цветовых вложений (используем одно состояние на вложение) (???)
    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    // Глобальные настройки смешивания
    VkPipelineColorBlendStateCreateInfo colorBlendState = {};
    colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendState.logicOpEnable = VK_FALSE;
    colorBlendState.logicOp = VK_LOGIC_OP_COPY;
    colorBlendState.attachmentCount = 1;
    colorBlendState.pAttachments = &colorBlendAttachment;

    // Информация инициализации графического конвейера
    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());    // Кол-во шейдерных этапов
    pipelineInfo.pStages = shaderStages.data();                 // Шейдерные этапы (их конфигураци)
    pipelineInfo.pVertexInputState = &vertexInputStage;         // Настройки этапа ввода вершинных данных
    pipelineInfo.pInputAssemblyState = &inputAssemblyStage;     // Настройки этапа сборки примитивов из полученных вершин
    pipelineInfo.pViewportState = &viewportState;               // Настройки области видимости
    pipelineInfo.pRasterizationState = &rasterizationStage;     // Настройки этапа растеризации
    pipelineInfo.pDepthStencilState = &depthStencilStage;       // Настройка этапа z-теста
    pipelineInfo.pMultisampleState = &multisamplingStage;       // Настройки этапа мультисемплинга
    pipelineInfo.pColorBlendState = &colorBlendState;           // Настройки этапа смешивания цветов
    pipelineInfo.layout = pipelineLayout;                       // Размещение конвейера
    pipelineInfo.renderPass = renderPass;                       // Связываем конвейер с соответствующим проходом рендеринга
    pipelineInfo.subpass = 0;                                   // Связываем с под-проходом (первый под-проход)

    // Создание графического конвейера
    if (vkCreateGraphicsPipelines(device->logicalDevice,
                                  nullptr, 1,
                                  &pipelineInfo,
                                  nullptr,
                                  m_pipeline) != VK_SUCCESS) {
        throw std::runtime_error("Vulkan: Error while creating pipeline");
    }

    kge::tools::LogMessage("Vulkan: Pipeline sucessfully initialized");

    // Шейдерные модули больше не нужны после создания конвейера
    for (VkPipelineShaderStageCreateInfo &shaderStageInfo : shaderStages) {
        vkDestroyShaderModule(device->logicalDevice, shaderStageInfo.module, nullptr);
    }
}

KGEVkGraphicsPipeline::~KGEVkGraphicsPipeline()
{
    if (m_device->logicalDevice != nullptr && *m_pipeline != nullptr && m_pipeline != nullptr) {
        vkDestroyPipeline(m_device->logicalDevice, *m_pipeline, nullptr);
        m_pipeline = nullptr;
        kge::tools::LogMessage("Vulkan: Pipeline sucessfully deinitialized");
    }
}
