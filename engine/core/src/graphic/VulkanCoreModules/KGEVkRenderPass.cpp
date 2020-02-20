#include "graphic/VulkanCoreModules/KGEVkRenderPass.h"

/**
* Инициализация прохода рендеринга.
* @param const vktoolkit::Device &device - устройство
* @param VkSurfaceKHR surface - хендл поверхности, передается лишь для проверки поддержки запрашиваемого формата
* @param VkFormat colorAttachmentFormat - формат цветовых вложений/изображений, должен поддерживаться поверхностью
* @param VkFormat depthStencilFormat - формат вложений глубины, должен поддерживаться устройством
* @return VkRenderPass - хендл прохода рендеринга
*
* @note - Проход рендеринга можно понимать как некую стадию на которой выполняются все команды рендерига и происходит цикл конвейера
* Проход состоит из под-проходов, и у каждого под-прохода может быть своя конфигурация конвейера. Конфигурация же прохода
* определяет в каком состоянии (размещении памяти) будет вложение (цветовое, глубины и тд)
*/
VkRenderPass KGEVkRenderPass::renderPass() const
{
    return m_renderPass;
}

KGEVkRenderPass::KGEVkRenderPass(const kge::vkstructs::Device *device,
                                 VkSurfaceKHR surface,
        VkFormat colorAttachmentFormat,
        VkFormat depthStencilFormat):
    m_device{device}
{
    // Проверка доступности формата вложений (изображений)
    kge::vkstructs::SurfaceInfo si = kge::vkutility::GetSurfaceInfo(device->physicalDevice, surface);
    if (!si.IsFormatSupported(colorAttachmentFormat)) {
        throw std::runtime_error("Vulkan: Required surface format is not supported. Can't initialize render-pass");
    }

    // Проверка доступности формата глубины
    if (!device->IsDepthFormatSupported(depthStencilFormat)) {
        throw std::runtime_error("Vulkan: Required depth-stencil format is not supported. Can't initialize render-pass");
    }

    // Массив описаний вложений
    // Пояснение: изображения в которые происходит рендеринг либо которые поступают на вход прохода называются вложениями.
    // Это может быть цветовое вложение (по сути обычное изображение) либо вложение глубины, трафарета и т.д.
    std::vector<VkAttachmentDescription> attachments;

    // Описаниие цветового вложения (изображение)
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = colorAttachmentFormat;                                       // Формат цвета должен соответствовать тому что будет использован при создании своп-чейна
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;                                      // Не использовать мультисемплинг (один семпл на пиксел)
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;                                 // На этапе начала прохода - очищать вложение
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;                               // На этапе конца прохода - хранить вложение (для дальнешей презентации)
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;                      // Подресурс трафарета (начало прохода) - не используется
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;                    // Подресурс трафарета (конце прохода) - не исрользуется
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;                            // Размещение памяти в начале (не имеет значения, любое)
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;                        // Размещение памяти к которому вложение будет приведено после окончания прохода (для представления)
    attachments.push_back(colorAttachment);

    // Описание вложения глубины трафарета (z-буфер)
    VkAttachmentDescription depthStencilAttachment = {};
    depthStencilAttachment.format = depthStencilFormat;
    depthStencilAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthStencilAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;                           // На этапе начала прохода - очищать вложение
    depthStencilAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;                     // На этапе конца прохода - не имеет значение (память не используется для презентации, можно не хранить)
    depthStencilAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;                // Подресурс трафарета (начало прохода) - не используется
    depthStencilAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;              // Подресурс трафарета (конце прохода) - не исрользуется
    depthStencilAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;                      // Размещение памяти в начале (не имеет значения, любое)
    depthStencilAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; // Размещение памяти к которому вложение будет приведено после окончания прохода (глубина-трафарет)
    attachments.push_back(depthStencilAttachment);


    // Массив ссылок на цветовые вложения
    // Для каждой ссылки указывается индекс цветового вложения в массиве вложений и ожидаемое размещение
    std::vector<VkAttachmentReference> colorAttachmentReferences = {
        {
            0,                                                       // Первый элемент массива вложений
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL                 // Ожидается что размещение будет VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        }
    };

    // Ссылка на вложение глубины-трафарета
    VkAttachmentReference depthStencilAttachemntReference = {
        1,                                                           // Второй элемент массива вложений
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL             // Ожидается что размещение будет VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    };

    // Описание единственного под-прохода
    VkSubpassDescription subpassDescription = {};
    subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescription.colorAttachmentCount = static_cast<uint32_t>(colorAttachmentReferences.size());  // Кол-во цветовых вложений
    subpassDescription.pColorAttachments = colorAttachmentReferences.data();                            // Цветовые вложения (вложения для записи)
    subpassDescription.pDepthStencilAttachment = &depthStencilAttachemntReference;                      // Глубина-трафарет (не используется)
    subpassDescription.inputAttachmentCount = 0;                                                        // Кол-во входных вложений (не используются)
    subpassDescription.pInputAttachments = nullptr;                                                     // Входные вложения (вложения для чтения, напр. того что было записано в пред-щем под-проходе)
    subpassDescription.preserveAttachmentCount = 0;                                                     // Кол-во хранимых вложений (не используется)
    subpassDescription.pPreserveAttachments = nullptr;                                                  // Хранимые вложения могут быть использованы для много-кратного использования в разных под-проходах
    subpassDescription.pResolveAttachments = nullptr;                                                   // Resolve-вложения (полезны при работе с мульти-семплингом, не используется)

    // Настройка зависимостей под-проходов
    // Они добавят неявный шаблон перехода вложений
    // Каждая зависимость под-прохода подготовит память во время переходов вложения между под-проходами
    // Пояснение: VK_SUBPASS_EXTERNAL ссылается на внешний, неявный под-проход
    std::vector<VkSubpassDependency> dependencies = {
        // Первая зависимость, начало конвейера
        // Перевод размещения от заключительного (final) к первоначальному (initial)
        {
            VK_SUBPASS_EXTERNAL,                                                       // Передыдущий под-проход - внешний, неявный
            0,                                                                         // Следующий под-проход - первый (и единсвтенный)
            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,                                      // Подразумевается что предыдыщий под-проход (внешний) завершен на этапе завершения конвейера
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,                             // Целевой подпроход (первый) начнется на этапе вывода цветовой информации из конвейера
            VK_ACCESS_MEMORY_READ_BIT,
            VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            VK_DEPENDENCY_BY_REGION_BIT
        },
        // Вторая зависимость, выход - конец конвейера
        // Перевод размещения от первоначального (initial) к заключительному (final)
        {
            0,                                                                         // Предыдщий под-проход - первый (и единсвтенный)
            VK_SUBPASS_EXTERNAL,                                                       // Следующий - внешний, неявный
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,                             // Предыдущий под-проход (первый) завершен на этапе вывода цветовой информации
            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,                                      // Следующий под-проход (внешний) начат на этапе завершения конвейера
            VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            VK_ACCESS_MEMORY_READ_BIT,
            VK_DEPENDENCY_BY_REGION_BIT
        },
    };

    // Создать проход
    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<unsigned int>(attachments.size()); //Кол-во описаний вложений
    renderPassInfo.pAttachments = attachments.data();                               //Описания вложений
    renderPassInfo.subpassCount = 1;                                                //Кол-вл под-проходов
    renderPassInfo.pSubpasses = &subpassDescription;                                //Описание под-проходов
    renderPassInfo.dependencyCount = static_cast<unsigned int>(dependencies.size());//Кол-во зависимсотей
    renderPassInfo.pDependencies = dependencies.data();                             //Зависимости

    if (vkCreateRenderPass(device->logicalDevice, &renderPassInfo, nullptr, &m_renderPass) != VK_SUCCESS) {
        throw std::runtime_error("Vulkan: Failed to create render pass!");
    }

    kge::tools::LogMessage("Vulkan: Render pass successfully initialized");
}

KGEVkRenderPass::~KGEVkRenderPass()
{
    // Если создан проход рендера - уничтожить
    if (m_renderPass != nullptr) {
        vkDestroyRenderPass(m_device->logicalDevice, m_renderPass, nullptr);
        m_renderPass = nullptr;
        kge::tools::LogMessage("Vulkan: Render pass successfully deinitialized");
    }
}
