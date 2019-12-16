#include "graphic/VulkanCoreModules/KGEVkSwapChain.h"

/**
* Swap-chain (список показа, цепочка свопинга) - представляет из себя набор сменяемых изображений
* @param const kge::vkstructs::Device &device - устройство, необходимо для создания
* @param VkSurfaceKHR surface - хкндл поверхоности, необходимо для создания объекта swap-chain и для проверки поддержки формата
* @param VkSurfaceFormatKHR surfaceFormat - формат изображений и цветовое пространство (должен поддерживаться поверхностью)
* @param VkFormat depthStencilFormat - формат вложений глубины (должен поддерживаться устройством)
* @param VkRenderPass renderPass - хендл прохода рендеринга, нужен для создания фрейм-буферов swap-chain
* @param unsigned int bufferCount - кол-во буферов кадра (напр. для тройной буферизации - 3)
* @param kge::vkstructs::Swapchain * oldSwapchain - передыдуший swap-chain (полезно в случае пересоздания свап-чейна, например, сменив размеро поверхности)
* @return kge::vkstructs::Swapchain структура описывающая swap-chain cодержащая необходимые хендлы
* @note - в одно изображение может происходить запись (рендеринг) в то время как другое будет показываться (презентация)
*/
KGEVkSwapChain::KGEVkSwapChain(kge::vkstructs::Swapchain *swapchain,
                               const kge::vkstructs::Device* device,
                               VkSurfaceKHR surface,
                               VkSurfaceFormatKHR surfaceFormat,
                               VkFormat depthStencilFormat,
                               VkRenderPass renderPass,
                               unsigned int bufferCount,
                               kge::vkstructs::Swapchain *oldSwapchain):
    m_swapchain{swapchain},
    m_device{device}
{
    // Информация о поверхности
    kge::vkstructs::SurfaceInfo si = kge::vkutility::GetSurfaceInfo(device->physicalDevice, surface);

    // Проверка доступности формата и цветового пространства изображений
    if (!si.IsSurfaceFormatSupported(surfaceFormat)) {
        throw std::runtime_error("Vulkan: Required surface format is not supported. Can't initialize swap-chain");
    }

    // Проверка доступности формата глубины
    if (!device->IsDepthFormatSupported(depthStencilFormat)) {
        throw std::runtime_error("Vulkan: Required depth-stencil format is not supported. Can't initialize render-pass");
    }

    // Если кол-во буферов задано
    if (bufferCount > 0) {
        // Проверить - возможно ли использовать запрашиваемое кол-во буферов (и изоображений соответственно)
        if (bufferCount < si.capabilities.minImageCount || (bufferCount > si.capabilities.maxImageCount && si.capabilities.maxImageCount !=0)) {
            std::string message = "Vulkan: Surface don't support " + std::to_string(bufferCount) + " images/buffers in swap-chain";
            throw std::runtime_error(message);
        }
    }
    // В противном случае попытаться найти оптимальное кол-во
    else {
        bufferCount = (si.capabilities.minImageCount + 1) > si.capabilities.maxImageCount ? si.capabilities.maxImageCount : (si.capabilities.minImageCount + 1);
    }

    // Выбор режима представления (FIFO_KHR по умолчнию, самый простой)
    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;

    // Если запрашиваемое кол-во буферов больше единицы - есть смысл выбрать более сложный режим,
    // но перед этим необходимо убедиться что поверхность его поддерживает
    if (bufferCount > 1) {
        for (const VkPresentModeKHR& availablePresentMode : si.presentModes) {
            //Если возможно - использовать VK_PRESENT_MODE_MAILBOX_KHR (вертикальная синхронизация)
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                presentMode = availablePresentMode;
                break;
            }
        }
    }

    // Информация о создаваемом swap-chain
    VkSwapchainCreateInfoKHR swapchainCreateInfo = {};
    swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfo.surface = surface;
    swapchainCreateInfo.minImageCount = bufferCount;                        // Минимальное кол-во изображений
    swapchainCreateInfo.imageFormat = surfaceFormat.format;					// Формат изображения
    swapchainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;			// Цветовое пространство
    swapchainCreateInfo.imageExtent = si.capabilities.currentExtent;        // Резрешение (расширение) изображений
    swapchainCreateInfo.imageArrayLayers = 1;                               // Слои (1 слой, не стереоскопический рендер)
    swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;   // Как используются изображения (как цветовые вложения)

    // Добавить информацию о формате и расширении в результирующий объект swap-chain'а (он будет отдан функцией)
    m_swapchain->imageFormat = swapchainCreateInfo.imageFormat;
    m_swapchain->imageExtent = swapchainCreateInfo.imageExtent;

    // Если старый swap-chain был передан - очистить его информацию о формате и расширении
    if (oldSwapchain != nullptr) {
        oldSwapchain->imageExtent = {};
        oldSwapchain->imageFormat = {};
    }

    // Индексы семейств
    std::vector<unsigned int> queueFamilyIndices = {
        static_cast<unsigned int>(device->queueFamilies.graphics),
        static_cast<unsigned int>(device->queueFamilies.present)
    };

    // Если для команд графики и представления используются разные семейства, значит доступ к ресурсу (в данном случае к буферам изображений)
    // должен быть распараллелен (следует использовать VK_SHARING_MODE_CONCURRENT, указав при этом кол-во семейств и их индексы)
    if (device->queueFamilies.graphics != device->queueFamilies.present) {
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchainCreateInfo.queueFamilyIndexCount = static_cast<uint32_t>(queueFamilyIndices.size());
        swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices.data();
    }
    // В противном случае подходящим будет VK_SHARING_MODE_EXCLUSIVE (с ресурсом работают команды одного семейства)
    else {
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    swapchainCreateInfo.preTransform = si.capabilities.currentTransform;                                        // Не используем трансформацмию изображения (трансформация поверхности по умолчнию) (???)
    swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;                                     // Смешивание альфа канала с другими окнами в системе (нет смешивания)
    swapchainCreateInfo.presentMode = presentMode;                                                              // Установка режима представления (тот что выбрали ранее)
    swapchainCreateInfo.clipped = VK_TRUE;                                                                      // Не рендерить перекрываемые другими окнами пиксели
    swapchainCreateInfo.oldSwapchain = (oldSwapchain != nullptr ? oldSwapchain->vkSwapchain : nullptr);  // Старый swap-chain (для более эффективного пересоздания можно указывать старый swap-chain)

    // Создание swap-chain (записать хендл в результирующий объект)
    if (vkCreateSwapchainKHR(device->logicalDevice, &swapchainCreateInfo, nullptr, &(m_swapchain->vkSwapchain)) != VK_SUCCESS) {
        throw std::runtime_error("Vulkan: Error in vkCreateSwapchainKHR function. Failed to create swapchain");
    }

    // Уничтожение предыдущего swap-chain, если он был передан
    if (oldSwapchain != nullptr) {
        vkDestroySwapchainKHR(device->logicalDevice, oldSwapchain->vkSwapchain, nullptr);
        oldSwapchain->vkSwapchain = nullptr;
    }

    // Получить хендлы изображений swap-chain
    // Кол-во изображений по сути равно кол-ву буферов (за это отвечает bufferCount при создании swap-chain)
    unsigned int swapChainImageCount = 0;
    vkGetSwapchainImagesKHR(device->logicalDevice, m_swapchain->vkSwapchain, &swapChainImageCount, nullptr);
    m_swapchain->images.resize(swapChainImageCount);
    vkGetSwapchainImagesKHR(device->logicalDevice, m_swapchain->vkSwapchain, &swapChainImageCount, m_swapchain->images.data());

    // Теперь необходимо создать image-views для каждого изображения (своеобразный интерфейс объектов изображений предостовляющий нужные возможности)
    // Если был передан старый swap-chain - предварительно очистить все его image-views
    if (oldSwapchain != nullptr) {
        if (!oldSwapchain->imageViews.empty()) {
            for (VkImageView const &swapchainImageView : oldSwapchain->imageViews) {
                vkDestroyImageView(device->logicalDevice, swapchainImageView, nullptr);
            }
            oldSwapchain->imageViews.clear();
        }
    }

    // Для каждого изображения (image) swap-chain'а создать свой imageView объект
    for (unsigned int i = 0; i < m_swapchain->images.size(); i++) {

        // Идентификатор imageView (будт добавлен в массив)
        VkImageView swapChainImageCount;

        // Информация для инициализации
        VkImageViewCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = m_swapchain->images[i];                       // Связь с изображением swap-chain
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;                        // Тип изображения (2Д текстура)
        createInfo.format = surfaceFormat.format;							// Формат изображения
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;			// По умолчанию
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        // Создать и добавить в массив
        if (vkCreateImageView(device->logicalDevice, &createInfo, nullptr, &swapChainImageCount) == VK_SUCCESS) {
            m_swapchain->imageViews.push_back(swapChainImageCount);
        }
        else {
            throw std::runtime_error("Vulkan: Error in vkCreateImageView function. Failed to create image views");
        }
    }


    // Буфер глубины (Z-буфер)
    // Буфер может быть один для всех фрейм-буферов, даже при двойной/тройной буферизации (в отличии от изображений swap-chain)
    // поскольку он не учавствует в презентации (память из него непосредственно не отображается на экране).

    // Если это пересоздание swap-chain (передан старый) следует очистить компоненты прежнего буфера глубины
    if (oldSwapchain != nullptr) {
        oldSwapchain->depthStencil.Deinit(device->logicalDevice);
    }

    // Создать буфер глубины-трафарета (обычное 2D-изображение с требуемым форматом)
    m_swapchain->depthStencil = kge::vkutility::CreateImageSingle(
                *device,
                VK_IMAGE_TYPE_2D,
                depthStencilFormat,
    { si.capabilities.currentExtent.width, si.capabilities.currentExtent.height, 1 },
                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                VK_IMAGE_TILING_OPTIMAL,
                swapchainCreateInfo.imageSharingMode);


    // Теперь необходимо создать фрейм-буферы привязанные к image-views объектам изображений и буфера глубины (изображения глубины)
    // Перед этим стоит очистить буферы старого swap-сhain (если он был передан)
    if (oldSwapchain != nullptr) {
        if (!oldSwapchain->framebuffers.empty()) {
            for (VkFramebuffer const &frameBuffer : oldSwapchain->framebuffers) {
                vkDestroyFramebuffer(device->logicalDevice, frameBuffer, nullptr);
            }
            oldSwapchain->framebuffers.clear();
        }
    }

    // Пройтись по всем image views и создать фрейм-буфер для каждого
    for (unsigned int i = 0; i < m_swapchain->imageViews.size(); i++) {

        // Хендл нового фреймбуфера
        VkFramebuffer framebuffer;

        std::vector<VkImageView> attachments(2);
        attachments[0] = m_swapchain->imageViews[i];                                    // Цветовое вложение (на каждый фрейм-буфер свое)
        attachments[1] = m_swapchain->depthStencil.vkImageView;                         // Буфер глубины (один на все фрейм-буферы)

        // Описание нового фреймбуфера
        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;                                        // Указание прохода рендера
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());    // Кол-во вложений
        framebufferInfo.pAttachments = attachments.data();                              // Связь с image-views объектом изображения swap-chain'а
        framebufferInfo.width = m_swapchain->imageExtent.width;                         // Разрешение (ширина)
        framebufferInfo.height = m_swapchain->imageExtent.height;                       // Разрешение (высота)
        framebufferInfo.layers = 1;                                                     // Один слой

        // В случае успешного создания - добавить в массив
        if (vkCreateFramebuffer(device->logicalDevice, &framebufferInfo, nullptr, &framebuffer) == VK_SUCCESS) {
            m_swapchain->framebuffers.push_back(framebuffer);
        }
        else {
            throw std::runtime_error("Vulkan: Error in vkCreateFramebuffer function. Failed to create frame buffers");
        }
    }

    kge::tools::LogMessage("Vulkan: Swap-chain successfully initialized");
}

KGEVkSwapChain::~KGEVkSwapChain()
{
    // Очистить фрейм-буферы
    if (!m_swapchain->framebuffers.empty()) {
        for (VkFramebuffer const &frameBuffer : m_swapchain->framebuffers) {
            vkDestroyFramebuffer(m_device->logicalDevice, frameBuffer, nullptr);
        }
        m_swapchain->framebuffers.clear();
    }

    // Очистить image-views объекты
    if (!m_swapchain->imageViews.empty()) {
        for (VkImageView const &imageView : m_swapchain->imageViews) {
            vkDestroyImageView(m_device->logicalDevice, imageView, nullptr);
        }
        m_swapchain->imageViews.clear();
    }

    // Очиска компонентов Z-буфера
    m_swapchain->depthStencil.Deinit(m_device->logicalDevice);

    // Очистить swap-chain
    if (m_swapchain->vkSwapchain != nullptr) {
        vkDestroySwapchainKHR(m_device->logicalDevice, m_swapchain->vkSwapchain, nullptr);
        m_swapchain->vkSwapchain = nullptr;
    }

    // Сбросить расширение и формат
    m_swapchain->imageExtent = {};
    m_swapchain->imageFormat = {};

    kge::tools::LogMessage("Vulkan: Swap-chain successfully deinitialized");
}
